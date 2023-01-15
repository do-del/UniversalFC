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

#define FOSC 24000000UL	//ϵͳʱ��
#define BRT (65536-(FOSC/115200+2)/4) //������ת����ֵ

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#define PWM1 		P10
#define PWM1_L 	P11
#define PWM2		P12
#define PWM2_L	P13
#define PWM3		P14
#define PWM3_L	P15

#define ADC_START (1<<6)	//AD�ӿ��ƼĴ���bit6��ADCת����������λ��д1��ʼADCת��
#define ADC_FLAG (1<<5)	//ADC���ƼĴ���bit5��ADCת��������־λ
#define ADC_SPEED 1			//ADC���üĴ���bit3~bit0������ADC����ʱ��Ƶ��
#define RES_FMT (1<<5)	//ADC���üĴ���bit5����1����ADC����Ҷ��룬��0����ADC��������
#define CSSETUP (0<<7)	//ADCTIMʱ����ƼĴ���ͨ��ѡ��ʱ�����λbit7��ռ��һ��ʱ����
#define CSHOLD (1<<5)		//ADCTIMʱ����ƼĴ���ͨ������ʱ�����λbit6~bit5��ռ��2��ʱ����
#define SMPDUTY 20			//ADCTIMʱ����ƼĴ���ͨ������ʱ�����λbit4~bit0��ռ��21��ʱ����������С��10

#define RX_LEN 16 //���ڽ������ݻ��泤��
bit busy;		//���ڷ�����ɱ�־
char wptr;	//���ڽ������ݳ���
char buffer[RX_LEN]; //���ڽ������ݻ���

void Port_Init(void);	//оƬ��λ�����ų�ʼ��
void PWMA_Config(void);	//PWMA���ú���
void ADC_Config(void);	//ADC���ú���
void CMP_Config(void);	//�Ƚ������ú���
void Timer0_Config(void);	//��ʱ��0���ú������������ѿ�����ʱ��0�����жϣ�4msһ�Σ�
void Timer3_Config(void);	//��ʱ��3���ú���
void Timer4_Config(void);	//��ʱ��4���ú���

#define delay_200ns() do{_nop_();_nop_();_nop_();_nop_();}while(0) //����MOS���ֲ��������ʱ�䣬�ֲ��õ�MOS�ܵ�ͨ�ض�ʱ�����Ϊ55ns���˴�����Ϊ200ns��ʱ
#define delay_dead() delay_200ns()
u16 Get_ADCRes(u8 ch);	//��ȡADCָ��ͨ���������
void Motor_Start(void); //�����������
void Motor_Step(void); //������ຯ��

void	Delay_n_ms(u8 dly)	// N ms��ʱ����
{
	u16	j;
	do
	{
		j = FOSC / 10000;
		while(--j)	;
	}while(--dly);
}

void delay_us(u8 us)	//N us��ʱ����
{
	do
	{
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();	//@24MHz
	}
	while(--us);
}

//�����ú���
void Uart_Config(void); //���ڳ�ʼ��
void UartSend(char dat); //���ڷ����ֽ�
void UartSendStr(char *p); //���ڷ����ַ���
void UartSendNum(int num); //���ڷ���-32767~32768��Χ�ڵ�����

u8 step; //�������ͨ����ţ��������࣬��Χ0~5
bit m_starting;	//�������������־
bit m_running;	//����������б�־
u8 demagnetizing_cnt;	//���ż���ֵ��1Ϊ��Ҫ���ţ�2Ϊ�������ţ�0Ϊ�Ѿ����ţ���ʼ��Ϊ0��������к�Ƚ����жϼ������¼�����1���ڼ����û���ʱ�䣩���ڶ�ʱ��4�жϻ�����������
bit t3_flag; //Timer3�����־��32.768ms��1һ��
bit t0_flag; //Timer0�����־��4ms��1һ��
#define TMP_LEN 8
u8 time_index;
u16 phase_time_tmp[TMP_LEN];
u8 timeout;
u8	PWM_Value;	// ����PWMռ�ձȵ�ֵ
u8	PWM_Set;	//Ŀ��PWM����
#define	D_START_PWM		30

//�����ò���
bit cmp_it_flag;
u16 debug_phase_time;
bit t4_it_flag1;
bit t4_it_flag2;

