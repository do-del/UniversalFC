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

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#define PWM1 		P10
#define PWM1_L 	P11
#define PWM2		P12
#define PWM2_L	P13
#define PWM3		P14
#define PWM3_L	P15

void Port_Init(void);	//оƬ��λ�����ų�ʼ��
void PWMA_Config(void);	//PWMA���ú���
void ADC_Config(void);
void CMP_Config(void);
void Timer0_Config(void);
void Timer3_Config(void);
void Timer4_Config(void);

void main(void)
{
	P_SW2 |= 0x80; //ʹ��XFR
	
	Port_Init();
	PWMA_Config();
	ADC_Config();
	CMP_Config();
	Timer0_Config();
	Timer3_Config();
	Timer4_Config();
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

void PWMA_Config(void)
{
	//�Ƚ�MOS��ѡͨ�ź����ͣ���ֹ��ͨ
	PWM1 = 0;
	PWM1_L = 0;
	PWM2 = 0;
	PWM2_L = 0;
	PWM3 = 0;
	PWM3_L = 0;
	
	//����ѡͨ����Ϊ�������
	P1M0 |= 0x3f; //0x3f = 0B0011 1111��bit0~bit5��1
	P1M1 &= ~0x3f; //~0x3f = 0B1100 0000,bit0~bit5��0
	
	PWMA_PSCR = 3;	//PWMA_PSCRΪPWMA��16λԤ��Ƶ���Ĵ���������16λ���ݶ�д��f_ck_int = f_ck_psc/(PSCR[15:0]+1)
	PWMA_DTR = 24;	//PWMA_DTRλPWMA�������Ĵ�����������������ʱ�䣬������������������δ��
	
	PWMA_ARR = 255;	//PWMA_ARRΪPWMA��16λ�Զ����ؼĴ���
	PWMA_CCER1 = 0;	//����/�Ƚ�ʹ�ܼĴ���1�����ü��Լ����ʹ��
	PWMA_CCER2 = 0;	//����/�Ƚ�ʹ�ܼĴ���2�����ü��Լ����ʹ��
	PWMA_SR1 = 0;		//״̬�Ĵ���1���жϱ��
	PWMA_SR2 = 0;		//״̬�Ĵ���2���ظ�������
	PWMA_ENO = 0;		//���ʹ�ܼĴ���
	PWMA_PS = 0;		//���ܽ��л�
	PWMA_IER = 0;		//�ж�ʹ�ܼĴ���
	
	PWMA_CCMR1 = 0x68;	//����/�Ƚ�ģʽ�Ĵ���1��0x68 = 0b0110 1000����Ϊ���������Ԥװ�أ�PWMģʽ1
	PWMA_CCR1 = 0;			//����/�ȽϼĴ�����16λ����ǰ�Ƚ�ֵ
	PWMA_CCER1 |= 0x05;	//0x05 = 0b0000 0101
	PWMA_PS |= 0;
}

void ADC_Config(void);
void CMP_Config(void);
void Timer0_Config(void);
void Timer3_Config(void);
void Timer4_Config(void);
