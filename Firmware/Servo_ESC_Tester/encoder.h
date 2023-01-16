#ifndef __ENCODER_H
#define __ENCODER_H

#define EC_A P32 //INT0
#define EC_B P33 //INT1
#define EC_KEY P55

bit encoder_flag; //编码器事件标志位
bit up_flag;	//向上拨动标志位
char encoder_counter; //编码器计数值，若发生编码器中断后未处理，则累计计数值

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
	IT0 = 1; //使能INT0下降沿中断
	EX0 = 1; //使能INT0中断
}

#endif
