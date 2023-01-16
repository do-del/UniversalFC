#include "stc8h.h"
#include <intrins.h>

/*
 * PWM1��P1.0���������������PWM��������Ƶ��U���λMOS��ѡͨ�źţ��ߵ�ƽ��ͨ���͵�ƽ��ֹ
 * PWM1_L��P1.1������������������Ƶ��U���λMOS��ѡͨ�źţ��ߵ�ƽ��ͨ���͵�ƽ��ֹ
 * PWM2��P1.2���������������PWM��������Ƶ��V���λMOS��ѡͨ�źţ��ߵ�ƽ��ͨ���͵�ƽ��ֹ
 * PWM2_L��P1.3������������������Ƶ��V���λMOS��ѡͨ�źţ��ߵ�ƽ��ͨ���͵�ƽ��ֹ
 * PWM3��P1.4���������������PWM��������Ƶ��W���λMOS��ѡͨ�źţ��ߵ�ƽ��ͨ���͵�ƽ��ֹ
 * PWM3_L��P1.5������������������Ƶ��W���λMOS��ѡͨ�źţ��ߵ�ƽ��ͨ���͵�ƽ��ֹ
 * ADC8��P0.0�������ø������룬�������U���Ӧ�綯�ƣ����ڹ�����Ƚ���������
 * ADC9��P0.1�������ø������룬�������V���Ӧ�綯��
 * ADC10��P0.2�������ø������룬�������W���Ӧ�綯��
 * CMP-��P3.6��������Ϊ�������룬�ӹ������·�е�
*/

/*
 * ��ˢ���ͨ�����
 * AB-AC-BC-BA-CA-CB-AB
 * ���Ӧ��Ӧ�綯�Ʊ仯
 * C�½�-B����-A�½�-C����-B�½�-A����-C�½�
*/

#include "sys.h"
#include "common.h"
#include "uart.h"
#include "pwm.h"
#include "adc.h"
#include "motor.h"
#include "timer.h"
#include "cmp.h"
#include "beep.h"

u16 cap_res_g[8];
u16 cap_res_lp;
u8 ch;

void Port_Init(void);	//оƬ��λ�����ų�ʼ��

void main(void)
{
	u8	i;
	u8 j;
	u32 sum;
	
	P_SW2 |= 0x80; //ʹ��XFR
	
	Port_Init();	//���ö˿ڳ�ʼ������
	
	PWMA_Config();	//����PWMA��ʼ������
	ADC_Config();	//����ADC��ʼ������
	CMP_Config();	//����ģ��Ƚ�����ʼ������
	Timer0_Config();	//���ö�ʱ��0��ʼ������
	Timer3_Config();	//���ö�ʱ��3��ʼ������
	Timer4_Config();	//���ö�ʱ��4��ʼ������

	PWMB_Config();
	
	Uart_Config();	//���ô��ڳ�ʼ������
	ES = 1;
	
	PWM_Set = 0;
	PWM_Value = 0;
	timeout = 0;
	
	Delay_n_ms(250);
	beep_1KHz(500);
	Delay_n_ms(250);
	
	EA = 1;	//�����ж�
	
	UartSendStr("--Brushless ESC Test--\r\n");
	
	while(1)
	{
		if(t0_flag)
		{
			t0_flag = 0;
			
			if(timeout != 0)
			{
				if(--timeout == 0)
				{
					m_running = 0;
					PWM_Value = 0;
					CMPCR1 = 0x8c;
					PWMA_ENO = 0;
					PWMA_CCR1 = 0;
					PWMA_CCR2 = 0;
					PWMA_CCR3 = 0;
					PWM1_L = 0;
					PWM2_L = 0;
					PWM3_L = 0;
					Delay_n_ms(250);
				}
			}
			if(!m_running && (PWM_Set >= D_START_PWM))
			{
				//UartSendStr("-Start-\r\n");
				m_starting = 1;
				for(i = 0;i<8;i++) phase_time_tmp[i] = 400; 
				Motor_Start();
				m_starting = 0;
				demagnetizing_cnt = 0;
				CMPCR1 &= ~0x40;
				if(step & 1)	CMPCR1 = 0xAC;		//�������ж�
				else			CMPCR1 = 0x9C;		//�½����ж�
				m_running = 1;
				Delay_n_ms(250);	//��ʱһ��, ����������
				Delay_n_ms(250);
				timeout = 125;		//������ʱʱ�� 125*4 = 500ms
			}
			if(m_running)
			{
				//UartSendStr("-Run-\r\n");
				if(PWM_Value < PWM_Set) PWM_Value++;
				if(PWM_Value > PWM_Set) PWM_Value--;
				if(PWM_Value<(D_START_PWM-5))
				{
					m_running = 0;
					PWM_Value = 0;
					CMPCR1 = 0x8c;
					PWMA_ENO = 0;
					PWM1_L = 0;
					PWM2_L = 0;
					PWM3_L = 0;
				}
				PWMA_CCR1L = PWM_Value;
				PWMA_CCR2L = PWM_Value;
				PWMA_CCR3L = PWM_Value;
			}
			
			cap_res_g[j] = pwmb_cap_res;
			j++;
			j &= 0x07;
			for(sum = 0,i = 0;i<8;i++)
			{
				sum += cap_res_g[i];
			}
			cap_res_lp = sum>>3;
			if(cap_res_lp<1100)
			{
				PWM_Set = 25;
			}
			else if(cap_res_lp>1900)
			{
				PWM_Set = 230;
			}
			else if((cap_res_lp>=1100) && (cap_res_lp <= 1900))
			{
				PWM_Set = (u8)((cap_res_lp - 1000)>>2);
			}
			
		}
	}
}

void Port_Init(void)
{
	P0M0 = 0x00;
	P0M1 = 0x00; //P0�˿ڳ�ʼ��Ϊ˫���
	P1M0 = 0x00;
	P1M1 = 0x00; //P1�˿ڳ�ʼ��Ϊ˫���
	P2M0 = 0x00;
	P2M1 = 0x00; //P2�˿ڳ�ʼ��Ϊ˫���
	P3M0 = 0x00;
	P3M1 = 0x00; //P3�˿ڳ�ʼ��Ϊ˫���
	P5M0 = 0x00;
	P5M0 = 0x00; //P5�˿ڳ�ʼ��Ϊ˫���
}
