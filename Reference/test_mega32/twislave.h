#ifndef __TWISLAVE_H__
#define __TWISLAVE_H__


#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include "CONFIG.h"


#define TWI_MAX_RECV_SIZE 	50

void InitI2C_Slave(uint8_t adr);

void I2CRxComdAction(void);

#endif

