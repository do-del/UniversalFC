#include "analog.h"

extern uint16_t Strom;
extern uint8_t  Strom_max;
extern uint16_t LeakStrom;

//�������ͨ����ADֵ,���ز���ֵ
uint16_t MessAD(uint8_t channel) {
	uint8_t sense;
	sense  = ADMUX;
	ADMUX  = channel;	//����ѡ���AREF��5V�ο���ѹ
	SFIOR  = 0;
	ADCSRA = 0xD3;		//�����״�ת����25������,һ�㲻����ȷ��

	ADCSRA |= 0x10;		//����жϱ�־
	ADMUX  = channel;
	ADCSRA |= 0x40;		//��ʼһ��ADCת��
	while(((ADCSRA & 0x10) == 0));

	//ת�����
	ADMUX = sense;
	ADCSRA = 0;
	SFIOR = 0x08;		//�ָ����Ƚ�����·���á�Ϊʹ��״̬
	return(ADCW);
}

//���ٲ�������Stromֵ
void FastStromConvert() {
	uint16_t i=0;  
	//+++++++++++++++++++
	#ifdef USE_AVR_mega32
		i = MessAD(6) * 4;   //!!!!!!Ҫ�����Լ��ĵ����!!!!
		if(i > 200) i = 200;
	#endif
	//------------------
	#ifdef USE_AVR_mega8
		i = MessAD(6) * 4;
		if(i > 200) i = 200;
	#endif
	//++++++++++++++++++
	Strom = i; 
	if (Strom_max < Strom) Strom_max = Strom;
 
	//SFIOR = 0x08;  //�ָ����Ƚ�����·���á�Ϊʹ��״̬
}

//������������Stromֵ��ÿ���²�����ֵ��ռ1/8Ȩ�أ��ϴ�ֵռ7/8Ȩ�أ�
void NormalStromConvert() {
	uint16_t i=0;  
	//+++++++++++++++++++
	#ifdef USE_AVR_mega32
		i = MessAD(6) * 4;  //!!!!!!Ҫ�����Լ��ĵ����!!!!
		if(i > 200) i = 200;
	#endif
	//------------------
	#ifdef USE_AVR_mega8
		i = MessAD(6) * 4;
		if(i > 200) i = 200;
	#endif
	//+++++++++++++++++
	Strom = (i + Strom * 7) / 8; 
	if (Strom_max < Strom) Strom_max = Strom;
 
	//SFIOR = 0x08;  //�ָ����Ƚ�����·���á�Ϊʹ��״̬
}



//��������t�Σ�ͬʱ�ı�ȫ�ֱ���Strom��,����õ����ϴ��򷵻�1�����򷵻�0
#define TEST_STROMBOUNDARY 120
uint8_t DelayM(uint8_t timer) {
	while(timer--) {
		FastStromConvert();
		if(Strom > (TEST_STROMBOUNDARY + LeakStrom)) {
        	FETS_OFF;
        	return(1);
       	} 
  	}
	return(0);  
}

