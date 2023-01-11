#ifndef __PORT_H__
#define __PORT_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include "CONFIG.h"
#include "timer0.h"


//��ʹ��mega32оƬ�ļĴ�������
#ifdef USE_AVR_mega32
	#define LEDGRN_ON	{ PORTA &= ~0x10; }		//�̵���
	#define LEDGRN_OFF	{ PORTA |=  0x10; }		//�̵ư�
	#define LEDRED_ON	{ PORTA &= ~0x08; }		//�����
	#define LEDRED_OFF	{ PORTA |=  0x08; }		//��ư�

	#define POS_ABC_OFF		{ PORTD &= ~0xB0; }	//��ЧӦ��A+,B+,C+ȫ��
	#define NEG_ABC_OFF		{ PORTB &= ~0x13; } //��ЧӦ��A-,B-,C-ȫ��
	#define FETS_OFF		{ NEG_ABC_OFF; POS_ABC_OFF; }	//����6����ЧӦ��ȫ��
	
	#define POS_A_ON		{ PORTD |= 0x80; }	//A+��
	#define POS_B_ON		{ PORTD |= 0x10; }	//B+��	
	#define POS_C_ON		{ PORTD |= 0x20; }	//C+��
	#define NEG_A_ON		{ PORTB |= 0x01; }	//A-��
	#define NEG_B_ON		{ PORTB |= 0x02; }	//B-��
	#define NEG_C_ON		{ PORTB |= 0x10; }	//C-��

	#define MUTEX_NEG_A_ON	{ PORTB &= ~0x12; NEG_A_ON; }	//������A-��
	#define MUTEX_NEG_B_ON	{ PORTB &= ~0x11; NEG_B_ON; }	//������B-��
	#define MUTEX_NEG_C_ON	{ PORTB &= ~0x03; NEG_C_ON; }	//������C-��

#endif


//��ʹ��mega8оƬ�ļĴ�������
#ifdef USE_AVR_mega8


#endif



void Port_Init();

void LedRedBlink(uint8_t anz);


#endif
