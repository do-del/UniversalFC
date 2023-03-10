#include "BLMC.h"

extern uint16_t Strom;
extern uint8_t  Strom_max;
extern uint16_t LeakStrom;
	
extern uint8_t  PWM;
extern uint16_t MinRpmPulse;

extern volatile uint8_t Phase;



//电机启动
uint8_t MotorStartUp(uint8_t pm) {
	uint16_t i, timer = 300;
	
	DISABLE_SENSE_INT;
	PWM = 5;
	SetPWM(PWM);
	Manual();

	//看看在最小转速下（PWM=5时,某2相通电300ms），电流是否会超过12A，超过则红灯闪烁，返回0
	MinRpmPulse = SetDelay(300);
	while(!CheckDelay(MinRpmPulse)) {
		FastStromConvert();
		if(Strom > 190) {		//!!!原型为120
			ALLPIN_AND_PWM_OFF;
			LedRedBlink(10);
			return 0;
		}
	}

	volatile uint16_t count = 0;
	//开始按入参PWM值启动
	PWM = pm;
	Phase = 0;

	while(1) {

		for(i = 0; i < timer; i++) {
			Wait(100);	//Wait(100)=延时0.1ms
		}
		
		//每次循环都测一下电流，超过6A则红灯闪烁，返回0
		FastStromConvert();
		if(Strom > 200) {	//!!!原型为60,若开12V则可设到150
			ALLPIN_AND_PWM_OFF;
			LedRedBlink(5);
			return 0;
		}
		
		//timer每次减少1/15，若减少到一定程度则说明已经转起来了(经计算循环51次)，返回1
		count++;
		if(timer >= 25)  timer -= timer/15 + 1;
		if(timer < 25) {
			//timer = 25;  if(++count >= 2000)	return 1;
			return 1;
		}

		Manual();
		Phase++;
		Phase %= 6;
		NormalStromConvert();
		PWM = pm;
		SetPWM(PWM);

		//若比较器中断标志被置位
		if(SENSE)
			LEDGRN_ON;
		

	}
	return 0;
}


