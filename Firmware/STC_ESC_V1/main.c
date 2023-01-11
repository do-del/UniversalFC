#define MAIN_Fosc		24000000L	//定义主时钟

#include	"STC8Hxxx.h"


#define ADC_START	(1<<6)	/* 自动清0 */
#define ADC_FLAG	(1<<5)	/* 软件清0 */

#define	ADC_SPEED	1		/* 0~15, ADC时钟 = SYSclk/2/(n+1) */
#define	RES_FMT		(1<<5)	/* ADC结果格式 0: 左对齐, ADC_RES: D9 D8 D7 D6 D5 D4 D3 D2, ADC_RESL: D1 D0 0  0  0  0  0  0 */
							/*             1: 右对齐, ADC_RES: 0  0  0  0  0  0  D9 D8, ADC_RESL: D7 D6 D5 D4 D3 D2 D1 D0 */

#define CSSETUP		(0<<7)	/* 0~1,  ADC通道选择时间      0: 1个ADC时钟, 1: 2个ADC时钟,  默认0(默认1个ADC时钟) */
#define CSHOLD		(1<<5)	/* 0~3,  ADC通道选择保持时间  (n+1)个ADC时钟, 默认1(默认2个ADC时钟)                */
#define SMPDUTY		20		/* 10~31, ADC模拟信号采样时间  (n+1)个ADC时钟, 默认10(默认11个ADC时钟)				*/
							/* ADC转换时间: 10位ADC固定为10个ADC时钟, 12位ADC固定为12个ADC时钟. 				*/

sbit PWM1   = P1^0;
sbit PWM1_L = P1^1;
sbit PWM2   = P1^2;
sbit PWM2_L = P1^3;
sbit PWM3   = P1^4;
sbit PWM3_L = P1^5;

u8	step;		//切换步骤
u8	PWM_Value;	// 决定PWM占空比的值
bit	B_RUN;		//运行标志
u8	PWW_Set;	//目标PWM设置
u16	adc11;
bit	B_4ms;		//4ms定时标志

u8	TimeOut;	//堵转超时
bit	B_start;	//启动模式
bit	B_Timer3_OverFlow;

u8	TimeIndex;		//换相时间保存索引
u16	PhaseTimeTmp[8];//8个换相时间, 其 sum/16 就是30度电角度
u16	PhaseTime;		//换相时间计数
u8	XiaoCiCnt;		//1:需要消磁, 2:正在消磁, 0已经消磁


/*************************/

void	Delay_n_ms(u8 dly)	// N ms延时函数
{
	u16	j;
	do
	{
		j = MAIN_Fosc / 10000;
		while(--j)	;
	}while(--dly);
}

void delay_us(u8 us)	//N us延时函数
{
	do
	{
		NOP(20);	//@24MHz
	}
	while(--us);
}

//========================================================================
// 函数: u16	Get_ADC10bitResult(u8 channel))	//channel = 0~15
//========================================================================
u16	Get_ADC10bitResult(u8 channel)	//channel = 0~15
{
	u8 i;
	ADC_RES = 0;
	ADC_RESL = 0;
	ADC_CONTR = 0x80 | ADC_START | channel; 
	NOP(5);			//
//	while((ADC_CONTR & ADC_FLAG) == 0)	;	//等待ADC结束
		i = 255;
		while(i != 0)
		{
			i--;
			if((ADC_CONTR & ADC_FLAG) != 0)	break;	//等待ADC结束
		}
	ADC_CONTR &= ~ADC_FLAG;
	return	((u16)ADC_RES * 256 + (u16)ADC_RESL);
}


void	Delay_500ns(void)
{
	NOP(6);
}

