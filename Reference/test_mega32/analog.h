#ifndef __ANALOG_H__
#define __ANALOG_H__


#include <avr/io.h> 
#include "port.h"


#define LIMIT_STROM		65
#define MAX_STROM		130


uint16_t MessAD(uint8_t channel);

void FastStromConvert();

void NormalStromConvert();

uint8_t DelayM(uint8_t timer);

#endif


