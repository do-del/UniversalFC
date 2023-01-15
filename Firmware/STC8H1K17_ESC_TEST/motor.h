#ifndef __MOTOR_H
#define __MOTOR_H

#define delay_200ns() do{_nop_();_nop_();_nop_();_nop_();}while(0) //根据MOS管手册调整死区时间，现采用的MOS管导通关断时间最大为55ns，此处调整为200ns延时
#define delay_dead() delay_200ns()

void	Delay_n_ms(u8 dly)	// N ms延时函数
{
	u16	j;
	do
	{
		j = FOSC / 10000;
		while(--j)	;
	}while(--dly);
}

void delay_us(u8 us)	//N us延时函数
{
	do
	{
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();	//@24MHz
	}
	while(--us);
}

//换相函数
//根据电机通道序号，给各相通电
void Motor_Step(void)
{
	switch(step)
	{
		case 0: //AB相通电，期间C相感应电动势由负到正变化，A上管导通，B下管导通，其余MOS管关断
			PWMA_ENO = 0x00; //关闭所有PWM输出，特别是上一步C相上管导通，需先关断，然后开启A相上管
			PWM1_L = 0; //A相下管关断
			PWM3_L = 0; //C相下管关断
			delay_dead(); //延迟，防止AC相上管同时导通
			PWMA_ENO = 0x01; //打开A相上管PWM
			PWM2_L = 1;	//B相下管导通
			ADC_CONTR = 0x80+10; //选择ADC10（即P0.2）采样
			if(m_running) CMPCR1 = 0x8c + 0x10; //比较器下降沿中断使能
			else CMPCR1 = 0x8c; //电机启动时关闭比较器中断
			break;
		case 1:	//AC相通电，期间B相感应电动势由正到负变化，A相上管导通，C相下管导通，其余MOS管关断
			PWMA_ENO = 0x01;	//A相上管导通
			PWM1_L = 0;	//A相下管关断
			PWM2_L = 0; //B相下管关断
			delay_dead(); //延迟，防止BC相下管同时导通
			PWM3_L = 1;	//C相下管导通
			ADC_CONTR = 0x80 + 9;	//选择ADC9（P0.1）采样
			if(m_running) CMPCR1 = 0x8c + 0x20; //比较器上升沿中断使能
			else CMPCR1 = 0x8c;	//电机启动时关闭比较器中断
			break;
		case 2:	//BC相通电，期间A相感应电动势由负到正变化，B相上管导通，C相下管导通，其余MOS管关断
			PWMA_ENO = 0x00; //关闭所有PWM输出，特别是上一步A相上管导通，需先关断，然后才能开启B相上管
			PWM1_L = 0;	//A相下管关断
			PWM2_L = 0; //B相下管关断
			delay_dead();
			PWMA_ENO = 0x04;	//打开B相上管PWM
			PWM3_L = 1;	//C相下管导通
			ADC_CONTR = 0x80+8;		//选择ADC8（P0.0）采样
			if(m_running) CMPCR1 = 0x8c + 0x10; //比较器下降沿中断使能
			else CMPCR1 = 0x8c; //电机启动时关闭比较器中断
			break;
		case 3: //BA相通电，期间C相感应电动势由正到负变化，B相上管导通，A相下管导通，其余MOS管关断
			PWMA_ENO = 0x04;	//打开B相上管，关闭A和C相上管
			PWM2_L = 0;	//B相下管关断
			PWM3_L = 0; //C相下管关断
			delay_dead();	//死区延时，错开AC相下管死区
			PWM1_L = 1;	//A相下管导通
			ADC_CONTR = 0x80 + 10;	//选择ADC10（P0.2）采样
			if(m_running) CMPCR1 = 0x8c + 0x20; //比较器上升沿中断使能
			else CMPCR1 = 0x8c;	//电机启动时关闭比较器中断
			break;
		case 4: //CA相通电，期间B相感应电动势由负到正变化，C相上管导通，A相下管导通，其余MOS管关断
			PWMA_ENO = 0x00;	//关闭所有上管PWM，防止BC相上管同时导通
			PWM2_L = 0;	//B相下管关断
			PWM3_L = 0;	//C相下管关断
			delay_dead();	//死区延时，错开BC相上管死区
			PWMA_ENO = 0x10;	//使能C相上管PWM
			PWM1_L = 1;	//A相下管导通
			ADC_CONTR = 0x80+9;	//ADC9（P0.1）采样
			if(m_running) CMPCR1 = 0x8c + 0x10; //比较器下降沿中断使能
			else CMPCR1 = 0x8c;	//电机启动时关闭比较器中断
			break;
		case 5:	//CB相通电，期间A相感应电动势由正到负变化，C相上管导通，B相下管导通，其余MOS管关断
			PWMA_ENO = 0x10;	//使能C相上管PWM
			PWM1_L = 0;	//A相下管关断
			PWM3_L = 0;	//C相下管关断
			delay_dead();	//死区延时，错开AB相下管死区
			PWM2_L = 1;	//B相下管导通
			ADC_CONTR = 0x80 + 8;	//ADC8（P0.0）采样
			if(m_running) CMPCR1 = 0x8c + 0x20; //比较器上升沿中断使能
			else CMPCR1 = 0x8c; //电机启动时关闭比较器中断
			break;
		default:
			break;
	}
}

void Motor_Start(void)
{
	u16 timer,i;
	CMPCR1 = 0x8C;	// 关比较器中断

	PWM_Value  = D_START_PWM;	// 初始占空比, 根据电机特性设置
	PWMA_CCR1L = PWM_Value;
	PWMA_CCR2L = PWM_Value;
	PWMA_CCR3L = PWM_Value;
	step = 0;
	Motor_Step();
	Delay_n_ms(50);
	//Delay_n_ms(250);// 初始位置
	timer = 200;	//风扇电机启动

	while(1)
	{
		for(i=0; i<timer; i++)	delay_us(100);  //根据电机加速特性, 最高转速等等调整启动加速速度
		timer -= timer /16;
		if(++step >= 6)	step = 0;
		Motor_Step();
		if(timer < 40)	return;
	}
}

#endif
