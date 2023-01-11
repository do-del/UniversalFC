#include "twislave.h"


uint8_t Byte_Counter = 0;
uint8_t I2C_RxBuffer = 0;

uint8_t twi_rx_buffer[TWI_MAX_RECV_SIZE] = {0,};
uint8_t twi_rx_idx = 0;

extern uint8_t  PwmSet;
extern uint8_t  PwmSetValue;

extern uint8_t  MotorAddress;
extern volatile uint16_t I2C_Timeout;

//初始化I2C接口
void InitI2C_Slave(uint8_t adr) {
	TWAR = adr + (MotorAddress << 1);
	
	//接着进入从机接收模式 
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
}

//I2C中断服务程序(从机接收)
SIGNAL(SIG_2WIRE_SERIAL) {		//ISR(TWI_vect)
	uint8_t status = TW_STATUS;
	
	switch(status) { 
		/*case TW_START:
			TWCR |= (1 <<TWINT);
			Byte_Counter = 0;
			break; */

		case TW_SR_SLA_ACK:
					twi_rx_idx = 0;
					TWCR |= (1 << TWINT);
					break;

		case TW_SR_DATA_ACK:
					if(twi_rx_idx < TWI_MAX_RECV_SIZE) { twi_rx_buffer[twi_rx_idx++] = TWDR; }
					TWCR |= (1 << TWINT);
					break;
					
		case TW_SR_STOP:
					I2C_Timeout = 1;
					TWCR |= (1 << TWINT);
					break;				
	

	}
}


//I2C处理命令流程
void I2CRxComdAction(void) {
	uint8_t tmp = ~twi_rx_buffer[1];
	if(twi_rx_buffer[0] == tmp) {
		PwmSet = 1;
		PwmSetValue = twi_rx_buffer[0];
		
		twi_rx_buffer[0] = twi_rx_buffer[1] = 0;		
	}
}



