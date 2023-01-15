#define MAIN_Fosc		24000000L	//������ʱ��

#include	"STC8Hxxx.h"


#define ADC_START	(1<<6)	/* �Զ���0 */
#define ADC_FLAG	(1<<5)	/* �����0 */

#define	ADC_SPEED	1		/* 0~15, ADCʱ�� = SYSclk/2/(n+1) */
#define	RES_FMT		(1<<5)	/* ADC�����ʽ 0: �����, ADC_RES: D9 D8 D7 D6 D5 D4 D3 D2, ADC_RESL: D1 D0 0  0  0  0  0  0 */
							/*             1: �Ҷ���, ADC_RES: 0  0  0  0  0  0  D9 D8, ADC_RESL: D7 D6 D5 D4 D3 D2 D1 D0 */

#define CSSETUP		(0<<7)	/* 0~1,  ADCͨ��ѡ��ʱ��      0: 1��ADCʱ��, 1: 2��ADCʱ��,  Ĭ��0(Ĭ��1��ADCʱ��) */
#define CSHOLD		(1<<5)	/* 0~3,  ADCͨ��ѡ�񱣳�ʱ��  (n+1)��ADCʱ��, Ĭ��1(Ĭ��2��ADCʱ��)                */
#define SMPDUTY		20		/* 10~31, ADCģ���źŲ���ʱ��  (n+1)��ADCʱ��, Ĭ��10(Ĭ��11��ADCʱ��)				*/
							/* ADCת��ʱ��: 10λADC�̶�Ϊ10��ADCʱ��, 12λADC�̶�Ϊ12��ADCʱ��. 				*/

sbit PWM1   = P1^0;
sbit PWM1_L = P1^1;
sbit PWM2   = P1^2;
sbit PWM2_L = P1^3;
sbit PWM3   = P1^4;
sbit PWM3_L = P1^5;

u8	step;		//�л�����
u8	PWM_Value;	// ����PWMռ�ձȵ�ֵ
bit	B_RUN;		//���б�־
u8	PWW_Set;	//Ŀ��PWM����
bit	B_4ms;		//4ms��ʱ��־

u8	TimeOut;	//��ת��ʱ
bit	B_start;	//����ģʽ
bit	B_Timer3_OverFlow;

u8	TimeIndex;		//����ʱ�䱣������
u16	PhaseTimeTmp[8];//8������ʱ��, �� sum/16 ����30�ȵ�Ƕ�
u16	PhaseTime;		//����ʱ�����
u8	XiaoCiCnt;		//1:��Ҫ����, 2:��������, 0�Ѿ�����


/*************************/

void	Delay_n_ms(u8 dly)	// N ms��ʱ����
{
	u16	j;
	do
	{
		j = MAIN_Fosc / 10000;
		while(--j)	;
	}while(--dly);
}

void delay_us(u8 us)	//N us��ʱ����
{
	do
	{
		NOP(20);	//@24MHz
	}
	while(--us);
}

void	Delay_500ns(void)
{
	NOP(6);
}

#define delay_200ns() do{_nop_();_nop_();_nop_();_nop_();}while(0) //����MOS���ֲ��������ʱ�䣬�ֲ��õ�MOS�ܵ�ͨ�ض�ʱ�����Ϊ55ns���˴�����Ϊ200ns��ʱ
#define delay_dead() delay_200ns()

void StepMotor(void) // �������к���
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
			if(B_RUN) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		case 1:	//AC��ͨ�磬�ڼ�B���Ӧ�綯�����������仯��A���Ϲܵ�ͨ��C���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x01;	//A���Ϲܵ�ͨ
			PWM1_L = 0;	//A���¹ܹض�
			PWM2_L = 0; //B���¹ܹض�
			delay_dead(); //�ӳ٣���ֹBC���¹�ͬʱ��ͨ
			PWM3_L = 1;	//C���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 9;	//ѡ��ADC9��P0.1������
			if(B_RUN) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
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
			if(B_RUN) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		case 3: //BA��ͨ�磬�ڼ�C���Ӧ�綯�����������仯��B���Ϲܵ�ͨ��A���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x04;	//��B���Ϲܣ��ر�A��C���Ϲ�
			PWM2_L = 0;	//B���¹ܹض�
			PWM3_L = 0; //C���¹ܹض�
			delay_dead();	//������ʱ����AC���¹�����
			PWM1_L = 1;	//A���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 10;	//ѡ��ADC10��P0.2������
			if(B_RUN) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
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
			if(B_RUN) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c;	//�������ʱ�رձȽ����ж�
			break;
		case 5:	//CB��ͨ�磬�ڼ�A���Ӧ�綯�����������仯��C���Ϲܵ�ͨ��B���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x10;	//ʹ��C���Ϲ�PWM
			PWM1_L = 0;	//A���¹ܹض�
			PWM3_L = 0;	//C���¹ܹض�
			delay_dead();	//������ʱ����AB���¹�����
			PWM2_L = 1;	//B���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 8;	//ADC8��P0.0������
			if(B_RUN) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		default:
			break;
	}
}



