#include <stc8g.h>
#include <intrins.h>
#include "sys.h"
#include "oled.h"
#include "timer.h"
#include "pwm.h"
#include "encoder.h"


void main(void)
{
	P_SW2 |= 0x80;
	
	P0M0 = 0x00;
	P0M1 = 0x00;
	P1M0 = 0x00;
	P1M1 = 0x00;
	P2M0 = 0x00;
	P2M1 = 0x00;
	P3M0 = 0x00;
	P3M1 = 0x00;
	P5M0 = 0x00;
	P5M1 = 0x00;
	
	Timer0Init();
	PwmInit();
	Int0_Init();
	
	OLED_Init();
	OLED_Clear();
	OLED_ShowString(30,0,"OLED TEST");
	OLED_ShowString(8,2,"ZHONGJINGYUAN");  
	OLED_ShowString(20,4,"2014/05/01");  
	OLED_ShowString(0,6,"ASCII:");  
	
	EA = 1; //使能总中断
	while(1)
	{
		if(encoder_flag)
		{
			encoder_flag = 0;
			if(encoder_counter>0)
				OLED_ShowString(63,6,"UP  ");
			else if(encoder_counter<0)
				OLED_ShowString(63,6,"DOWN");
			else
				OLED_ShowString(63,6,"NONE");
			encoder_counter = 0;
		}
		Delay_n_ms(5);
	}
}