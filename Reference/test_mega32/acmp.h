#ifndef __ACMP_H__
#define __ACMP_H__


#include <avr/io.h> 
#include <avr/interrupt.h>
#include "CONFIG.h"
#include "port.h"
#include "pwm.h"
#include "analog.h"


#define SENSE_A ADMUX = 0;		//�Ƚ�����ͨ��ΪNULL_A
#define SENSE_B ADMUX = 1;		//�Ƚ�����ͨ��ΪNULL_B
#define SENSE_C ADMUX = 2;		//�Ƚ�����ͨ��ΪNULL_C

#define SENSE		((ACSR & 0x10))		//�жϱȽ����жϱ�־�Ƿ���λ
#define SENSE_L		(!(ACSR & 0x20))	//�Ƚ������Ϊ�͵�ƽ
#define SENSE_H		((ACSR & 0x20))		//�Ƚ������Ϊ�ߵ�ƽ

//#define ENABLE_SENSE_INT 	{ ACSR |= 0x0A;  }	//ʹ�ܱȽ����ж�(�½��ز����ж�)
#define ENABLE_SENSE_INT 	{ ACSR |= 0x08;  }	//ʹ�ܱȽ����ж�
#define DISABLE_SENSE_INT	{ ACSR &= ~0x08; }	//���ܱȽ����ж�

#define SENSE_FALLING_INT	{ ACSR &= ~0x01; }	//�Ƚ����½��ز����ж�
#define SENSE_RISING_INT	{ ACSR |= 0x03;	 }	//�Ƚ��������ز����ж�
#define SENSE_TOGGLE_INT	{ ACSR &= ~0x03; }	//�Ƚ�����ƽ�仯�����ж�

#define ACMP_MULTI_ENABLE	{ SFIOR = 0x08;	 }	//�Ƚ�������·��������ʹ��



void Manual();




#endif

