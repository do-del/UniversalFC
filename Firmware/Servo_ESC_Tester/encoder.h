#ifndef __ENCODER_H
#define __ENCODER_H

#define EC_A P32 //INT0
#define EC_B P33 //INT1
#define EC_KEY P55

bit encoder_flag; //�������¼���־λ
bit up_flag;	//���ϲ�����־λ
char encoder_counter; //����������ֵ���������������жϺ�δ�������ۼƼ���ֵ

void INT0_Isr() interrupt 0
{
	if(!encoder_flag)
	{
		encoder_flag = 1;
		//encoder_counter = 0;
	}
	else
	{
		if(P33)
		{
			up_flag = 1;
			encoder_counter++;
		}
		else
		{
			up_flag = 0;
			encoder_counter--;
		}
	}
}

void Int0_Init()
{
	IT0 = 1; //ʹ��INT0�½����ж�
	EX0 = 1; //ʹ��INT0�ж�
}

#endif
