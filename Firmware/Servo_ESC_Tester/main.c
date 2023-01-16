#include <stc8g.h>
#include <intrins.h>
#include "sys.h"
#include "oled.h"
#include "timer.h"
#include "pwm.h"
#include "encoder.h"

#define WID_UP 2500
#define WID_DOWN 500
#define WID_DEFAULT WID_DOWN
u8 count_tmp;
u32 wid;

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
	
	wid = WID_DEFAULT;
	OLED_Init();
	OLED_Clear();
	OLED_ShowString(8,0,"MODE:SERVO");
	OLED_ShowString(8,2,"Freq(Hz):50");
	OLED_ShowString(8,4,"Wid(us):      ");
	OLED_ShowNum(79,4,wid,4,16);
	OLED_ShowString(8,6,"Out: ON"); 
	
	EA = 1; //使能总中断
	while(1)
	{
		if(encoder_flag)
		{
			count_tmp = count;
			encoder_flag = 0;
			if(up_flag)
				wid += 50;
			else
				wid -= 50;
			if(wid > WID_UP) wid = WID_UP;
			if(wid < WID_DOWN) wid = WID_DOWN;
			OLED_ShowString(8,4,"Wid(us):      ");
			OLED_ShowNum(79,4,wid,4,16);
			PwmSetHighUs(wid,wid,wid);
		}
	}
}