void StepMotor(void) // 换相序列函数
{
	switch(step)
	{
	case 0:  // AB  PWM1, PWM2_L=1
			PWMA_ENO = 0x00;	PWM1_L=0;	PWM3_L=0;
			Delay_500ns();
			PWMA_ENO = 0x01;		// 打开A相的高端PWM
			PWM2_L = 1;				// 打开B相的低端
			ADC_CONTR = 0x80+10;	// 选择P0.2作为ADC输入 即C相电压
			CMPCR1 = 0x8c + 0x10;	//比较器下降沿中断
			break;
	case 1:  // AC  PWM1, PWM3_L=1
			PWMA_ENO = 0x01;	PWM1_L=0;	PWM2_L=0;	// 打开A相的高端PWM
			Delay_500ns();
			PWM3_L = 1;				// 打开C相的低端
			ADC_CONTR = 0x80+9;		// 选择P0.1作为ADC输入 即B相电压
			CMPCR1 = 0x8c + 0x20;	//比较器上升沿中断
			break;
	case 2:  // BC  PWM2, PWM3_L=1
			PWMA_ENO = 0x00;	PWM1_L=0;	PWM2_L=0;
			Delay_500ns();
			PWMA_ENO = 0x04;		// 打开B相的高端PWM
			PWM3_L = 1;				// 打开C相的低端
			ADC_CONTR = 0x80+8;		// 选择P0.0作为ADC输入 即A相电压
			CMPCR1 = 0x8c + 0x10;	//比较器下降沿中断
			break;
	case 3:  // BA  PWM2, PWM1_L=1
			PWMA_ENO = 0x04;	PWM2_L=0;	PWM3_L=0;	// 打开B相的高端PWM
			Delay_500ns();
			PWM1_L = 1;				// 打开C相的低端
			ADC_CONTR = 0x80+10;	// 选择P0.2作为ADC输入 即C相电压
			CMPCR1 = 0x8c + 0x20;	//比较器上升沿中断
			break;
	case 4:  // CA  PWM3, PWM1_L=1
			PWMA_ENO = 0x00;	PWM2_L=0;	PWM3_L=0;
			Delay_500ns();
			PWMA_ENO = 0x10;		// 打开C相的高端PWM
			PWM1_L = 1;				// 打开A相的低端
			adc11 = ((adc11 *7)>>3) + Get_ADC10bitResult(11);
			ADC_CONTR = 0x80+9;		// 选择P0.1作为ADC输入 即B相电压
			CMPCR1 = 0x8c + 0x10;	//比较器下降沿中断
			break;
	case 5:  // CB  PWM3, PWM2_L=1
			PWMA_ENO = 0x10;	PWM1_L=0;	PWM3_L=0;	// 打开C相的高端PWM
			Delay_500ns();
			PWM2_L = 1;				// 打开B相的低端
			ADC_CONTR = 0x80+8;		// 选择P0.0作为ADC输入 即A相电压
			CMPCR1 = 0x8c + 0x20;	//比较器上升沿中断
			break;

	default:
			break;
	}

	if(B_start)		CMPCR1 = 0x8C;	// 启动时禁止下降沿和上升沿中断
}



