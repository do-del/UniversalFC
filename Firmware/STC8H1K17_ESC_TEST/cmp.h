#ifndef __CMP_H
#define __CMP_H

#define TMP_LEN 8
u8 time_index;
u16 phase_time_tmp[TMP_LEN];

//比较器初始化
void CMP_Config(void)
{
	//设置CMP-引脚为高阻输入，M1相应bit为1，M0相应bit为0
	P3M1 |= 0x40; //0x40 = 0b0100 0000,bit6置1
	P3M0 &= ~0x40; //~0x40 = 0b1011 1111，bit6置0
	
	CMPCR1 = 0x8C;	//比较器控制寄存器1。bit7模块使能，bit6中断标志位，bit5上升沿中断使能，bit4下降沿中断使能
									//bit3正极选择，bit2负极选择，bit1结果输出控制，bit0比较结果
									//0x80 = 0b1000 1100，使能比较器，ADC为正极输入，P3.6为负极输入
	CMPCR2 = 60; //比较器控制寄存器2，bit7输出方向控制，bit6模拟滤波控制，置0使能，bit5~bit0滤波时钟数
}

//比较器中断函数
//检测到过零事件后，根据定时器3的计数值计算离上一次过零事件的时间
//求出8次换相所用时间除以16，计算每半次换相所需时间，即为离下次换相所需时间
void CMP_ISR(void) interrupt 21
{
	u16 phase_time;
	u8 i;
	
	CMPCR1 &= ~0x40; //清除中断标志位
	
	if(demagnetizing_cnt == 0) //消磁后检测过零事件，未消磁不处理比较器中断
	{
		//T4T3M &= ~(1<<3); //Timer3停止计数
		TR1 = 0;
		//if(t3_flag)
		if(t1_flag)
		{
			//t3_flag = 0;
			t1_flag = 0;
			phase_time = 0x1fff;
		}
		else
		{
			//phase_time = (((u16)T3H<<8)+(u16)T3L)>>1;
			phase_time = (((u16)TH1<<8)+(u16)TL1)>>1;
			phase_time &= 0x1ffff;
		}
		//T3H = 0;
		//T3L = 0;
		//T4T3M |= (1<<3);
		TH1 = 0;
		TL1 = 0;
		TR1 = 1;
		
		phase_time_tmp[time_index] = phase_time;
		if(++time_index >=8) time_index = 0;
		for(phase_time = 0,i = 0;i<8;i++)
		{
			phase_time += phase_time_tmp[i];
		}
		phase_time >>= 4;
		if(phase_time > 40 && phase_time < 1000)
		{
			timeout = 125;
		}
		if(phase_time > 40) phase_time -= 40;
		else phase_time = 20;
		
		/*
		T4T3M &= ~(1<<7);
		phase_time = phase_time << 1;
		phase_time = 0 - phase_time;
		T4H = (u8)(phase_time >> 8);
		T4L = (u8)phase_time;
		T4T3M |= (1<<7);
		demagnetizing_cnt = 1;
		*/
		
		AUXR &= ~(0x10);;
		phase_time = phase_time << 1;
		phase_time = 0 - phase_time;
		T2H = (u8)(phase_time >> 8);
		T2L = (u8)phase_time;
		AUXR |= 0x10; //Timer2开始计数
		demagnetizing_cnt = 1;
	}
}

#endif