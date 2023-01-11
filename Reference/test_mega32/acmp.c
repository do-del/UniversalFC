#include "acmp.h"

extern volatile uint8_t Phase;
extern volatile uint8_t CountT2Overflow;
extern uint8_t  StromConvertCmd;
extern uint16_t CntCommutation;
extern uint8_t  InterT2Cnt;


//模拟比较器中断
SIGNAL(SIG_COMPARATOR) {
	uint8_t sense = 0;
	static uint8_t rpmctcnt = 0, lastt2cnt = 0, currt2cnt = 0;
	
	do {
		if(SENSE_H) sense = 1;
		else		sense = 0;

		switch(Phase) {
		  case 0:	//AB导通PWM
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
			
		  case 1:	//AC导通PWM
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

		  case 2:	//BC导通PWM
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

		  case 3:	//BA导通PWM
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

		  case 4:	//CA导通PWM
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

		  case 5:	//CB导通PWM
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



//根据当前的Phase值驱动相应的MOS管工作
void Manual() {
	switch(Phase){
	  case 0:	//AB导通PWM
		PWM_A_ON;
		MUTEX_NEG_B_ON;
		SENSE_C; 
		SENSE_RISING_INT;
		break;
	  case 1:	//AC导通PWM
		PWM_A_ON;
		MUTEX_NEG_C_ON;
		SENSE_B; 
		SENSE_FALLING_INT;
		break;
	 case 2:	//BC导通PWM
		PWM_B_ON;
		MUTEX_NEG_C_ON;
		SENSE_A; 
		SENSE_RISING_INT;
		break;
	 case 3:	//BA导通PWM
		PWM_B_ON;
		MUTEX_NEG_A_ON;
		SENSE_C; 
		SENSE_FALLING_INT;
		break;
	 case 4:	//CA导通PWM
		PWM_C_ON;
		MUTEX_NEG_A_ON;
		SENSE_B; 
		SENSE_RISING_INT;
		break;
	 case 5:	//CB导通PWM
		PWM_C_ON;
		MUTEX_NEG_B_ON;
		SENSE_A; 
		SENSE_FALLING_INT;
		break;
	}
}