void PWMA_config(void)
{
	P_SW2 |= 0x80;		//SFR enable   

	PWM1   = 0;
	PWM1_L = 0;
	PWM2   = 0;
	PWM2_L = 0;
	PWM3   = 0;
	PWM3_L = 0;
	P1n_push_pull(0x3f);

	PWMA_PSCR = 3;		// 预分频寄存器, 分频 Fck_cnt = Fck_psc/(PSCR[15:0}+1), 边沿对齐PWM频率 = SYSclk/((PSCR+1)*(AAR+1)), 中央对齐PWM频率 = SYSclk/((PSCR+1)*(AAR+1)*2).
	PWMA_DTR  = 24;		// 死区时间配置, n=0~127: DTR= n T,   0x80 ~(0x80+n), n=0~63: DTR=(64+n)*2T,  
						//				0xc0 ~(0xc0+n), n=0~31: DTR=(32+n)*8T,   0xE0 ~(0xE0+n), n=0~31: DTR=(32+n)*16T,
	PWMA_ARR    = 255;	// 自动重装载寄存器,  控制PWM周期
	PWMA_CCER1  = 0;
	PWMA_CCER2  = 0;
	PWMA_SR1    = 0;
	PWMA_SR2    = 0;
	PWMA_ENO    = 0;
	PWMA_PS     = 0;
	PWMA_IER    = 0;
//	PWMA_ISR_En = 0;

	PWMA_CCMR1  = 0x68;		// 通道模式配置, PWM模式1, 预装载允许
	PWMA_CCR1   = 0;		// 比较值, 控制占空比(高电平时钟数)
	PWMA_CCER1 |= 0x05;		// 开启比较输出, 高电平有效
	PWMA_PS    |= 0;		// 选择IO, 0:选择P1.0 P1.1, 1:选择P2.0 P2.1, 2:选择P6.0 P6.1, 
//	PWMA_ENO   |= 0x01;		// IO输出允许,  bit7: ENO4N, bit6: ENO4P, bit5: ENO3N, bit4: ENO3P,  bit3: ENO2N,  bit2: ENO2P,  bit1: ENO1N,  bit0: ENO1P
//	PWMA_IER   |= 0x02;		// 使能中断

	PWMA_CCMR2  = 0x68;		// 通道模式配置, PWM模式1, 预装载允许
	PWMA_CCR2   = 0;		// 比较值, 控制占空比(高电平时钟数)
	PWMA_CCER1 |= 0x50;		// 开启比较输出, 高电平有效
	PWMA_PS    |= (0<<2);	// 选择IO, 0:选择P1.2 P1.3, 1:选择P2.2 P2.3, 2:选择P6.2 P6.3, 
//	PWMA_ENO   |= 0x04;		// IO输出允许,  bit7: ENO4N, bit6: ENO4P, bit5: ENO3N, bit4: ENO3P,  bit3: ENO2N,  bit2: ENO2P,  bit1: ENO1N,  bit0: ENO1P
//	PWMA_IER   |= 0x04;		// 使能中断

	PWMA_CCMR3  = 0x68;		// 通道模式配置, PWM模式1, 预装载允许
	PWMA_CCR3   = 0;		// 比较值, 控制占空比(高电平时钟数)
	PWMA_CCER2 |= 0x05;		// 开启比较输出, 高电平有效
	PWMA_PS    |= (0<<4);	// 选择IO, 0:选择P1.4 P1.5, 1:选择P2.4 P2.5, 2:选择P6.4 P6.5, 
//	PWMA_ENO   |= 0x10;		// IO输出允许,  bit7: ENO4N, bit6: ENO4P, bit5: ENO3N, bit4: ENO3P,  bit3: ENO2N,  bit2: ENO2P,  bit1: ENO1N,  bit0: ENO1P
//	PWMA_IER   |= 0x08;		// 使能中断

	PWMA_BKR    = 0x80;		// 主输出使能 相当于总开关
	PWMA_CR1    = 0x81;		// 使能计数器, 允许自动重装载寄存器缓冲, 边沿对齐模式, 向上计数,  bit7=1:写自动重装载寄存器缓冲(本周期不会被打扰), =0:直接写自动重装载寄存器本(周期可能会乱掉)
	PWMA_EGR    = 0x01;		//产生一次更新事件, 清除计数器和与分频计数器, 装载预分频寄存器的值
//	PWMA_ISR_En = PWMA_IER;	//设置标志允许通道1~4中断处理
}

//	PWMA_PS   = (0<<6)+(0<<4)+(0<<2)+0;	//选择IO, 4项从高到低(从左到右)对应PWM1 PWM2 PWM3 PWM4, 0:选择P1.x, 1:选择P2.x, 2:选择P6.x, 
//  PWMA_PS    PWM4N PWM4P    PWM3N PWM3P    PWM2N PWM2P    PWM1N PWM1P
//    00       P1.7  P1.6     P1.5  P1.4     P1.3  P1.2     P1.1  P1.0
//    01       P2.7  P2.6     P2.5  P2.4     P2.3  P2.2     P2.1  P2.0
//    02       P6.7  P6.6     P6.5  P6.4     P6.3  P6.2     P6.1  P6.0
//    03       P3.3  P3.4      --    --       --    --       --    --