//马达和MOSFET自检 
void MotorTest() {
	uint16_t i, t = 0;
	uint8_t  anz = 0, boundary = 60; //相当于5A电流
	uint8_t  MosfetOkay = 0;
	uint16_t TONE[8] = {6250, 5566, 5000, 4692, 4167, 3756, 3333, 3125};

	DISABLE_SENSE_INT;
	cli();					//!!!关所有中断
	ALLPIN_AND_PWM_OFF;

	Strom_max = 0;
	DelayM(50);
	LeakStrom = Strom_max;

	t = 1000;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//测试每一相的MOS管开关陡峭特性，以保证不会两个MOS管同时导通
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//(1)A+管和A-管的开关陡峭特性测试
	Strom = 0;
	for(i = 0; i < t; i++) {
		NEG_A_ON;
		DelayM(1);
		FETS_OFF;
		Delay(5);

		POS_A_ON;
		DelayM(1);
		FETS_OFF;

		if(Strom > boundary + LeakStrom) {
			anz = 4; FETS_OFF; break;
		}
		Delay(5);
	}
	Delay(10000);
	
	//(2)B+管和B-管的开关陡峭特性测试
	Strom = 0;
	for(i = 0; i < t; i++) {
		NEG_B_ON; 
		DelayM(1);
		FETS_OFF;
		Delay(5);
  		
		POS_B_ON;;
		DelayM(1);
		FETS_OFF;

  		if(Strom > boundary + LeakStrom) {
			anz = 5; FETS_OFF; break;
		}
  		Delay(5);		
	}
	Delay(10000);

	//(3)C+管和C-管的开关陡峭特性测试
	Strom = 0;
	for(i = 0; i < t; i++) {
		NEG_C_ON; 
		DelayM(1);
		FETS_OFF;
		Delay(5);

		POS_C_ON;
		DelayM(1);
		FETS_OFF;
		if(Strom > boundary + LeakStrom) {
			anz = 6; FETS_OFF; break;
		}
		Delay(5);
	}
	Delay(10000);

	//若有MOS管不好,则红灯连续闪烁，进入死循环
	if(anz) while(1) LedRedBlink(anz);


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//测量每相的上臂MOS管（也就是X+管）能否导通
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	volatile uint16_t tmpA[200]= {0,};
	#define SOUND_CONST  750000    //大约0.5秒时间
	#define SOUND_E 	 1		   //

	SENSE_A;
    FETS_OFF;
	Strom = 0;
	//(1)A+管和B-C-管导通（循环120次）
	NEG_B_ON;
    NEG_C_ON;
	for(i = 0; i < SOUND_CONST / TONE[0]; i++) {
		POS_A_ON;	
		Delay(1);
		tmpA[i] = MessAD(0);
		if(tmpA[i] > 50)	{ MosfetOkay |= 0x01; } 
		else 				{ MosfetOkay &= ~0x01;}
		POS_ABC_OFF;	
		Delay(TONE[0]); //Delay(1600)=1ms
    }
	FETS_OFF;

	//(2)B+管和A-C-管导通
	NEG_A_ON;
	NEG_C_ON;
	for(i = 0; i < SOUND_CONST / TONE[1]; i++) {
		POS_B_ON; 
		Delay(SOUND_E);
		if(MessAD(1) > 50) { MosfetOkay |= 0x02; } 
		else 			   { MosfetOkay &= ~0x02;}
		POS_ABC_OFF;
		Delay(TONE[1]);		
	}
	FETS_OFF

	//(3)C+管和A-B-管导通
	NEG_A_ON;
	NEG_B_ON;
	for(i = 0; i < SOUND_CONST / TONE[2]; i++) {
		POS_C_ON;
		Delay(SOUND_E);
		if(MessAD(2) > 50) { MosfetOkay |= 0x04; } 
		else 			   { MosfetOkay &= ~0x04;}
		POS_ABC_OFF;
		Delay(TONE[2]);
	}
	FETS_OFF

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//测量每相的下臂MOS管（也就是X-管）能否导通
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
	//volatile uint8_t tmpA[200] = {0,}, tmpB[200] = {0,};
	//(1)B+管和A-管导通
    NEG_A_ON;
    for(i=0; i< SOUND_CONST / TONE[3]; i++) {
		POS_B_ON;
		Delay(SOUND_E);
		if(MessAD(0) > 128) { MosfetOkay &= ~0x08;} 
		else 				{ MosfetOkay |= 0x08; }
		POS_ABC_OFF;
		Delay(TONE[3]);
	}		
	FETS_OFF;

	//(2)B+管和C-管导通
	NEG_C_ON;
	for(i = 0; i < SOUND_CONST / TONE[4]; i++) {
		POS_B_ON;
		Delay(SOUND_E);
		if(MessAD(2) > 128) { MosfetOkay &= ~0x20;} 
		else 				{ MosfetOkay |= 0x20; }
		POS_ABC_OFF;
		Delay(TONE[4]);
	}
	FETS_OFF;

	//(3)C+管和B-管导通
	NEG_B_ON;
	for(i = 0; i < SOUND_CONST / TONE[5]; i++) {
		POS_C_ON;
		Delay(SOUND_E);
		if(MessAD(1) > 128) { MosfetOkay &= ~0x10;} 
		else 				{ MosfetOkay |= 0x10; }
		POS_ABC_OFF
		Delay(TONE[5]);		
	}
	FETS_OFF;
	
	sei();		//!!!开中断

	//若有某个MOS管不正常，则红LED闪烁，进入死循环
	//if(!(MosfetOkay & 0x01))  { anz = 1; } else
	//if(!(MosfetOkay & 0x02))  { anz = 2; } else
	//if(!(MosfetOkay & 0x04))  { anz = 3; } else
	if(!(MosfetOkay & 0x08))  { anz = 4; } else
	if(!(MosfetOkay & 0x10))  { anz = 5; } else
	if(!(MosfetOkay & 0x20))  { anz = 6; }  

	if(anz) while(1) LedRedBlink(anz);
}











 