void PWMA_config(void)
{
	P_SW2 |= 0x80;		//SFR enable   
	
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

//	PWMA_PS   = (0<<6)+(0<<4)+(0<<2)+0;	//ѡ��IO, 4��Ӹߵ���(������)��ӦPWM1 PWM2 PWM3 PWM4, 0:ѡ��P1.x, 1:ѡ��P2.x, 2:ѡ��P6.x, 
//  PWMA_PS    PWM4N PWM4P    PWM3N PWM3P    PWM2N PWM2P    PWM1N PWM1P
//    00       P1.7  P1.6     P1.5  P1.4     P1.3  P1.2     P1.1  P1.0
//    01       P2.7  P2.6     P2.5  P2.4     P2.3  P2.2     P2.1  P2.0
//    02       P6.7  P6.6     P6.5  P6.4     P6.3  P6.2     P6.1  P6.0
//    03       P3.3  P3.4      --    --       --    --       --    --


void ADC_config(void)	//ADC��ʼ������(Ϊ��ʹ��ADC��������Ƚ����ź�, ʵ��û������ADCת��)
{
	P1n_pure_input(0xc0);	//����Ϊ��������
	P0n_pure_input(0x0f);	//����Ϊ��������
	ADC_CONTR = 0x80 + 6;	//ADC on + channel
	ADCCFG = RES_FMT + ADC_SPEED;
	P_SW2 |=  0x80;	//����XSFR
	ADCTIM = CSSETUP + CSHOLD + SMPDUTY;
}

void CMP_config(void)	//�Ƚ�����ʼ������
{
	CMPCR1 = 0x8C;			// 1000 1100 �򿪱Ƚ�����P3.6��Ϊ�Ƚ����ķ�������ˣ�ADC������Ϊ������� 
	CMPCR2 = 60;			//60��ʱ���˲�   �ȽϽ���仯��ʱ������, 0~63
	P3n_pure_input(0x40);	//CMP-(P3.6)����Ϊ����.
	
	P_SW2 |= 0x80;		//SFR enable   
//	CMPEXCFG |= (0<<6);	//bit7 bit6: �Ƚ�����������ѡ��: 0: 0mV,  1: 10mV, 2: 20mV, 3: 30mV
//	CMPEXCFG |= (0<<2);	//bit2: ���븺����ѡ��, 0: ѡ��P3.6������,   1: ѡ���ڲ�BandGap��ѹBGv��������.
//	CMPEXCFG |=  0;		//bit1 bit0: ����������ѡ��, 0: ѡ��P3.7������,   1: ѡ��P5.0������,  2: ѡ��P5.1������,  3: ѡ��ADC����(��ADC_CHS[3:0]��ѡ���ADC�������������).
//	CMPEXCFG = (0<<6)+(0<<2)+3;
}

void CMP_ISR(void) interrupt 21		//�Ƚ����жϺ���, ��⵽���綯�ƹ�0�¼�
{
	u8	i;
	CMPCR1 &= ~0x40;	// ���������жϱ�־λ

	if(XiaoCiCnt == 0)	//���ź�ż���0�¼�,   XiaoCiCnt=1:��Ҫ����, =2:��������, =0�Ѿ�����
	{
		T4T3M &= ~(1<<3);	// Timer3ֹͣ����
		if(B_Timer3_OverFlow)	//�л�ʱ����(Timer3)�����
		{
			B_Timer3_OverFlow = 0;
			PhaseTime = 8000;	//����ʱ�����8ms, 2212���12V��ת�����130us�л�һ��(200RPS 12000RPM), 480mA
		}
		else
		{
			PhaseTime = (((u16)T3H << 8) + T3L) >> 1;	//��λΪ1us
			if(PhaseTime >= 8000)	PhaseTime = 8000;	//����ʱ�����8ms, 2212���12V��ת�����130us�л�һ��(200RPS 12000RPM), 480mA
		}
		T3H = 0;	T3L = 0;
		T4T3M |=  (1<<3);	//Timer3��ʼ����

		PhaseTimeTmp[TimeIndex] = PhaseTime;	//����һ�λ���ʱ��
		if(++TimeIndex >= 8)	TimeIndex = 0;	//�ۼ�8��
		for(PhaseTime=0, i=0; i<8; i++)	PhaseTime += PhaseTimeTmp[i];	//��8�λ���ʱ���ۼӺ�
		PhaseTime = PhaseTime >> 4;		//��8�λ���ʱ���ƽ��ֵ��һ��, ��30�ȵ�Ƕ�
		if((PhaseTime >= 40) && (PhaseTime <= 1000))	TimeOut = 125;	//��ת500ms��ʱ
		if( PhaseTime >= 60)	PhaseTime -= 40;	//���������˲�����������ͺ�ʱ��
		else					PhaseTime  = 20;
		
	//	PhaseTime = 20;	//ֻ��20us, �����ͺ�����, ���ڼ���˲�����������ͺ�ʱ��
		T4T3M &= ~(1<<7);				//Timer4ֹͣ����
		PhaseTime  = PhaseTime  << 1;	//2������1us
		PhaseTime = 0 - PhaseTime;
		T4H = (u8)(PhaseTime >> 8);		//װ��30�Ƚ���ʱ
		T4L = (u8)PhaseTime;
		T4T3M |=  (1<<7);	//Timer4��ʼ����
		XiaoCiCnt = 1;		//1:��Ҫ����, 2:��������, 0�Ѿ�����
	}
}

void Timer0_config(void)	//Timer0��ʼ������
{
	Timer0_16bitAutoReload(); // T0������16λ�Զ���װ
	Timer0_12T();
	TH0 = (65536UL-MAIN_Fosc/12 / 250) / 256; //4ms
	TL0 = (65536UL-MAIN_Fosc/12 / 250) % 256;
	TR0 = 1; // �򿪶�ʱ��0

	ET0 = 1;// ����ET0�ж�
}

void Timer0_ISR(void) interrupt 1	//Timer0�жϺ���, 20us
{
	B_4ms = 1;	//4ms��ʱ��־
}

//============================ timer3��ʼ������ ============================================
void	Timer3_Config(void)
{
	P_SW2 |= 0x80;		//SFR enable   
	T4T3M &= 0xf0;		//ֹͣ����, ��ʱģʽ, 12Tģʽ, �����ʱ��
	T3H = 0;
	T3L = 0;

	T3T4PIN = 0x01;		//ѡ��IO, 0x00: T3--P0.4, T3CLKO--P0.5, T4--P0.6, T4CLKO--P0.7;    0x01: T3--P0.0, T3CLKO--P0.1, T4--P0.2, T4CLKO--P0.3;
	IE2   |=  (1<<5);	//�����ж�
	T4T3M |=  (1<<3);	//��ʼ����
}

//============================ timer4��ʼ������ ============================================
void	Timer4_Config(void)
{
	P_SW2 |= 0x80;		//SFR enable   
	T4T3M &= 0x0f;		//ֹͣ����, ��ʱģʽ, 12Tģʽ, �����ʱ��
	T4H = 0;
	T4L = 0;

	T3T4PIN = 0x01;		//ѡ��IO, 0x00: T3--P0.4, T3CLKO--P0.5, T4--P0.6, T4CLKO--P0.7;    0x01: T3--P0.0, T3CLKO--P0.1, T4--P0.2, T4CLKO--P0.3;
	IE2   |=  (1<<6);	//�����ж�
//	T4T3M |=  (1<<7);	//��ʼ����
}

//=========================== timer3�жϺ��� =============================================
void timer3_ISR (void) interrupt TIMER3_VECTOR
{
	B_Timer3_OverFlow = 1;	//�����־
}

//=========================== timer4�жϺ��� =============================================
void timer4_ISR (void) interrupt TIMER4_VECTOR
{
	T4T3M &= ~(1<<7);	//Timer4ֹͣ����
	if(XiaoCiCnt == 1)		//�����Ҫ����. ÿ�μ�⵽��0�¼����һ���ж�Ϊ30�Ƚ���ʱ, ����������ʱ.
	{
		XiaoCiCnt = 2;		//1:��Ҫ����, 2:��������, 0�Ѿ�����
		if(B_RUN)	//�����������
		{
			if(++step >= 6)	step = 0;
			StepMotor();
		}
										//����ʱ��, �������Ȧ(���)������С��0�Ĺ�����, ���ַ��綯��, ����Խ������ʱ��Խ��, ��0���Ҫ�����ʱ��֮��
										//100%ռ�ձ�ʱʩ�ӽ��ظ���, �����������, ����ʾ����������ʱ��.
										//ʵ����, ֻҪ�ڻ������ʱ��ʮus�ż�����, �Ϳ�����
		T4H = (u8)((65536UL - 40*2) >> 8);	//װ��������ʱ
		T4L = (u8)(65536UL - 40*2);
		T4T3M |=  (1<<7);	//Timer4��ʼ����
	}
	else if(XiaoCiCnt == 2)	XiaoCiCnt = 0;		//1:��Ҫ����, 2:��������, 0�Ѿ�����
}


#define	D_START_PWM		30
/******************* ǿ�Ƶ���������� ***************************/
void StartMotor(void)
{
	u16 timer,i;
	CMPCR1 = 0x8C;	// �رȽ����ж�

	PWM_Value  = D_START_PWM;	// ��ʼռ�ձ�, ���ݵ����������
	PWMA_CCR1L = PWM_Value;
	PWMA_CCR2L = PWM_Value;
	PWMA_CCR3L = PWM_Value;
	step = 0;	StepMotor();	Delay_n_ms(50);	//Delay_n_ms(250);// ��ʼλ��
	timer = 200;	//���ȵ������

	while(1)
	{
		for(i=0; i<timer; i++)	delay_us(100);  //���ݵ����������, ���ת�ٵȵȵ������������ٶ�
		timer -= timer /16;
		if(++step >= 6)	step = 0;
		StepMotor();
		if(timer < 40)	return;
	}
}

/**********************************************/
void main(void)
{
	u8	i;
	
	P2n_standard(0xf8);
	P3n_standard(0xbf);
	P5n_standard(0x10);
	
	PWMA_config();
	ADC_config();
	CMP_config();
	Timer0_config();	// Timer0��ʼ������
	Timer3_Config();	// Timer3��ʼ������
	Timer4_Config();	// Timer4��ʼ������
	PWW_Set = 0;
	TimeOut = 0;

	EA  = 1; // �����ж�

	while (1)
	{
		if(B_4ms)		// 4msʱ϶
		{
			B_4ms = 0;

			if(TimeOut != 0)
			{
				if(--TimeOut == 0)	//��ת��ʱ
				{
					B_RUN  = 0;
					PWM_Value = 0;
					CMPCR1 = 0x8C;	// �رȽ����ж�
					PWMA_ENO  = 0;
					PWMA_CCR1L = 0;	PWMA_CCR2L = 0;	PWMA_CCR3L = 0;
					PWM1_L=0;	PWM2_L=0;	PWM3_L=0;
					Delay_n_ms(250);	//��תʱ,��ʱһ��ʱ��������
				}
			}

			if(!B_RUN && (PWW_Set >= D_START_PWM))	// ռ�ձȴ����趨ֵ, ���ҵ��δ����, ���������
			{
				B_start = 1;		//����ģʽ
				for(i=0; i<8; i++)	PhaseTimeTmp[i] = 400;
				StartMotor();		// �������
				B_start = 0;
				XiaoCiCnt = 0;		//��ʼ����ʱ
				CMPCR1 &= ~0x40;	// ����жϱ�־λ
				if(step & 1)	CMPCR1 = 0xAC;		//�������ж�
				else			CMPCR1 = 0x9C;		//�½����ж�
				B_RUN = 1;
				Delay_n_ms(250);	//��ʱһ��, ����������
				Delay_n_ms(250);
				TimeOut = 125;		//������ʱʱ�� 125*4 = 500ms
			}

			if(B_RUN)	//����������
			{
				if(PWM_Value < PWW_Set)	PWM_Value++;	//���Ÿ����λ��
				if(PWM_Value > PWW_Set)	PWM_Value--;
				if(PWM_Value < (D_START_PWM-10))	// ͣת, ͣתռ�ձ� �� ����ռ�ձ� С10/256
				{
					B_RUN = 0;
					PWM_Value = 0;
					CMPCR1 = 0x8C;	// �رȽ����ж�
					PWMA_ENO  = 0;
					PWMA_CCR1L = 0;	PWMA_CCR2L = 0;	PWMA_CCR3L = 0;
					PWM1_L=0;	PWM2_L=0;	PWM3_L=0;
				}
				else
				{
					PWMA_CCR1L = PWM_Value;
					PWMA_CCR2L = PWM_Value;
					PWMA_CCR3L = PWM_Value;
				}
			}
			else
			{
				//adc11 = ((adc11 *7)>>3) + Get_ADC10bitResult(11);
			}	
			PWW_Set = 100;
		}
	}
}