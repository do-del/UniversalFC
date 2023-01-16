#ifndef __ENCODER_H
#define __ENCODER_H

#define EC_A P32 //INT0
#define EC_B P33 //INT1
#define EC_KEY P55

bit encoder_flag; //�������¼���־λ
bit up_flag;	//���ϲ�����־λ
char count; //����������ֵ���������������жϺ�δ�������ۼƼ���ֵ

void INT0_Isr() interrupt 0
{
	bit a,b;
	a = P32;
	b = P33;
	if(!encoder_flag)
	{
		encoder_flag = 1;
		count = 0;
	}
	if(a != b)
	{
		count++;
		if(count>100) count = 100;
		up_flag = 1;
	}
	else
	{
		up_flag = 0;
		count--;
		if(count<-100) count = -100;
	}

}

void Int0_Init()
{
	IT0 = 0; //ʹ��INT0�½��غ��������ж�
	EX0 = 1; //ʹ��INT0�ж�
}

#endif
