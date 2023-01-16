#include "stc8h.h"
#include <intrins.h>

/*
 * PWM1接P1.0，配置推挽输出和PWM输出，控制电机U相高位MOS管选通信号，高电平导通，低电平截止
 * PWM1_L接P1.1，配置推挽输出，控制电机U相低位MOS管选通信号，高电平导通，低电平截止
 * PWM2接P1.2，配置推挽输出和PWM输出，控制电机V相高位MOS管选通信号，高电平导通，低电平截止
 * PWM2_L接P1.3，配置推挽输出，控制电机V相低位MOS管选通信号，高电平导通，低电平截止
 * PWM3接P1.4，配置推挽输出和PWM输出，控制电机W相高位MOS管选通信号，高电平导通，低电平截止
 * PWM3_L接P1.5，配置推挽输出，控制电机W相低位MOS管选通信号，高电平导通，低电平截止
 * ADC8（P0.0），配置高阻输入，采样电机U相感应电动势，用于过零检测比较器正输入
 * ADC9（P0.1），配置高阻输入，采样电机V相感应电动势
 * ADC10（P0.2），配置高阻输入，采样电机W相感应电动势
 * CMP-（P3.6），配置为高阻输入，接过零检测电路中点
*/

/*
 * 无刷电机通电次序
 * AB-AC-BC-BA-CA-CB-AB
 * 相对应感应电动势变化
 * C下降-B上升-A下降-C上升-B下降-A上升-C下降
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

void Port_Init(void);	//芯片复位后引脚初始化

void main(void)
{
	u8	i;
	u8 j;
	u32 sum;
	
	P_SW2 |= 0x80; //使能XFR
	
	Port_Init();	//调用端口初始化函数
	
	PWMA_Config();	//调用PWMA初始化函数
	ADC_Config();	//调用ADC初始化函数
	CMP_Config();	//调用模拟比较器初始化函数
	Timer0_Config();	//调用定时器0初始化函数
	Timer3_Config();	//调用定时器3初始化函数
	Timer4_Config();	//调用定时器4初始化函数

	PWMB_Config();
	
	Uart_Config();	//调用串口初始化函数
	ES = 1;
	
	PWM_Set = 0;
	PWM_Value = 0;
	timeout = 0;
	
	Delay_n_ms(250);
	beep_1KHz(500);
	Delay_n_ms(250);
	
	EA = 1;	//打开总中断
	
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
				if(step & 1)	CMPCR1 = 0xAC;		//上升沿中断
				else			CMPCR1 = 0x9C;		//下降沿中断
				m_running = 1;
				Delay_n_ms(250);	//延时一下, 先启动起来
				Delay_n_ms(250);
				timeout = 125;		//启动超时时间 125*4 = 500ms
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
	P0M1 = 0x00; //P0端口初始化为双向口
	P1M0 = 0x00;
	P1M1 = 0x00; //P1端口初始化为双向口
	P2M0 = 0x00;
	P2M1 = 0x00; //P2端口初始化为双向口
	P3M0 = 0x00;
	P3M1 = 0x00; //P3端口初始化为双向口
	P5M0 = 0x00;
	P5M0 = 0x00; //P5端口初始化为双向口
}
