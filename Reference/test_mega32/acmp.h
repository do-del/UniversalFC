#ifndef __ACMP_H__
#define __ACMP_H__


#include <avr/io.h> 
#include <avr/interrupt.h>
#include "CONFIG.h"
#include "port.h"
#include "pwm.h"
#include "analog.h"


#define SENSE_A ADMUX = 0;		//比较器负通道为NULL_A
#define SENSE_B ADMUX = 1;		//比较器负通道为NULL_B
#define SENSE_C ADMUX = 2;		//比较器负通道为NULL_C

#define SENSE		((ACSR & 0x10))		//判断比较器中断标志是否置位
#define SENSE_L		(!(ACSR & 0x20))	//比较器输出为低电平
#define SENSE_H		((ACSR & 0x20))		//比较器输出为高电平

//#define ENABLE_SENSE_INT 	{ ACSR |= 0x0A;  }	//使能比较器中断(下降沿产生中断)
#define ENABLE_SENSE_INT 	{ ACSR |= 0x08;  }	//使能比较器中断
#define DISABLE_SENSE_INT	{ ACSR &= ~0x08; }	//禁能比较器中断

#define SENSE_FALLING_INT	{ ACSR &= ~0x01; }	//比较器下降沿产生中断
#define SENSE_RISING_INT	{ ACSR |= 0x03;	 }	//比较器上升沿产生中断
#define SENSE_TOGGLE_INT	{ ACSR &= ~0x03; }	//比较器电平变化产生中断

#define ACMP_MULTI_ENABLE	{ SFIOR = 0x08;	 }	//比较器“多路复用器”使能



void Manual();




#endif