void UartIsr() interrupt 4
{
	if(TI)
	{
		TI = 0;
		busy = 0;
	}
	if(RI)
	{
		RI = 0;
		buffer[wptr++] = SBUF;
		if(SBUF == '\n')
			wptr |= 0x80;
	}
}

void UartRxTest(void)
{
	if(wptr & 0x80)
	{
		unsigned char len = wptr & 0x7f;
		u8 i;
		UartSendStr(buffer);
		for(i = 0;i<len;i++)
			buffer[i] = 0;
		wptr = 0;
	}
}

void main(void)
{
	u8	i;
	u16	j;
	
	P_SW2 |= 0x80; //ʹ��XFR
	
	Port_Init();	//���ö˿ڳ�ʼ������
	PWMA_Config();	//����PWMA��ʼ������
	ADC_Config();	//����ADC��ʼ������
	CMP_Config();	//����ģ��Ƚ�����ʼ������
	Timer0_Config();	//���ö�ʱ��0��ʼ������
	Timer3_Config();	//���ö�ʱ��3��ʼ������
	Timer4_Config();	//���ö�ʱ��4��ʼ������
	
	Uart_Config();	//���ô��ڳ�ʼ������
	ES = 1;
	
	PWM_Set = 100;
	timeout = 0;
	
	EA = 1;	//�����ж�
	
	UartSendStr("--Brushless ESC Test--\r\n");
	
	//PWMA_ENO = 0x15; //0x15 = 0b0001 0101������PWMA���ܣ�ʹ��PWM1P��PWM2P��PWM3P���
	
	while(1)
	{
		if(t0_flag)
		{
			t0_flag = 0;
			if(timeout != 0)
			{
				if(--timeout == 0)
				{
					UartSendStr("timeout\r\n");
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
				UartSendStr("-ST-\r\n");
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
				if(PWM_Value < PWM_Set) PWM_Value++;
				if(PWM_Value > PWM_Set) PWM_Value--;
				PWMA_CCR1 = PWM_Value;
				PWMA_CCR2 = PWM_Value;
				PWMA_CCR3 = PWM_Value;
			}
			if(cmp_it_flag)
			{
				cmp_it_flag = 0;
				UartSendStr("-cmp it-\r\n");
				UartSendStr("ph:");
				UartSendNum(debug_phase_time);
				UartSendStr("\r\n");
			}
			if(t4_it_flag1)
			{
				t4_it_flag1 = 0;
				UartSendStr("-t4 it 1-\r\n");
			}
			if(t4_it_flag2)
			{
				t4_it_flag2 = 0;
				UartSendStr("-t4 it 2-\r\n");
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

void PWMA_Config(void)
{
	//�Ƚ�MOS��ѡͨ�ź����ͣ���ֹ��ͨ
	PWM1 = 0;
	PWM1_L = 0;
	PWM2 = 0;
	PWM2_L = 0;
	PWM3 = 0;
	PWM3_L = 0;
	
	//����ѡͨ����Ϊ���������M0��ӦbitΪ1��M1��ӦbitΪ0
	P1M0 |= 0x3f; //0x3f = 0B0011 1111��bit0~bit5��1
	P1M1 &= ~0x3f; //~0x3f = 0B1100 0000,bit0~bit5��0
	
	PWMA_PSCR = 3;	//PWMA_PSCRΪPWMA��16λԤ��Ƶ���Ĵ���������16λ���ݶ�д��f_ck_int = f_ck_psc/(PSCR[15:0]+1)
	PWMA_DTR = 24;	//PWMA_DTRλPWMA�������Ĵ�����������������ʱ�䣬������������������δ��
	
	PWMA_ARR = 0xff;	//PWMA_ARRΪPWMA��16λ�Զ����ؼĴ���
	PWMA_CCER1 = 0;	//����/�Ƚ�ʹ�ܼĴ���1�����ü��Լ����ʹ��
	PWMA_CCER2 = 0;	//����/�Ƚ�ʹ�ܼĴ���2�����ü��Լ����ʹ��
	PWMA_SR1 = 0;		//״̬�Ĵ���1���жϱ��
	PWMA_SR2 = 0;		//״̬�Ĵ���2���ظ�������
	PWMA_ENO = 0;		//���ʹ�ܼĴ���
	PWMA_PS = 0;		//���ܽ��л�
	PWMA_IER = 0;		//�ж�ʹ�ܼĴ���
	
	PWMA_CCMR1 = 0x68;	//����/�Ƚ�ģʽ�Ĵ���1��0x68 = 0b0110 1000����Ϊ���������Ԥװ�أ�PWMģʽ1
	PWMA_CCR1 = 0x00;			//����/�ȽϼĴ�����16λ����ǰ�Ƚ�ֵ
	PWMA_CCER1 |= 0x01;	//����/�Ƚ�ʹ�ܼĴ���1��ԭ0x05 = 0b0000 0101,�ָ�Ϊ0x01��ʹ��OC1���
	PWMA_PS |= 0;				//PWMA IOѡ��
	
	PWMA_CCMR2 = 0x68;	//����/�Ƚ�ģʽ�Ĵ���2��0x68 = 0b0110 1000����Ϊ���������Ԥװ�أ�PWMģʽ1
	PWMA_CCR2 = 0x00;			//����/�ȽϼĴ�����16λ����ǰ�Ƚ�ֵ����PWMA_ARR�Ƚ�
	PWMA_CCER1 |= 0x10;	//����/�Ƚ�ʹ�ܼĴ���1��ԭ0x50 = 0b0101 0000���ָ�Ϊ0x10��ʹ��OC2���
	PWMA_PS |= (0<<2);	//PWMA IOѡ��
	
	PWMA_CCMR3 = 0x68;	//����/�Ƚ�ģʽ�Ĵ���3��0x68 = 0b0110 1000����Ϊ���������Ԥװ�أ�PWMģʽ1
	PWMA_CCR3 = 0x00;			//����/�ȽϼĴ�����16λ����ǰ�Ƚ�ֵ
	PWMA_CCER2 |= 0x01;	//����/�Ƚ�ʹ�ܼĴ���2��ԭ0x05 = 0b0000 0101,�ָ�Ϊ0x01��ʹ��OC3���
	PWMA_PS |= (0<<4);	//PWMA IOѡ��
	
	PWMA_BKR = 0x80;	//ʹ��OC��OCN���
	PWMA_CR1 = 0x81;	//���ƼĴ���1��ʹ���Զ�Ԥװ�أ����ض��룬���ϼ�����ʹ�ܼ�����
	PWMA_EGR = 0x01;	//��ʼ��������
}

void ADC_Config(void)
{
	//��������ģʽΪ�������룬M1��ӦbitΪ1��M0��ӦbitΪ0
	P1M1 |= 0xc0; //��־λ��1
	P1M0 &= ~0xc0; //��־λ��0
	
	//��������ģʽΪ�������룬M1��ӦbitΪ1��M0��ӦbitΪ0
	P0M1 |= 0x0f;  //��־λ��1
	P0M0 &= ~0x0f; //��־λ��0
	
	ADC_CONTR = 0x80+6; //ADC���ƼĴ�����bit7��1��ADC��Դ��bit3~bit0Ϊͨ��ѡ��λ��STC8H1K28ϵ����ADC12~ADC14.STC8H1K08ϵ����ADC2~ADC7
	ADCCFG = RES_FMT + ADC_SPEED; //ADC���üĴ�����bit5�����ʽ���ƣ���0����룬��1�Ҷ��룬bit3~bit0����ADCʱ��Ƶ�ʣ�f=Sysclk/2/(speed+1)
	ADCTIM = CSSETUP + CSHOLD + SMPDUTY;	//ADCʱ����ƼĴ�����bit7��T_setup | bit6~bit5��T_hold | bit4~bit0��T_duty
}

void CMP_Config(void)
{
	//����CMP-����Ϊ�������룬M1��ӦbitΪ1��M0��ӦbitΪ0
	P3M1 |= 0x40; //0x40 = 0b0100 0000,bit6��1
	P3M0 &= ~0x40; //~0x40 = 0b1011 1111��bit6��0
	
	CMPCR1 = 0x8C;	//�Ƚ������ƼĴ���1��bit7ģ��ʹ�ܣ�bit6�жϱ�־λ��bit5�������ж�ʹ�ܣ�bit4�½����ж�ʹ��
									//bit3����ѡ��bit2����ѡ��bit1���������ƣ�bit0�ȽϽ��
									//0x80 = 0b1000 1100��ʹ�ܱȽ�����ADCΪ�������룬P3.6Ϊ��������
	CMPCR2 = 60; //�Ƚ������ƼĴ���2��bit7���������ƣ�bit6ģ���˲����ƣ���0ʹ�ܣ�bit5~bit0�˲�ʱ����
}

void Timer0_Config(void)
{
	TMOD &= ~0x03;		//��ʱ��0/1ģʽ�Ĵ�����~0x03 = 0b1111 1100��bit1~bit0���㣬��ʱ��0����Ϊ16λ�Զ�����ģʽ
	AUXR &= ~(1<<7);	//�����Ĵ���1��bit7��0����ʱ��0ʱ��ΪCPUʱ��12��Ƶ��24MHz/12 = 2MHz
	
	//12��Ƶ�£���ʱ������=(65536-[TH0,TL0])*12/FOSC,��4ms��ʱ��������= 0.004 = 1/250
	TH0 = (65536UL - FOSC / 12 / 250)>>8;	//��ʱ��0�����Ĵ���
	TL0 = (u8)(65536UL - FOSC / 12 / 250); //��ʱ��0�����Ĵ���
	TR0 = 1;	//������ʱ��0
	ET0 = 1;	//ʹ�ܶ�ʱ��0�ж�
}

void Timer3_Config(void)
{
	T4T3M &= 0xf0;	//��ʱ��3/4���ƼĴ�������4λ��ʱ��4����4λ��ʱ��3����4λ���㣬��ʱ��3ֹͣ������������ʱ����12��Ƶ���ر�ʱ�����
	T3H = 0;	//��ʱ��3�����Ĵ���
	T3L = 0;	//��ʱ��3�����Ĵ���
	
	T3T4PIN = 0x01;	//��ʱ��3/4���������л��Ĵ�����bit0ѡ��λ����1ѡ��P0.0��P0.1��P0.2��P0.3
	IE2 |= (1<<5);	//�ж�ʹ�ܼĴ���2��bit5Ϊ��ʱ��3������ж�����λ����1���ж�
	T4T3M |= (1<<3);	//��ʱ��3/4���ƼĴ�����bit3��1����ʱ��3��ʼ����
}

void Timer4_Config(void)
{
	T4T3M &= 0x0f; //��ʱ��3/4���ƼĴ�������4λ��ʱ��4����4λ��ʱ��3����4λ���㣬��ʱ��4ֹͣ������������ʱ����12��Ƶ���ر�ʱ�����
	T4H = 0;	//��ʱ��4�����Ĵ���
	T4L = 0;	//��ʱ��4�����Ĵ���
	
	//T4H = (u8)((65536UL - 40*2) >> 8);	//�����ã���ʱ��4����Ƶ��Ϊ2MHz��40usҪ��80��
	//T4L = (u8)(65536UL - 40*2);	//������
	
	T3T4PIN = 0x01;	//��ʱ��3/4���������л��Ĵ�����bit0ѡ��λ����1ѡ��P0.0��P0.1��P0.2��P0.3
	IE2 |= (1<<6);	//�ж�ʹ�ܼĴ���2��bit6Ϊ��ʱ��4������ж�����λ����1���ж�
	//T4T3M |=  (1<<7);	//��ʼ������������
}

u16 Get_ADCRes(u8 ch)
{
	u8 i = 255; //ADCת����ʱ������ģ����ʱ
	ADC_RES = 0;	//ADCת������Ĵ���������
	ADC_RESL = 0; //ADCת������Ĵ���L������
	ADC_CONTR = 0x80|ADC_START|ch; //ѡ��ADCͨ������ʼת��
	_nop_();
	while(i != 0)
	{
		i--;
		if((ADC_CONTR & ADC_FLAG) != 0)	break;	//�ȴ�ADC����
	}
	ADC_CONTR &= ~ADC_FLAG;	//���ת����ɱ�־λ
	return	((u16)ADC_RES>>8 + (u16)ADC_RESL); //���ض�ȡ���
}

void Motor_Start(void)
{
	u16 timer,i;
	CMPCR1 = 0x8C;	// �رȽ����ж�

	PWM_Value  = D_START_PWM;	// ��ʼռ�ձ�, ���ݵ����������
	PWMA_CCR1L = PWM_Value;
	PWMA_CCR2L = PWM_Value;
	PWMA_CCR3L = PWM_Value;
	step = 0;
	Motor_Step();
	Delay_n_ms(50);
	//Delay_n_ms(250);// ��ʼλ��
	timer = 200;	//���ȵ������

	while(1)
	{
		for(i=0; i<timer; i++)	delay_us(100);  //���ݵ����������, ���ת�ٵȵȵ������������ٶ�
		timer -= timer /16;
		if(++step >= 6)	step = 0;
		Motor_Step();
		if(timer < 40)	return;
	}
}

//���ຯ��
//���ݵ��ͨ����ţ�������ͨ��
void Motor_Step(void)
{
	switch(step)
	{
		case 0: //AB��ͨ�磬�ڼ�C���Ӧ�綯���ɸ������仯��A�Ϲܵ�ͨ��B�¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x00; //�ر�����PWM������ر�����һ��C���Ϲܵ�ͨ�����ȹضϣ�Ȼ����A���Ϲ�
			PWM1_L = 0; //A���¹ܹض�
			PWM3_L = 0; //C���¹ܹض�
			delay_dead(); //�ӳ٣���ֹAC���Ϲ�ͬʱ��ͨ
			PWMA_ENO = 0x01; //��A���Ϲ�PWM
			PWM2_L = 1;	//B���¹ܵ�ͨ
			ADC_CONTR = 0x80+10; //ѡ��ADC10����P0.2������
			if(m_running) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		case 1:	//AC��ͨ�磬�ڼ�B���Ӧ�綯�����������仯��A���Ϲܵ�ͨ��C���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x01;	//A���Ϲܵ�ͨ
			PWM1_L = 0;	//A���¹ܹض�
			PWM2_L = 0; //B���¹ܹض�
			delay_dead(); //�ӳ٣���ֹBC���¹�ͬʱ��ͨ
			PWM3_L = 1;	//C���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 9;	//ѡ��ADC9��P0.1������
			if(m_running) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
			else CMPCR1 = 0x8c;	//�������ʱ�رձȽ����ж�
			break;
		case 2:	//BC��ͨ�磬�ڼ�A���Ӧ�綯���ɸ������仯��B���Ϲܵ�ͨ��C���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x00; //�ر�����PWM������ر�����һ��A���Ϲܵ�ͨ�����ȹضϣ�Ȼ����ܿ���B���Ϲ�
			PWM1_L = 0;	//A���¹ܹض�
			PWM2_L = 0; //B���¹ܹض�
			delay_dead();
			PWMA_ENO = 0x04;	//��B���Ϲ�PWM
			PWM3_L = 1;	//C���¹ܵ�ͨ
			ADC_CONTR = 0x80+8;		//ѡ��ADC8��P0.0������
			if(m_running) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		case 3: //BA��ͨ�磬�ڼ�C���Ӧ�綯�����������仯��B���Ϲܵ�ͨ��A���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x04;	//��B���Ϲܣ��ر�A��C���Ϲ�
			PWM2_L = 0;	//B���¹ܹض�
			PWM3_L = 0; //C���¹ܹض�
			delay_dead();	//������ʱ����AC���¹�����
			PWM1_L = 1;	//A���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 10;	//ѡ��ADC10��P0.2������
			if(m_running) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
			else CMPCR1 = 0x8c;	//�������ʱ�رձȽ����ж�
			break;
		case 4: //CA��ͨ�磬�ڼ�B���Ӧ�綯���ɸ������仯��C���Ϲܵ�ͨ��A���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x00;	//�ر������Ϲ�PWM����ֹBC���Ϲ�ͬʱ��ͨ
			PWM2_L = 0;	//B���¹ܹض�
			PWM3_L = 0;	//C���¹ܹض�
			delay_dead();	//������ʱ����BC���Ϲ�����
			PWMA_ENO = 0x10;	//ʹ��C���Ϲ�PWM
			PWM1_L = 1;	//A���¹ܵ�ͨ
			ADC_CONTR = 0x80+9;	//ADC9��P0.1������
			if(m_running) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c;	//�������ʱ�رձȽ����ж�
			break;
		case 5:	//CB��ͨ�磬�ڼ�A���Ӧ�綯�����������仯��C���Ϲܵ�ͨ��B���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x10;	//ʹ��C���Ϲ�PWM
			PWM1_L = 0;	//A���¹ܹض�
			PWM3_L = 0;	//C���¹ܹض�
			delay_dead();	//������ʱ����AB���¹�����
			PWM2_L = 1;	//B���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 8;	//ADC8��P0.0������
			if(m_running) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		default:
			break;
	}
}

void Uart_Config(void)
{
	SCON = 0x50;
	T2L = BRT;
	T2H = BRT>>8;
	AUXR = 0x15;
	wptr = 0;
	busy = 0;
}

void UartSend(char dat)
{
	while(busy);
	busy = 1;
	SBUF = dat;
}

void UartSendStr(char *p)
{
	while(*p)
	{
		UartSend(*p++);
	}
}

void UartSendNum(int num)
{
	char buf[8];
	u8 dat;
	u8 i = 0;
	if(num<0)
	{
		UartSend('-');
		num = -num;
	}
	while(num)
	{
		dat = num%10;
		buf[i] = dat + 0x30;
		num /= 10;
		i++;
	}
	UartSendStr(buf);
}

//�Ƚ����жϺ���
//��⵽�����¼��󣬸��ݶ�ʱ��3�ļ���ֵ��������һ�ι����¼���ʱ��
//���8�λ�������ʱ�����16������ÿ��λ�������ʱ�䣬��Ϊ���´λ�������ʱ��
void CMP_ISR(void) interrupt 21
{
	u16 phase_time;
	u8 i;
	
	CMPCR1 %= ~0x40; //����жϱ�־λ
	
	if(demagnetizing_cnt == 0) //���ź�������¼���δ���Ų�����Ƚ����ж�
	{
		T4T3M &= ~(1<<3); //Timer3ֹͣ����
		if(t3_flag)
		{
			t3_flag = 0;
			phase_time = 0x1fff;
		}
		else
		{
			phase_time = (((u16)T3H<<8)+(u16)T3L)>>1;
			phase_time &= 0x1ffff;
		}
		T3H = 0;
		T3L = 0;
		T4T3M |= (1<<3);
		
		phase_time_tmp[time_index] = phase_time;
		if(++time_index >=8) time_index = 0;
		for(phase_time = 0,i = 0;i<8;i++)
		{
			phase_time += phase_time_tmp[i];
		}
		phase_time >>= 4;
		debug_phase_time = phase_time;
		if(phase_time > 40 && phase_time < 1000)
		{
			cmp_it_flag = 1;
			timeout = 125;
		}
		if(phase_time > 40) phase_time -= 40;
		else phase_time = 20;
		
		T4T3M &= ~(1<<7);
		phase_time = phase_time << 1;
		phase_time = 0 - phase_time;
		T4H = (u8)(phase_time >> 8);
		T4L = (u8)phase_time;
		T4T3M |= (1<<7);
		demagnetizing_cnt = 1;
	}
}

void Timer0_ISR(void) interrupt 1
{
	//PWM1 = !PWM1; //��ʱ��0�жϲ��ԣ���תPWM1���ŵ�ƽ
	t0_flag = 1;
}

void Timer3_ISR(void) interrupt 19
{
	//PWM2 = !PWM2; //��ʱ��3�зֶβ��ԣ���תPWM2���ŵ�ƽ
	t3_flag = 1;
}

//������������ö�ʱ��4����������ţ�����ԭ��򻯳ɶ���ʱ���˴���Ϊ40us��ʱ������Խ������ʱ��Խ��
//�������󣬶ϵ����һ�࣬������Ȧ�Ĵ��ڻ�����Ըе綯�ƣ��������ţ�������Ƚ����ڷǸ�Ӧ�綯�ƹ���ʱ���ж�
//�˴������޸�demagnetzing_cnt���ż���ֵ����������������ıȽ����ж�
void Timer4_ISR(void) interrupt 20
{
	T4T3M &= ~(1<<7); //��ʱ��3/4���ƼĴ���bit7��0����ʱ��4ֹͣ����
	
	if(demagnetizing_cnt == 1)	//�ж����ż���ֵ��Ϊ1Ϊ��Ҫ����
	{
		demagnetizing_cnt = 2;	//�����ż���ֵ��Ϊ2���Ȼ��࣬�ٽ�������
		if(m_running)
		{
			t4_it_flag1 = 1;
			if(++step >= 6) step = 0;
			Motor_Step();
		}
		
		T4H = (u8)((65536UL - 40*2) >> 8);	//40us������(65536 - t *2)��t��λus����8λ����T4H����8λ����T4L
		T4L = (u8)(65536UL - 40*2);	//40us����
		T4T3M |=  (1<<7);	//Timer4��ʼ����
	}
	else if(demagnetizing_cnt == 2)
	{
		t4_it_flag2 = 1;
		demagnetizing_cnt = 0;
	}
}
