#include "acmp.h"

extern volatile uint8_t Phase;
extern volatile uint8_t CountT2Overflow;
extern uint8_t  StromConvertCmd;
extern uint16_t CntCommutation;
extern uint8_t  InterT2Cnt;


//ģ��Ƚ����ж�
SIGNAL(SIG_COMPARATOR) {
	uint8_t sense = 0;
	static uint8_t rpmctcnt = 0, lastt2cnt = 0, currt2cnt = 0;
	
	do {
		if(SENSE_H) sense = 1;
		else		sense = 0;

		switch(Phase) {
		  case 0:	//AB��ͨPWM
			PWM_A_ON;
			if(sense) {
				MUTEX_NEG_C_ON;
				if(StromConvertCmd) NormalStromConvert();
				SENSE_FALLING_INT;
				SENSE_B;
				Phase++;
				CntCommutation++; rpmctcnt++;
			} else {
				MUTEX_NEG_B_ON;
			}
			break;
			
		  case 1:	//AC��ͨPWM
			MUTEX_NEG_C_ON;
			if(!sense) {
				PWM_B_ON;
				if(StromConvertCmd) NormalStromConvert();
				SENSE_A;
				SENSE_RISING_INT;
				Phase++;
				CntCommutation++; rpmctcnt++;
			} else {
				PWM_A_ON;
			}
			break;

		  case 2:	//BC��ͨPWM
			PWM_B_ON;
			if(sense) {
				MUTEX_NEG_A_ON;
				if(StromConvertCmd) NormalStromConvert();
				SENSE_C;
				SENSE_FALLING_INT;
				Phase++; 
				CntCommutation++; rpmctcnt++;
			} else {
				MUTEX_NEG_C_ON;
			}		  	
			break;

		  case 3:	//BA��ͨPWM
			MUTEX_NEG_A_ON;
			if(!sense) {
				PWM_C_ON;
				if(StromConvertCmd) NormalStromConvert();
				SENSE_B;
				SENSE_RISING_INT;
				Phase++; 
				CntCommutation++; rpmctcnt++;
			} else {
				PWM_B_ON;
			}
			break;

		  case 4:	//CA��ͨPWM
			PWM_C_ON;
			if(sense) {
				MUTEX_NEG_B_ON;
				if(StromConvertCmd) NormalStromConvert();
				SENSE_A;
				SENSE_FALLING_INT;
				Phase++; 
				CntCommutation++; rpmctcnt++;
			} else {
				MUTEX_NEG_A_ON;
			}
		    break;

		  case 5:	//CB��ͨPWM
			MUTEX_NEG_B_ON;
			if(!sense) {
				PWM_A_ON;
				if(StromConvertCmd) NormalStromConvert();
				SENSE_C;
				SENSE_RISING_INT;
				Phase = 0;
				CntCommutation++; rpmctcnt++;
			} else {
				PWM_C_ON;
			}
		    break;
		}


	} while((SENSE_L && sense) || (SENSE_H && ! sense));

	if(rpmctcnt == 6) {
		rpmctcnt = 0;
		currt2cnt = CountT2Overflow;		
		InterT2Cnt = currt2cnt - lastt2cnt;
		lastt2cnt = currt2cnt; 
	}
}



//���ݵ�ǰ��Phaseֵ������Ӧ��MOS�ܹ���
void Manual() {
	switch(Phase){
	  case 0:	//AB��ͨPWM
		PWM_A_ON;
		MUTEX_NEG_B_ON;
		SENSE_C; 
		SENSE_RISING_INT;
		break;
	  case 1:	//AC��ͨPWM
		PWM_A_ON;
		MUTEX_NEG_C_ON;
		SENSE_B; 
		SENSE_FALLING_INT;
		break;
	 case 2:	//BC��ͨPWM
		PWM_B_ON;
		MUTEX_NEG_C_ON;
		SENSE_A; 
		SENSE_RISING_INT;
		break;
	 case 3:	//BA��ͨPWM
		PWM_B_ON;
		MUTEX_NEG_A_ON;
		SENSE_C; 
		SENSE_FALLING_INT;
		break;
	 case 4:	//CA��ͨPWM
		PWM_C_ON;
		MUTEX_NEG_A_ON;
		SENSE_B; 
		SENSE_RISING_INT;
		break;
	 case 5:	//CB��ͨPWM
		PWM_C_ON;
		MUTEX_NEG_B_ON;
		SENSE_A; 
		SENSE_FALLING_INT;
		break;
	}
}
