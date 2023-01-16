#ifndef __PWM_H
#define __PWM_H

#define PWM1 		P10
#define PWM1_L 	P11
#define PWM2		P12
#define PWM2_L	P13
#define PWM3		P14
#define PWM3_L	P15

u16 pwmb_cap; //����ֵ��ʱ��ű���
u16 pwmb_cap_up; //�ɼ�����������ʱ��
u16 pwmb_cap_res; //�ߵ�ƽ����ʱ��
//u8 pwmb_count;	//�������ֵ
bit pwmb_up_flag; //���������ر�־

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

//PWMB���ú���
//PWMB_CCMR1����CC8SλΪ1ʹͨ��Ϊ���룬ʹPWMB_CCR1�Ĵ�����Ϊֻ��
//����PWMB_CCMR1�е�IC8Fλ�����˲�ʱ��
//����PWMB_CCER1�е�CC8P����ת������
//����Ԥ��Ƶ����PWMB_CCMR1�Ĵ����е�IC8PSC��0��ֹ
//����PWMB_CCER1�е�CC1E=1�����������ֵ������Ĵ�����
//����PWMB_IER�е�CC8IE��������ж�����
//	��������ʱ����������ֵ�����͵�PWMB_CCR1
//	CC8IF��־����1�������������������������CC8IFδ�����㣬��CC8OFҲ����1
//	������CC8IEλ�������ж�
void PWMB_Config(void)
{
	//��P2.3����Ϊ��������
	P2M1 |= 0x08; //��־λ��1��0x08 = 0b0000 1000
	P2M0 &= ~0x08; //��־λ��0
	P2PU |= 0x08; //�����������裬bit3��1ʹ����������
	
	PWMB_CCER2 = 0x00;
	PWMB_ENO &= ~0x40; //0x0 = 0b0100 0000����־λ���㣬�ر�PWM8���
	PWMB_PSCR = 0x00;
	PWMB_DTR = 0x00;
	PWMB_PS &= 0x3f; //0b0011 1111
	
	PWMB_CCMR3 = 0x32;
	PWMB_CCMR4 = 0x31; //0x31 = 0b0011 0001��bit7~bit4Ϊ�˲���ѡ��11ѡ8��ʱ������bit3~bit2Ԥ��Ƶ����00��Ԥ��Ƶ��������ÿһ�α��أ�bit1~bit0����Ϊ����
	//PWMB_CCER2 = 0x10;//0b0001 0000��bit5ΪCC8P��bit4ΪCC8E�����벶��ģʽ�£�CC8P��1�½��أ���0�����أ�CC8E��1�������벶����0�ر�
	PWMB_CCER2 = 0x11;
	PWMB_CCER2 |= 0x00;	//PWM7������
	PWMB_CCER2 |= 0x20; //PWM8�½��� 
	PWMB_IER |= 0x10; //0b0001 0001������PWM8�жϣ����������ж�
	
	PWMB_CR1 = 0x01; //0b0000 0101��bit1ΪUDISB��������������жϣ��ж�ʹ��
	
}

bit pwmb_it_flag;
u16 dtmp;
u16 uptmp;
void PWMB_ISR(void) interrupt 27
{
	if(PWMB_SR1 & 0x10)
	{
		PWMB_SR1 &=  ~0x10;
		pwmb_it_flag = 1;
		
		pwmb_cap_res = (PWMB_CCR8 - PWMB_CCR7)/24;
	}
}

#endif
