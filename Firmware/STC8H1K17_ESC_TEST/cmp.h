#ifndef __CMP_H
#define __CMP_H

#define TMP_LEN 8
u8 time_index;
u16 phase_time_tmp[TMP_LEN];

//�Ƚ�����ʼ��
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

//�Ƚ����жϺ���
//��⵽�����¼��󣬸��ݶ�ʱ��3�ļ���ֵ��������һ�ι����¼���ʱ��
//���8�λ�������ʱ�����16������ÿ��λ�������ʱ�䣬��Ϊ���´λ�������ʱ��
void CMP_ISR(void) interrupt 21
{
	u16 phase_time;
	u8 i;
	
	CMPCR1 &= ~0x40; //����жϱ�־λ
	
	if(demagnetizing_cnt == 0) //���ź�������¼���δ���Ų�����Ƚ����ж�
	{
		//T4T3M &= ~(1<<3); //Timer3ֹͣ����
		TR1 = 0;
		//if(t3_flag)
		if(t1_flag)
		{
			//t3_flag = 0;
			t1_flag = 0;
			phase_time = 0x1fff;
		}
		else
		{
			//phase_time = (((u16)T3H<<8)+(u16)T3L)>>1;
			phase_time = (((u16)TH1<<8)+(u16)TL1)>>1;
			phase_time &= 0x1ffff;
		}
		//T3H = 0;
		//T3L = 0;
		//T4T3M |= (1<<3);
		TH1 = 0;
		TL1 = 0;
		TR1 = 1;
		
		phase_time_tmp[time_index] = phase_time;
		if(++time_index >=8) time_index = 0;
		for(phase_time = 0,i = 0;i<8;i++)
		{
			phase_time += phase_time_tmp[i];
		}
		phase_time >>= 4;
		if(phase_time > 40 && phase_time < 1000)
		{
			timeout = 125;
		}
		if(phase_time > 40) phase_time -= 40;
		else phase_time = 20;
		
		/*
		T4T3M &= ~(1<<7);
		phase_time = phase_time << 1;
		phase_time = 0 - phase_time;
		T4H = (u8)(phase_time >> 8);
		T4L = (u8)phase_time;
		T4T3M |= (1<<7);
		demagnetizing_cnt = 1;
		*/
		
		AUXR &= ~(0x10);;
		phase_time = phase_time << 1;
		phase_time = 0 - phase_time;
		T2H = (u8)(phase_time >> 8);
		T2L = (u8)phase_time;
		AUXR |= 0x10; //Timer2��ʼ����
		demagnetizing_cnt = 1;
	}
}

#endif