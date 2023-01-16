#ifndef __PWM_H
#define __PWM_H

#define PWM1 		P10
#define PWM1_L 	P11
#define PWM2		P12
#define PWM2_L	P13
#define PWM3		P14
#define PWM3_L	P15

u16 pwmb_cap; //计数值临时存放变量
u16 pwmb_cap_up; //采集到的上升沿时刻
u16 pwmb_cap_res; //高电平持续时间
//u8 pwmb_count;	//溢出计数值
bit pwmb_up_flag; //捕获到上升沿标志

void PWMA_Config(void)
{
	//先将MOS管选通信号拉低，防止误导通
	PWM1 = 0;
	PWM1_L = 0;
	PWM2 = 0;
	PWM2_L = 0;
	PWM3 = 0;
	PWM3_L = 0;
	
	//配置选通引脚为推挽输出，M0相应bit为1，M1相应bit为0
	P1M0 |= 0x3f; //0x3f = 0B0011 1111，bit0~bit5置1
	P1M1 &= ~0x3f; //~0x3f = 0B1100 0000,bit0~bit5置0
	
	PWMA_PSCR = 3;	//PWMA_PSCR为PWMA的16位预分频器寄存器，可以16位数据读写，f_ck_int = f_ck_psc/(PSCR[15:0]+1)
	PWMA_DTR = 24;	//PWMA_DTR位PWMA的死区寄存器，设置死区持续时间，本程序死区互补功能未开
	
	PWMA_ARR = 0xff;	//PWMA_ARR为PWMA的16位自动重载寄存器
	PWMA_CCER1 = 0;	//捕获/比较使能寄存器1，配置极性及输出使能
	PWMA_CCER2 = 0;	//捕获/比较使能寄存器2，配置极性及输出使能
	PWMA_SR1 = 0;		//状态寄存器1，中断标记
	PWMA_SR2 = 0;		//状态寄存器2，重复捕获标记
	PWMA_ENO = 0;		//输出使能寄存器
	PWMA_PS = 0;		//功能脚切换
	PWMA_IER = 0;		//中断使能寄存器
	
	PWMA_CCMR1 = 0x68;	//捕获/比较模式寄存器1，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR1 = 0x00;			//捕获/比较寄存器，16位，当前比较值
	PWMA_CCER1 |= 0x01;	//捕获/比较使能寄存器1，原0x05 = 0b0000 0101,现改为0x01，使能OC1输出
	PWMA_PS |= 0;				//PWMA IO选择
	
	PWMA_CCMR2 = 0x68;	//捕获/比较模式寄存器2，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR2 = 0x00;			//捕获/比较寄存器，16位，当前比较值，与PWMA_ARR比较
	PWMA_CCER1 |= 0x10;	//捕获/比较使能寄存器1，原0x50 = 0b0101 0000，现改为0x10，使能OC2输出
	PWMA_PS |= (0<<2);	//PWMA IO选择
	
	PWMA_CCMR3 = 0x68;	//捕获/比较模式寄存器3，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR3 = 0x00;			//捕获/比较寄存器，16位，当前比较值
	PWMA_CCER2 |= 0x01;	//捕获/比较使能寄存器2，原0x05 = 0b0000 0101,现改为0x01，使能OC3输出
	PWMA_PS |= (0<<4);	//PWMA IO选择
	
	PWMA_BKR = 0x80;	//使能OC和OCN输出
	PWMA_CR1 = 0x81;	//控制寄存器1，使能自动预装载，边沿对齐，向上计数，使能计数器
	PWMA_EGR = 0x01;	//初始化计数器
}

//PWMB配置函数
//PWMB_CCMR1配置CC8S位为1使通道为输入，使PWMB_CCR1寄存器变为只读
//配置PWMB_CCMR1中的IC8F位设置滤波时间
//配置PWMB_CCER1中的CC8P配置转换边沿
//配置预分频器，PWMB_CCMR1寄存器中的IC8PSC置0禁止
//配置PWMB_CCER1中的CC1E=1；允许捕获计数值到捕获寄存器中
//配置PWMB_IER中的CC8IE运行相关中断请求
//	捕获输入时，计数器的值被传送到PWMB_CCR1
//	CC8IF标志被置1。当发生至少两次连续捕获而CC8IF未被清零，则CC8OF也被置1
//	设置了CC8IE位，产生中断
void PWMB_Config(void)
{
	//将P2.3设置为高阻输入
	P2M1 |= 0x08; //标志位置1，0x08 = 0b0000 1000
	P2M0 &= ~0x08; //标志位清0
	P2PU |= 0x08; //设置上拉电阻，bit3置1使能上拉电阻
	
	PWMB_CCER2 = 0x00;
	PWMB_ENO &= ~0x40; //0x0 = 0b0100 0000，标志位清零，关闭PWM8输出
	PWMB_PSCR = 0x00;
	PWMB_DTR = 0x00;
	PWMB_PS &= 0x3f; //0b0011 1111
	
	PWMB_CCMR3 = 0x32;
	PWMB_CCMR4 = 0x31; //0x31 = 0b0011 0001；bit7~bit4为滤波器选择，11选8个时钟数，bit3~bit2预分频器；00无预分频器，捕获每一次边沿；bit1~bit0定义为输入
	//PWMB_CCER2 = 0x10;//0b0001 0000；bit5为CC8P，bit4为CC8E；输入捕获模式下，CC8P置1下降沿，清0上升沿，CC8E置1开启输入捕获，置0关闭
	PWMB_CCER2 = 0x11;
	PWMB_CCER2 |= 0x00;	//PWM7上升沿
	PWMB_CCER2 |= 0x20; //PWM8下降沿 
	PWMB_IER |= 0x10; //0b0001 0001，开启PWM8中断，开启更新中断
	
	PWMB_CR1 = 0x01; //0b0000 0101，bit1为UDISB，清零允许更新中断，中断使能
	
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
