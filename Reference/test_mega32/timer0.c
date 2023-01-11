#include "timer0.h"

volatile uint8_t  CountT2Overflow = 0;
volatile uint16_t CountMilliseconds = 0;
volatile uint16_t SIO_Timeout = 0;
volatile uint16_t I2C_Timeout = 0;


//T0����жϴ������(ϵͳʱ�Ӷ�ʱ��)
SIGNAL(SIG_OVERFLOW0) { 
	static uint8_t cnt = 0;	

	//ÿ���4��Ϊ1ms
	if(!cnt--) {
		cnt = 3;
		CountMilliseconds += 1;

		if(SIO_Timeout > 1) SIO_Timeout--;  
		if(I2C_Timeout > 1) I2C_Timeout--;
	}
}

//T2����жϴ������(���ڼ���rpmת��)
SIGNAL(SIG_OVERFLOW2) {
	CountT2Overflow++;
}


//��ʼ����ʱ��0
void Timer_Init() {
	TIMER0_CK8_INIT;	//��ʱ��0Ԥ��8��Ƶ������ж�Ƶ��ΪΪ1M/256=4kHz��
	TIMER0_START;		//��ʱ��0����
	TIMER2_INT_ENABLE;	//��ʱ��2�ж�ʹ��
} 

//�������ɺ�����Ԥ��ʱ��ֵ
uint16_t SetDelay(uint16_t t) {
	return(CountMilliseconds + t - 1);                                             
}

//�����ε�Ԥ��ʱ��ֵʱ���Ƿ�,δ���򷵻�0�����򷵻�ĳ��ֵ
char CheckDelay (uint16_t t) {
	return(((t - CountMilliseconds) & 0x8000) >> 8);
}

//�趨��ʱw����
void Delay_ms(uint16_t w) {
	uint16_t akt;
	akt = SetDelay(w);
	while (!CheckDelay(akt));
}

//��ʱ����-O2���뻷����Ϊ(5*t+3)/8M�룻��-O0���뻷����Ϊ(10*t+22)/8M��
uint8_t Delay(unsigned int timer) {
	while(timer--);
	return(0);  
}

//��ʱt΢��
void Wait(uint8_t t) {
    t = (unsigned char)TCNT0 + t;
    while((TCNT0 - t) & 0x80);
}
