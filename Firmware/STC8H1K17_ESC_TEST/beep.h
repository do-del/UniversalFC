#ifndef __BEEP_H
#define __BEEP_H

void beep_1KHz(u16 _ms)
{
	PWM3_L = 1;
	
	while(_ms--)
	{
		PWM1 = 1;
		delay_us(50);
		PWM1 = 0;
		delay_us(225);
		delay_us(225);
		
		PWM2 = 1;
		delay_us(50);
		PWM2 = 0;
		delay_us(225);
		delay_us(225);
	}
	PWM3_L = 0;
	PWM1 = 0;
	PWM2 = 0;
}

#endif
