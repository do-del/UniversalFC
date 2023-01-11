#include "BLMC.h"

extern uint16_t Strom;
extern uint8_t  Strom_max;
extern uint16_t LeakStrom;
	
extern uint8_t  PWM;
extern uint16_t MinRpmPulse;

extern volatile uint8_t Phase;



//�������
uint8_t MotorStartUp(uint8_t pm) {
	uint16_t i, timer = 300;
	
	DISABLE_SENSE_INT;
	PWM = 5;
	SetPWM(PWM);
	Manual();

	//��������Сת���£�PWM=5ʱ,ĳ2��ͨ��300ms���������Ƿ�ᳬ��12A������������˸������0
	MinRpmPulse = SetDelay(300);
	while(!CheckDelay(MinRpmPulse)) {
		FastStromConvert();
		if(Strom > 190) {		//!!!ԭ��Ϊ120
			ALLPIN_AND_PWM_OFF;
			LedRedBlink(10);
			return 0;
		}
	}

	volatile uint16_t count = 0;
	//��ʼ�����PWMֵ����
	PWM = pm;
	Phase = 0;

	while(1) {

		for(i = 0; i < timer; i++) {
			Wait(100);	//Wait(100)=��ʱ0.1ms
		}
		
		//ÿ��ѭ������һ�µ���������6A������˸������0
		FastStromConvert();
		if(Strom > 200) {	//!!!ԭ��Ϊ60,����12V����赽150
			ALLPIN_AND_PWM_OFF;
			LedRedBlink(5);
			return 0;
		}
		
		//timerÿ�μ���1/15�������ٵ�һ���̶���˵���Ѿ�ת������(������ѭ��51��)������1
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

		//���Ƚ����жϱ�־����λ
		if(SENSE)
			LEDGRN_ON;
		

	}
	return 0;
}


//����MOSFET�Լ� 
void MotorTest() {
	uint16_t i, t = 0;
	uint8_t  anz = 0, boundary = 60; //�൱��5A����
	uint8_t  MosfetOkay = 0;
	uint16_t TONE[8] = {6250, 5566, 5000, 4692, 4167, 3756, 3333, 3125};

	DISABLE_SENSE_INT;
	cli();					//!!!�������ж�
	ALLPIN_AND_PWM_OFF;

	Strom_max = 0;
	DelayM(50);
	LeakStrom = Strom_max;

	t = 1000;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//����ÿһ���MOS�ܿ��ض������ԣ��Ա�֤��������MOS��ͬʱ��ͨ
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//(1)A+�ܺ�A-�ܵĿ��ض������Բ���
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
	
	//(2)B+�ܺ�B-�ܵĿ��ض������Բ���
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

	//(3)C+�ܺ�C-�ܵĿ��ض������Բ���
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

	//����MOS�ܲ���,����������˸��������ѭ��
	if(anz) while(1) LedRedBlink(anz);


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//����ÿ����ϱ�MOS�ܣ�Ҳ����X+�ܣ��ܷ�ͨ
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	volatile uint16_t tmpA[200]= {0,};
	#define SOUND_CONST  750000    //��Լ0.5��ʱ��
	#define SOUND_E 	 1		   //

	SENSE_A;
    FETS_OFF;
	Strom = 0;
	//(1)A+�ܺ�B-C-�ܵ�ͨ��ѭ��120�Σ�
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

	//(2)B+�ܺ�A-C-�ܵ�ͨ
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

	//(3)C+�ܺ�A-B-�ܵ�ͨ
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
	//����ÿ����±�MOS�ܣ�Ҳ����X-�ܣ��ܷ�ͨ
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
	//volatile uint8_t tmpA[200] = {0,}, tmpB[200] = {0,};
	//(1)B+�ܺ�A-�ܵ�ͨ
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

	//(2)B+�ܺ�C-�ܵ�ͨ
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

	//(3)C+�ܺ�B-�ܵ�ͨ
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
	
	sei();		//!!!���ж�

	//����ĳ��MOS�ܲ����������LED��˸��������ѭ��
	//if(!(MosfetOkay & 0x01))  { anz = 1; } else
	//if(!(MosfetOkay & 0x02))  { anz = 2; } else
	//if(!(MosfetOkay & 0x04))  { anz = 3; } else
	if(!(MosfetOkay & 0x08))  { anz = 4; } else
	if(!(MosfetOkay & 0x10))  { anz = 5; } else
	if(!(MosfetOkay & 0x20))  { anz = 6; }  

	if(anz) while(1) LedRedBlink(anz);
}











 