void ADC_config(void)	//ADC初始化函数(为了使用ADC输入端做比较器信号, 实际没有启动ADC转换)
{
	P1n_pure_input(0xc0);	//设置为高阻输入
	P0n_pure_input(0x0f);	//设置为高阻输入
	ADC_CONTR = 0x80 + 6;	//ADC on + channel
	ADCCFG = RES_FMT + ADC_SPEED;
	P_SW2 |=  0x80;	//访问XSFR
	ADCTIM = CSSETUP + CSHOLD + SMPDUTY;
}

void CMP_config(void)	//比较器初始化程序
{
	CMPCR1 = 0x8C;			// 1000 1100 打开比较器，P3.6作为比较器的反相输入端，ADC引脚作为正输入端 
	CMPCR2 = 60;			//60个时钟滤波   比较结果变化延时周期数, 0~63
	P3n_pure_input(0x40);	//CMP-(P3.6)设置为高阻.
	
	P_SW2 |= 0x80;		//SFR enable   
//	CMPEXCFG |= (0<<6);	//bit7 bit6: 比较器迟滞输入选择: 0: 0mV,  1: 10mV, 2: 20mV, 3: 30mV
//	CMPEXCFG |= (0<<2);	//bit2: 输入负极性选择, 0: 选择P3.6做输入,   1: 选择内部BandGap电压BGv做负输入.
//	CMPEXCFG |=  0;		//bit1 bit0: 输入正极性选择, 0: 选择P3.7做输入,   1: 选择P5.0做输入,  2: 选择P5.1做输入,  3: 选择ADC输入(由ADC_CHS[3:0]所选择的ADC输入端做正输入).
//	CMPEXCFG = (0<<6)+(0<<2)+3;
}

void CMP_ISR(void) interrupt 21		//比较器中断函数, 检测到反电动势过0事件
{
	u8	i;
	CMPCR1 &= ~0x40;	// 需软件清除中断标志位

	if(XiaoCiCnt == 0)	//消磁后才检测过0事件,   XiaoCiCnt=1:需要消磁, =2:正在消磁, =0已经消磁
	{
		T4T3M &= ~(1<<3);	// Timer3停止运行
		if(B_Timer3_OverFlow)	//切换时间间隔(Timer3)有溢出
		{
			B_Timer3_OverFlow = 0;
			PhaseTime = 8000;	//换相时间最大8ms, 2212电机12V空转最高速130us切换一相(200RPS 12000RPM), 480mA
		}
		else
		{
			PhaseTime = (((u16)T3H << 8) + T3L) >> 1;	//单位为1us
			if(PhaseTime >= 8000)	PhaseTime = 8000;	//换相时间最大8ms, 2212电机12V空转最高速130us切换一相(200RPS 12000RPM), 480mA
		}
		T3H = 0;	T3L = 0;
		T4T3M |=  (1<<3);	//Timer3开始运行

		PhaseTimeTmp[TimeIndex] = PhaseTime;	//保存一次换相时间
		if(++TimeIndex >= 8)	TimeIndex = 0;	//累加8次
		for(PhaseTime=0, i=0; i<8; i++)	PhaseTime += PhaseTimeTmp[i];	//求8次换相时间累加和
		PhaseTime = PhaseTime >> 4;		//求8次换相时间的平均值的一半, 即30度电角度
		if((PhaseTime >= 40) && (PhaseTime <= 1000))	TimeOut = 125;	//堵转500ms超时
		if( PhaseTime >= 60)	PhaseTime -= 40;	//修正由于滤波电容引起的滞后时间
		else					PhaseTime  = 20;
		
	//	PhaseTime = 20;	//只给20us, 则无滞后修正, 用于检测滤波电容引起的滞后时间
		T4T3M &= ~(1<<7);				//Timer4停止运行
		PhaseTime  = PhaseTime  << 1;	//2个计数1us
		PhaseTime = 0 - PhaseTime;
		T4H = (u8)(PhaseTime >> 8);		//装载30度角延时
		T4L = (u8)PhaseTime;
		T4T3M |=  (1<<7);	//Timer4开始运行
		XiaoCiCnt = 1;		//1:需要消磁, 2:正在消磁, 0已经消磁
	}
}

