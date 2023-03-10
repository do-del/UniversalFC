#ifndef __TIMER_H
#define __TIMER_H

//bit t3_flag; //Timer3溢出标志，32.768ms置1一次
bit t0_flag; //Timer0溢出标志，4ms置1一次
bit t1_flag;

//Timer0设为4ms计时，用于主循环刷新
void Timer0_Config(void)
{
	TMOD &= ~0x03;		//定时器0/1模式寄存器，~0x03 = 0b1111 1100，bit1~bit0清零，定时器0配置为16位自动重载模式
	AUXR &= ~(1<<7);	//辅助寄存器1，bit7置0，定时器0时钟为CPU时钟12分频，24MHz/12 = 2MHz
	
	//12分频下，定时器周期=(65536-[TH0,TL0])*12/FOSC,如4ms定时，这周期= 0.004 = 1/250
	TH0 = (65536UL - FOSC / 12 / 250)>>8;	//定时器0计数寄存器
	TL0 = (u8)(65536UL - FOSC / 12 / 250); //定时器0计数寄存器
	TR0 = 1;	//启动定时器0
	ET0 = 1;	//使能定时器0中断
}

//作用与Timer3相同，替换Timer3，用于只有Timer0~Timer2的型号芯片
void Timer1_Config(void)
{
	TMOD &= ~0x30;	
	TMOD |= 0x10; //配置Timer1为16位不自动重载模式
	AUXR &= ~(1<<6); //Timer1时钟为CPU时钟12分频
	
	TH1 = 0;
	TL1 = 0;
	TR1 = 1;	//启动定时器1
	ET1 = 1;	//使能定时器1中断
}

//替换Timer4，用于只有Timer0~Timer2的型号芯片
void Timer2_Config(void)
{
	AUXR &= ~(0x80); //0x80 = 0b0000 1000，bit3清零，Timer2用作定时器
	AUXR &= ~(0x30); //0x30 = 0b0000 0100，bit2清零，Timer2时钟为CPU时钟12分频
	T2H = 0;
	T2L = 0;
	IE2 |= (1<<2); //使能Timer2溢出中断
	//AUXR |= 0x10; //Timer2开始计数
}

/*
void Timer3_Config(void)
{
	T4T3M &= 0xf0;	//定时器3/4控制寄存器，高4位定时器4，低4位定时器3。低4位清零，定时器3停止计数，用作定时器，12分频，关闭时钟输出
	T3H = 0;	//定时器3计数寄存器
	T3L = 0;	//定时器3计数寄存器
	
	T3T4PIN = 0x01;	//定时器3/4功能引脚切换寄存器，bit0选择位，置1选择P0.0，P0.1，P0.2，P0.3
	IE2 |= (1<<5);	//中断使能寄存器2，bit5为定时器3的溢出中断允许位，置1开中断
	T4T3M |= (1<<3);	//定时器3/4控制寄存器，bit3置1，定时器3开始计数
}
*/

/*
void Timer4_Config(void)
{
	T4T3M &= 0x0f; //定时器3/4控制寄存器，高4位定时器4，低4位定时器3。低4位清零，定时器4停止计数，用作定时器，12分频，关闭时钟输出
	T4H = 0;	//定时器4计数寄存器
	T4L = 0;	//定时器4计数寄存器
	
	//T4H = (u8)((65536UL - 40*2) >> 8);	//测试用，定时器4计数频率为2MHz，40us要用80次
	//T4L = (u8)(65536UL - 40*2);	//测试用
	
	T3T4PIN = 0x01;	//定时器3/4功能引脚切换寄存器，bit0选择位，置1选择P0.0，P0.1，P0.2，P0.3
	IE2 |= (1<<6);	//中断使能寄存器2，bit6为定时器4的溢出中断允许位，置1开中断
	//T4T3M |=  (1<<7);	//开始计数，测试用
}
*/

void Timer0_ISR(void) interrupt 1
{
	//PWM1 = !PWM1; //定时器0中断测试，翻转PWM1引脚电平
	t0_flag = 1;
}

void Timer1_ISR(void) interrupt 3
{
	//PWM2 = !PWM2; //定时器1中断测试，翻转PWM1引脚电平
	t1_flag = 1;
}

/*
void Timer3_ISR(void) interrupt 19
{
	//PWM2 = !PWM2; //定时器3中分段测试，翻转PWM2引脚电平
	t3_flag = 1;
}
*/

void Timer2_ISR(void) interrupt 12
{
	AUXR &= ~(0x10);
	
	if(demagnetizing_cnt == 1)	//判断消磁计数值，为1为需要消磁
	{
		demagnetizing_cnt = 2;	//将消磁计数值改为2，先换相，再进行消磁
		if(m_running)
		{
			if(++step >= 6) step = 0;
			Motor_Step();
		}
		
		T2H = (u8)((65536UL - 40*2) >> 8);	//40us计数，(65536 - t *2)，t单位us，高8位放入T4H，低8位放入T4L
		T2L = (u8)(65536UL - 40*2);	//40us计数
		AUXR |= 0x10; //Timer2开始计数
	}
	else if(demagnetizing_cnt == 2)
	{
		demagnetizing_cnt = 0;
	}
}

/*
//电机启动后利用定时器4来换相和消磁，消磁原理简化成短延时，此处设为40us延时，电流越大，消磁时间越长
//电机换相后，断电的那一相，由于线圈的存在会产生自感电动势，若不消磁，会引起比较器在非感应电动势过零时刻中断
//此处根据修改demagnetzing_cnt消磁计数值来跳过因消磁引起的比较器中断
void Timer4_ISR(void) interrupt 20
{
	T4T3M &= ~(1<<7); //定时器3/4控制寄存器bit7置0，定时器4停止计数
	
	if(demagnetizing_cnt == 1)	//判断消磁计数值，为1为需要消磁
	{
		demagnetizing_cnt = 2;	//将消磁计数值改为2，先换相，再进行消磁
		if(m_running)
		{
			if(++step >= 6) step = 0;
			Motor_Step();
		}
		
		T4H = (u8)((65536UL - 40*2) >> 8);	//40us计数，(65536 - t *2)，t单位us，高8位放入T4H，低8位放入T4L
		T4L = (u8)(65536UL - 40*2);	//40us计数
		T4T3M |=  (1<<7);	//Timer4开始计数
	}
	else if(demagnetizing_cnt == 2)
	{
		demagnetizing_cnt = 0;
	}
}
*/
#endif