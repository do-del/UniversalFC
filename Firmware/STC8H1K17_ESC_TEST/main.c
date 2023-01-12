#include "stc8h.h"
#include <intrins.h>

/*
 * PWM1接P1.0，配置推挽输出和PWM输出，控制电机U相高位MOS管选通信号，高电平导通，低电平截止
 * PWM1_L接P1.1，配置推挽输出，控制电机U相低位MOS管选通信号，高电平导通，低电平截止
 * PWM2接P1.2，配置推挽输出和PWM输出，控制电机V相高位MOS管选通信号，高电平导通，低电平截止
 * PWM2_L接P1.3，配置推挽输出，控制电机V相低位MOS管选通信号，高电平导通，低电平截止
 * PWM3接P1.4，配置推挽输出和PWM输出，控制电机W相高位MOS管选通信号，高电平导通，低电平截止
 * PWM3_L接P1.5，配置推挽输出，控制电机W相低位MOS管选通信号，高电平导通，低电平截止
 * ADC8（P0.0），配置高阻输入，采样电机U相感应电动势，用于过零检测比较器正输入
 * ADC9（P0.1），配置高阻输入，采样电机V相感应电动势
 * ADC10（P0.2），配置高阻输入，采样电机W相感应电动势
 * CMP-（P3.6），配置为高阻输入，接过零检测电路中点
*/

/*
 * 无刷电机通电次序
 * AB-AC-BC-BA-CA-CB-AB
 * 相对应感应电动势变化
 * C下降-B上升-A下降-C上升-B下降-A上升-C下降
*/

#define FOSC 24000000UL	//系统时钟
#define BRT (65536-(FOSC/115200+2)/4) //波特率转计数值

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#define PWM1 		P10
#define PWM1_L 	P11
#define PWM2		P12
#define PWM2_L	P13
#define PWM3		P14
#define PWM3_L	P15

#define RX_LEN 16
bit busy;		//串口发送完成标志
char wptr;	//串口接收数据长度
char buffer[RX_LEN]; //串口接收数据缓存

void Port_Init(void);	//芯片复位后引脚初始化
void PWMA_Config(void);	//PWMA配置函数
void ADC_Config(void);
void CMP_Config(void);
void Timer0_Config(void);
void Timer3_Config(void);
void Timer4_Config(void);

//调试用函数
void Uart_Config(void); //串口初始化
void UartSend(char dat); //串口发生字节
void UartSendStr(char *p); //串口发送字符串

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
	P_SW2 |= 0x80; //使能XFR
	
	Port_Init();
	PWMA_Config();
	ADC_Config();
	CMP_Config();
	Timer0_Config();
	Timer3_Config();
	Timer4_Config();
	
	Uart_Config();
	ES = 1;
	EA = 1;
	
	UartSendStr("--Brushless ESC Test--\r\n");
	
	PWMA_ENO = 0x15; //0x15 = 0b0001 0101
	
	while(1)
	{
		UartRxTest();
	}
}

void Port_Init(void)
{
	P0M0 = 0x00;
	P0M1 = 0x00; //P0端口初始化为双向口
	P1M0 = 0x00;
	P1M1 = 0x00; //P1端口初始化为双向口
	P2M0 = 0x00;
	P2M1 = 0x00; //P2端口初始化为双向口
	P3M0 = 0x00;
	P3M1 = 0x00; //P3端口初始化为双向口
	P5M0 = 0x00;
	P5M0 = 0x00; //P5端口初始化为双向口
}

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
	
	PWMA_ARR = 255;	//PWMA_ARR为PWMA的16位自动重载寄存器
	PWMA_CCER1 = 0;	//捕获/比较使能寄存器1，配置极性及输出使能
	PWMA_CCER2 = 0;	//捕获/比较使能寄存器2，配置极性及输出使能
	PWMA_SR1 = 0;		//状态寄存器1，中断标记
	PWMA_SR2 = 0;		//状态寄存器2，重复捕获标记
	PWMA_ENO = 0;		//输出使能寄存器
	PWMA_PS = 0;		//功能脚切换
	PWMA_IER = 0;		//中断使能寄存器
	
	PWMA_CCMR1 = 0x68;	//捕获/比较模式寄存器1，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR1 = 127;			//捕获/比较寄存器，16位，当前比较值
	PWMA_CCER1 |= 0x05;	//捕获/比较使能寄存器1，0x05 = 0b0000 0101
	PWMA_PS |= 0;				//PWMA IO选择
	
	PWMA_CCMR2 = 0x68;	//捕获/比较模式寄存器2，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR2 = 63;			//捕获/比较寄存器，16位，当前比较值，与PWMA_ARR比较
	PWMA_CCER1 |= 0x50;	//捕获/比较使能寄存器1，0x05 = 0b0101 0000
	PWMA_PS |= (0<<2);	//PWMA IO选择
	
	PWMA_CCMR3 = 0x68;	//捕获/比较模式寄存器3，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR3 = 191;			//捕获/比较寄存器，16位，当前比较值
	PWMA_CCER2 |= 0x05;	//捕获/比较使能寄存器2，0x05 = 0b0000 0101
	PWMA_PS |= (0<<4);	//PWMA IO选择
	
	PWMA_BKR = 0x80;	//使能OC和OCN输出
	PWMA_CR1 = 0x81;	//控制寄存器1，使能自动预装载，边沿对齐，向上计数，使能计数器
	PWMA_EGR = 0x01;	//初始化计数器
}

void ADC_Config(void)
{
	//设置引脚模式为高阻输入，M1相应bit为1，M0相应bit为0
	P1M1 |= 0xc0; //标志位置1
	P1M0 &= ~0xc0; //标志位清0
	
	//设置引脚模式为高阻输入，M1相应bit为1，M0相应bit为0
	P0M1 |= 0x0f;  //标志位置1
	P0M0 &= ~0x0f; //标志位清0
	
}

void CMP_Config(void)
{
	
}

void Timer0_Config(void)
{
	
}

void Timer3_Config(void)
{
	
}

void Timer4_Config(void)
{
	
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