void Timer0_config(void)	//Timer0初始化函数
{
	Timer0_16bitAutoReload(); // T0工作于16位自动重装
	Timer0_12T();
	TH0 = (65536UL-MAIN_Fosc/12 / 250) / 256; //4ms
	TL0 = (65536UL-MAIN_Fosc/12 / 250) % 256;
	TR0 = 1; // 打开定时器0

	ET0 = 1;// 允许ET0中断
}

void Timer0_ISR(void) interrupt 1	//Timer0中断函数, 20us
{
	B_4ms = 1;	//4ms定时标志
}

//============================ timer3初始化函数 ============================================
void	Timer3_Config(void)
{
	P_SW2 |= 0x80;		//SFR enable   
	T4T3M &= 0xf0;		//停止计数, 定时模式, 12T模式, 不输出时钟
	T3H = 0;
	T3L = 0;

	T3T4PIN = 0x01;		//选择IO, 0x00: T3--P0.4, T3CLKO--P0.5, T4--P0.6, T4CLKO--P0.7;    0x01: T3--P0.0, T3CLKO--P0.1, T4--P0.2, T4CLKO--P0.3;
	IE2   |=  (1<<5);	//允许中断
	T4T3M |=  (1<<3);	//开始运行
}

//============================ timer4初始化函数 ============================================
void	Timer4_Config(void)
{
	P_SW2 |= 0x80;		//SFR enable   
	T4T3M &= 0x0f;		//停止计数, 定时模式, 12T模式, 不输出时钟
	T4H = 0;
	T4L = 0;

	T3T4PIN = 0x01;		//选择IO, 0x00: T3--P0.4, T3CLKO--P0.5, T4--P0.6, T4CLKO--P0.7;    0x01: T3--P0.0, T3CLKO--P0.1, T4--P0.2, T4CLKO--P0.3;
	IE2   |=  (1<<6);	//允许中断
//	T4T3M |=  (1<<7);	//开始运行
}

//=========================== timer3中断函数 =============================================
void timer3_ISR (void) interrupt TIMER3_VECTOR
{
	B_Timer3_OverFlow = 1;	//溢出标志
}

//=========================== timer4中断函数 =============================================
void timer4_ISR (void) interrupt TIMER4_VECTOR
{
	T4T3M &= ~(1<<7);	//Timer4停止运行
	if(XiaoCiCnt == 1)		//标记需要消磁. 每次检测到过0事件后第一次中断为30度角延时, 设置消磁延时.
	{
		XiaoCiCnt = 2;		//1:需要消磁, 2:正在消磁, 0已经消磁
		if(B_RUN)	//电机正在运行
		{
			if(++step >= 6)	step = 0;
			StepMotor();
		}
										//消磁时间, 换相后线圈(电感)电流减小到0的过程中, 出现反电动势, 电流越大消磁时间越长, 过0检测要在这个时间之后
										//100%占空比时施加较重负载, 电机电流上升, 可以示波器看消磁时间.
										//实际上, 只要在换相后延时几十us才检测过零, 就可以了
		T4H = (u8)((65536UL - 40*2) >> 8);	//装载消磁延时
		T4L = (u8)(65536UL - 40*2);
		T4T3M |=  (1<<7);	//Timer4开始运行
	}
	else if(XiaoCiCnt == 2)	XiaoCiCnt = 0;		//1:需要消磁, 2:正在消磁, 0已经消磁
}


