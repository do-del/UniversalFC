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
u32 wid;
bit en;

u8 count_tmp;

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
	
	wid = WID_DEFAULT;
	en = 1;
	
	OLED_ShowString(25,0,"--TESTER--");
	OLED_ShowString(8,2,"Freq(Hz):  50");
	OLED_ShowString(8,4,"Wid(us) :    ");
	OLED_ShowNum(80,4,wid,4,16);
	OLED_ShowString(8,6,"Output  :  ON"); 
	
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
			OLED_ShowString(8,4,"Wid(us) :    ");
			OLED_ShowNum(80,4,wid,4,16);
			PwmSetHighUs(wid,wid,wid);
		}
		
		if(!EC_KEY)
		{
			delay_ms(5);
			if(!EC_KEY)
			{
				en = !en;
				if(en)
				{
					OLED_ShowString(8,6,"Output  :  ON");
					CR = 1;	//启动PCA计时器
				}
				else
				{
					OLED_ShowString(8,6,"Output  : OFF");
					CR = 0;	//启动PCA计时器
				}
				while(!EC_KEY);
			}
		}
		delay_ms(5);
	}
}