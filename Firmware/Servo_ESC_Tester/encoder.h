#ifndef __ENCODER_H
#define __ENCODER_H

#define EC_A P32 //INT0
#define EC_B P33 //INT1
#define EC_KEY P55

bit encoder_flag; //编码器事件标志位
bit up_flag;	//向上拨动标志位
char count; //编码器计数值，若发生编码器中断后未处理，则累计计数值

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
	IT0 = 0; //使能INT0下降沿和上升沿中断
	EX0 = 1; //使能INT0中断
}

#endif