#define	D_START_PWM		30
/******************* 强制电机启动函数 ***************************/
void StartMotor(void)
{
	u16 timer,i;
	CMPCR1 = 0x8C;	// 关比较器中断

	PWM_Value  = D_START_PWM;	// 初始占空比, 根据电机特性设置
	PWMA_CCR1L = PWM_Value;
	PWMA_CCR2L = PWM_Value;
	PWMA_CCR3L = PWM_Value;
	step = 0;	StepMotor();	Delay_n_ms(50);	//Delay_n_ms(250);// 初始位置
	timer = 200;	//风扇电机启动

	while(1)
	{
		for(i=0; i<timer; i++)	delay_us(100);  //根据电机加速特性, 最高转速等等调整启动加速速度
		timer -= timer /16;
		if(++step >= 6)	step = 0;
		StepMotor();
		if(timer < 40)	return;
	}
}

/**********************************************/
void main(void)
{
	u8	i;
	u16	j;
	
	P2n_standard(0xf8);
	P3n_standard(0xbf);
	P5n_standard(0x10);
	
	adc11 = 0;
	
	PWMA_config();
	ADC_config();
	CMP_config();
	Timer0_config();	// Timer0初始化函数
	Timer3_Config();	// Timer3初始化函数
	Timer4_Config();	// Timer4初始化函数
	PWW_Set = 0;
	TimeOut = 0;

	EA  = 1; // 打开总中断

	
	while (1)
	{
		if(B_4ms)		// 4ms时隙
		{
			B_4ms = 0;

			if(TimeOut != 0)
			{
				if(--TimeOut == 0)	//堵转超时
				{
					B_RUN  = 0;
					PWM_Value = 0;
					CMPCR1 = 0x8C;	// 关比较器中断
					PWMA_ENO  = 0;
					PWMA_CCR1L = 0;	PWMA_CCR2L = 0;	PWMA_CCR3L = 0;
					PWM1_L=0;	PWM2_L=0;	PWM3_L=0;
					Delay_n_ms(250);	//堵转时,延时一点时间再启动
				}
			}

			if(!B_RUN && (PWW_Set >= D_START_PWM))	// 占空比大于设定值, 并且电机未运行, 则启动电机
			{
				B_start = 1;		//启动模式
				for(i=0; i<8; i++)	PhaseTimeTmp[i] = 400;
				StartMotor();		// 启动电机
				B_start = 0;
				XiaoCiCnt = 0;		//初始进入时
				CMPCR1 &= ~0x40;	// 清除中断标志位
				if(step & 1)	CMPCR1 = 0xAC;		//上升沿中断
				else			CMPCR1 = 0x9C;		//下降沿中断
				B_RUN = 1;
				Delay_n_ms(250);	//延时一下, 先启动起来
				Delay_n_ms(250);
				TimeOut = 125;		//启动超时时间 125*4 = 500ms
			}

			if(B_RUN)	//正在运行中
			{
				if(PWM_Value < PWW_Set)	PWM_Value++;	//油门跟随电位器
				if(PWM_Value > PWW_Set)	PWM_Value--;
				if(PWM_Value < (D_START_PWM-10))	// 停转, 停转占空比 比 启动占空比 小10/256
				{
					B_RUN = 0;
					PWM_Value = 0;
					CMPCR1 = 0x8C;	// 关比较器中断
					PWMA_ENO  = 0;
					PWMA_CCR1L = 0;	PWMA_CCR2L = 0;	PWMA_CCR3L = 0;
					PWM1_L=0;	PWM2_L=0;	PWM3_L=0;
				}
				else
				{
					PWMA_CCR1L = PWM_Value;
					PWMA_CCR2L = PWM_Value;
					PWMA_CCR3L = PWM_Value;
				}
			}
			else
			{
				adc11 = ((adc11 *7)>>3) + Get_ADC10bitResult(11);
			}
			
			j = adc11;
			if(j != adc11)	j = adc11;
			PWW_Set = (u8)(j >> 5);	//油门是8位的
		}
	}
}