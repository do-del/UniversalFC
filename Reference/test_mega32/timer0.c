#include "timer0.h"

volatile uint8_t  CountT2Overflow = 0;
volatile uint16_t CountMilliseconds = 0;
volatile uint16_t SIO_Timeout = 0;
volatile uint16_t I2C_Timeout = 0;


//T0溢出中断处理程序(系统时钟定时器)
SIGNAL(SIG_OVERFLOW0) { 
	static uint8_t cnt = 0;	

	//每溢出4次为1ms
	if(!cnt--) {
		cnt = 3;
		CountMilliseconds += 1;

		if(SIO_Timeout > 1) SIO_Timeout--;  
		if(I2C_Timeout > 1) I2C_Timeout--;
	}
}

//T2溢出中断处理程序(用于计算rpm转速)
SIGNAL(SIG_OVERFLOW2) {
	CountT2Overflow++;
}


//初始化定时器0
void Timer_Init() {
	TIMER0_CK8_INIT;	//定时器0预设8分频（溢出中断频率为为1M/256=4kHz）
	TIMER0_START;		//定时器0启动
	TIMER2_INT_ENABLE;	//定时器2中断使能
} 

//返回若干毫秒后的预期时钟值
uint16_t SetDelay(uint16_t t) {
	return(CountMilliseconds + t - 1);                                             
}

//检查入参的预期时钟值时间是否到,未到则返回0，到则返回某个值
char CheckDelay (uint16_t t) {
	return(((t - CountMilliseconds) & 0x8000) >> 8);
}

//设定延时w毫秒
void Delay_ms(uint16_t w) {
	uint16_t akt;
	akt = SetDelay(w);
	while (!CheckDelay(akt));
}

//延时：在-O2编译环境下为(5*t+3)/8M秒；在-O0编译环境下为(10*t+22)/8M秒
uint8_t Delay(unsigned int timer) {
	while(timer--);
	return(0);  
}

//延时t微秒
void Wait(uint8_t t) {
    t = (unsigned char)TCNT0 + t;
    while((TCNT0 - t) & 0x80);
}
