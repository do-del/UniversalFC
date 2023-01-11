#ifndef __PORT_H__
#define __PORT_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include "CONFIG.h"
#include "timer0.h"


//若使用mega32芯片的寄存器定义
#ifdef USE_AVR_mega32
	#define LEDGRN_ON	{ PORTA &= ~0x10; }		//绿灯亮
	#define LEDGRN_OFF	{ PORTA |=  0x10; }		//绿灯暗
	#define LEDRED_ON	{ PORTA &= ~0x08; }		//红灯亮
	#define LEDRED_OFF	{ PORTA |=  0x08; }		//红灯暗

	#define POS_ABC_OFF		{ PORTD &= ~0xB0; }	//场效应管A+,B+,C+全关
	#define NEG_ABC_OFF		{ PORTB &= ~0x13; } //场效应管A-,B-,C-全关
	#define FETS_OFF		{ NEG_ABC_OFF; POS_ABC_OFF; }	//所有6个场效应管全关
	
	#define POS_A_ON		{ PORTD |= 0x80; }	//A+开
	#define POS_B_ON		{ PORTD |= 0x10; }	//B+开	
	#define POS_C_ON		{ PORTD |= 0x20; }	//C+开
	#define NEG_A_ON		{ PORTB |= 0x01; }	//A-开
	#define NEG_B_ON		{ PORTB |= 0x02; }	//B-开
	#define NEG_C_ON		{ PORTB |= 0x10; }	//C-开

	#define MUTEX_NEG_A_ON	{ PORTB &= ~0x12; NEG_A_ON; }	//互斥型A-开
	#define MUTEX_NEG_B_ON	{ PORTB &= ~0x11; NEG_B_ON; }	//互斥型B-开
	#define MUTEX_NEG_C_ON	{ PORTB &= ~0x03; NEG_C_ON; }	//互斥型C-开

#endif


//若使用mega8芯片的寄存器定义
#ifdef USE_AVR_mega8


#endif



void Port_Init();

void LedRedBlink(uint8_t anz);


#endif
