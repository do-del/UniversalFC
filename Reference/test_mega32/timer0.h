#ifndef __TIMER0_H__
#define __TIMER0_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include "CONFIG.h"


//若使用mega32芯片的寄存器定义
#ifdef USE_AVR_mega32

#define TIMER0_CK8_INIT		{ TCCR0  = 2;		}
#define TIMER0_START		{ TIMSK |= 1;		}
#define TIMER2_INT_ENABLE	{ TIMSK |= 0x40;	}

#endif


//若使用mega8芯片的寄存器定义
#ifdef USE_AVR_mega8




#endif



void Timer_Init();

uint16_t SetDelay(uint16_t t);

char CheckDelay (uint16_t t);

void Delay_ms(uint16_t w);

uint8_t Delay(unsigned int timer);

void Wait(uint8_t t);





#endif

 
