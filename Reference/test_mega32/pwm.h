#ifndef __PWM_H__
#define __PWM_H__

#include <avr/io.h>
#include "CONFIG.h"
#include "port.h"


#define MAX_PWM		255


//��ʹ��mega32оƬ�ļĴ�������
#ifdef USE_AVR_mega32
	#define PWM_C_ON	{ TCCR1A = 0x81; TCCR2 = 0x41; PORTD &= ~0x90; }	//T2ʱ��Դ����Ƶ
	#define PWM_B_ON	{ TCCR1A = 0x21; TCCR2 = 0x41; PORTD &= ~0xA0; } 	//T2ʱ��Դ����Ƶ
	#define PWM_A_ON	{ TCCR1A = 0x01; TCCR2 = 0x61; PORTD &= ~0x30; }	//T2ʱ��Դ����Ƶ
						  //OCR����ȫ����PWM�ѿ�,��Ϊ��ͨGPIO��������͵�ƽ
	#define PWM_OFF		{ OCR1A = 0; OCR1B = 0; OCR2 = 0; TCCR1A = 1; TCCR2 = 0x41; PORTD &= ~0xB0; }
	#define ALLPIN_AND_PWM_OFF	{ PORTB &= ~0x13; PWM_OFF; }					
#endif


//��ʹ��mega8оƬ�ļĴ�������
#ifdef USE_AVR_mega8


#endif


void PWM_Init();

void SetPWM(uint8_t m);

#endif

