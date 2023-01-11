#include "pwm.h"


extern uint8_t  MaxPWM;

//��ʼ��PWM����
void PWM_Init() {
	PWM_OFF;
	TCCR1B = 1;		//T1����Ƶ
}


//����PWMռ�ձ�
void SetPWM(uint8_t m) {
	uint8_t tmp_pwm = m;

	if(tmp_pwm > MaxPWM) {
		tmp_pwm = MaxPWM;
		LEDRED_ON;
	}
/*
	if(Strom > MAX_STROM) {
		OCR1A = 0; OCR1B = 0; OCR2 = 0;
		NEG_ABC_OFF;
		LEDRED_ON;
		Strom--;
	}
*/

	OCR1A = tmp_pwm; OCR1B = tmp_pwm; OCR2 = tmp_pwm;
}


