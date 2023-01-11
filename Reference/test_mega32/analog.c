#include "analog.h"

extern uint16_t Strom;
extern uint8_t  Strom_max;
extern uint16_t LeakStrom;

//测量入参通道的AD值,返回测量值
uint16_t MessAD(uint8_t channel) {
	uint8_t sense;
	sense  = ADMUX;
	ADMUX  = channel;	//总是选外接AREF的5V参考电压
	SFIOR  = 0;
	ADCSRA = 0xD3;		//启动首次转换（25个周期,一般不会正确）

	ADCSRA |= 0x10;		//清除中断标志
	ADMUX  = channel;
	ADCSRA |= 0x40;		//开始一次ADC转换
	while(((ADCSRA & 0x10) == 0));

	//转换完毕
	ADMUX = sense;
	ADCSRA = 0;
	SFIOR = 0x08;		//恢复“比较器多路复用”为使能状态
	return(ADCW);
}

//快速测量电流Strom值
void FastStromConvert() {
	uint16_t i=0;  
	//+++++++++++++++++++
	#ifdef USE_AVR_mega32
		i = MessAD(6) * 4;   //!!!!!!要根据自己的电阻改!!!!
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
 
	//SFIOR = 0x08;  //恢复“比较器多路复用”为使能状态
}

//正常测量电流Strom值（每次新测量的值仅占1/8权重，上次值占7/8权重）
void NormalStromConvert() {
	uint16_t i=0;  
	//+++++++++++++++++++
	#ifdef USE_AVR_mega32
		i = MessAD(6) * 4;  //!!!!!!要根据自己的电阻改!!!!
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
 
	//SFIOR = 0x08;  //恢复“比较器多路复用”为使能状态
}



//测量电流t次（同时改变全局变量Strom）,若测得电流较大，则返回1，否则返回0
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

