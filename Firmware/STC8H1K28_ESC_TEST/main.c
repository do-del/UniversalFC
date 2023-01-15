#include "stc8h.h"
#include <intrins.h>

/*
 * PWM1接P1.0，配置推挽输出和PWM输出，控制电机U相高位MOS管选通信号，高电平导通，低电平截止
 * PWM1_L接P1.1，配置推挽输出，控制电机U相低位MOS管选通信号，高电平导通，低电平截止
 * PWM2接P1.2，配置推挽输出和PWM输出，控制电机V相高位MOS管选通信号，高电平导通，低电平截止
 * PWM2_L接P1.3，配置推挽输出，控制电机V相低位MOS管选通信号，高电平导通，低电平截止
 * PWM3接P1.4，配置推挽输出和PWM输出，控制电机W相高位MOS管选通信号，高电平导通，低电平截止
 * PWM3_L接P1.5，配置推挽输出，控制电机W相低位MOS管选通信号，高电平导通，低电平截止
 * ADC8（P0.0），配置高阻输入，采样电机U相感应电动势，用于过零检测比较器正输入
 * ADC9（P0.1），配置高阻输入，采样电机V相感应电动势
 * ADC10（P0.2），配置高阻输入，采样电机W相感应电动势
 * CMP-（P3.6），配置为高阻输入，接过零检测电路中点
*/

/*
 * 无刷电机通电次序
 * AB-AC-BC-BA-CA-CB-AB
 * 相对应感应电动势变化
 * C下降-B上升-A下降-C上升-B下降-A上升-C下降
*/

#define FOSC 24000000UL	//系统时钟
#define BRT (65536-(FOSC/115200+2)/4) //波特率转计数值

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#define PWM1 		P10
#define PWM1_L 	P11
#define PWM2		P12
#define PWM2_L	P13
#define PWM3		P14
#define PWM3_L	P15

#define ADC_START (1<<6)	//AD从控制寄存器bit6，ADC转换启动控制位，写1开始ADC转换
#define ADC_FLAG (1<<5)	//ADC控制寄存器bit5，ADC转换结束标志位
#define ADC_SPEED 1			//ADC配置寄存器bit3~bit0，设置ADC工作时钟频率
#define RES_FMT (1<<5)	//ADC配置寄存器bit5，置1配置ADC结果右对齐，置0配置ADC结果左对齐
#define CSSETUP (0<<7)	//ADCTIM时序控制寄存器通道选择时间控制位bit7，占用一个时钟数
#define CSHOLD (1<<5)		//ADCTIM时序控制寄存器通道保持时间控制位bit6~bit5，占用2个时钟数
#define SMPDUTY 20			//ADCTIM时序控制寄存器通道采样时间控制位bit4~bit0，占用21个时钟数，不得小于10

#define RX_LEN 16 //串口接收数据缓存长度
bit busy;		//串口发送完成标志
char wptr;	//串口接收数据长度
char buffer[RX_LEN]; //串口接收数据缓存

void Port_Init(void);	//芯片复位后引脚初始化
void PWMA_Config(void);	//PWMA配置函数
void ADC_Config(void);	//ADC配置函数
void CMP_Config(void);	//比较器配置函数
void Timer0_Config(void);	//定时器0配置函数，函数中已开启定时器0及其中断（4ms一次）
void Timer3_Config(void);	//定时器3配置函数
void Timer4_Config(void);	//定时器4配置函数

#define delay_200ns() do{_nop_();_nop_();_nop_();_nop_();}while(0) //根据MOS管手册调整死区时间，现采用的MOS管导通关断时间最大为55ns，此处调整为200ns延时
#define delay_dead() delay_200ns()
u16 Get_ADCRes(u8 ch);	//获取ADC指定通道采样结果
void Motor_Start(void); //电机启动函数
void Motor_Step(void); //电机换相函数

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

//调试用函数
void Uart_Config(void); //串口初始化
void UartSend(char dat); //串口发生字节
void UartSendStr(char *p); //串口发送字符串
void UartSendNum(int num); //串口发送-32767~32768范围内的数字

u8 step; //电机各相通电序号，六步换相，范围0~5
bit m_starting;	//电机正在启动标志
bit m_running;	//电机正在运行标志
u8 demagnetizing_cnt;	//消磁计数值，1为需要消磁，2为正在消磁，0为已经消磁，初始化为0，电机运行后比较器中断检测过零事件后置1（期间设置换相时间），在定时器4中断换相后进行消磁
bit t3_flag; //Timer3溢出标志，32.768ms置1一次
bit t0_flag; //Timer0溢出标志，4ms置1一次
#define TMP_LEN 8
u8 time_index;
u16 phase_time_tmp[TMP_LEN];
u8 timeout;
u8	PWM_Value;	// 决定PWM占空比的值
u8	PWM_Set;	//目标PWM设置
#define	D_START_PWM		30

//调试用参数
bit cmp_it_flag;
u16 debug_phase_time;
bit t4_it_flag1;
bit t4_it_flag2;

void UartIsr() interrupt 4
{
	if(TI)
	{
		TI = 0;
		busy = 0;
	}
	if(RI)
	{
		RI = 0;
		buffer[wptr++] = SBUF;
		if(SBUF == '\n')
			wptr |= 0x80;
	}
}

void UartRxTest(void)
{
	if(wptr & 0x80)
	{
		unsigned char len = wptr & 0x7f;
		u8 i;
		UartSendStr(buffer);
		for(i = 0;i<len;i++)
			buffer[i] = 0;
		wptr = 0;
	}
}

void main(void)
{
	u8	i;
	u16	j;
	
	P_SW2 |= 0x80; //使能XFR
	
	Port_Init();	//调用端口初始化函数
	PWMA_Config();	//调用PWMA初始化函数
	ADC_Config();	//调用ADC初始化函数
	CMP_Config();	//调用模拟比较器初始化函数
	Timer0_Config();	//调用定时器0初始化函数
	Timer3_Config();	//调用定时器3初始化函数
	Timer4_Config();	//调用定时器4初始化函数
	
	Uart_Config();	//调用串口初始化函数
	ES = 1;
	
	PWM_Set = 100;
	timeout = 0;
	
	EA = 1;	//打开总中断
	
	UartSendStr("--Brushless ESC Test--\r\n");
	
	//PWMA_ENO = 0x15; //0x15 = 0b0001 0101，测试PWMA功能，使能PWM1P，PWM2P，PWM3P输出
	
	while(1)
	{
		if(t0_flag)
		{
			t0_flag = 0;
			if(timeout != 0)
			{
				if(--timeout == 0)
				{
					UartSendStr("timeout\r\n");
					m_running = 0;
					PWM_Value = 0;
					CMPCR1 = 0x8c;
					PWMA_ENO = 0;
					PWMA_CCR1 = 0;
					PWMA_CCR2 = 0;
					PWMA_CCR3 = 0;
					PWM1_L = 0;
					PWM2_L = 0;
					PWM3_L = 0;
					Delay_n_ms(250);
				}
			}
			if(!m_running && (PWM_Set >= D_START_PWM))
			{
				UartSendStr("-ST-\r\n");
				m_starting = 1;
				for(i = 0;i<8;i++) phase_time_tmp[i] = 400; 
				Motor_Start();
				m_starting = 0;
				demagnetizing_cnt = 0;
				CMPCR1 &= ~0x40;
				if(step & 1)	CMPCR1 = 0xAC;		//上升沿中断
				else			CMPCR1 = 0x9C;		//下降沿中断
				m_running = 1;
				Delay_n_ms(250);	//延时一下, 先启动起来
				Delay_n_ms(250);
				timeout = 125;		//启动超时时间 125*4 = 500ms
			}
			if(m_running)
			{
				if(PWM_Value < PWM_Set) PWM_Value++;
				if(PWM_Value > PWM_Set) PWM_Value--;
				PWMA_CCR1 = PWM_Value;
				PWMA_CCR2 = PWM_Value;
				PWMA_CCR3 = PWM_Value;
			}
			if(cmp_it_flag)
			{
				cmp_it_flag = 0;
				UartSendStr("-cmp it-\r\n");
				UartSendStr("ph:");
				UartSendNum(debug_phase_time);
				UartSendStr("\r\n");
			}
			if(t4_it_flag1)
			{
				t4_it_flag1 = 0;
				UartSendStr("-t4 it 1-\r\n");
			}
			if(t4_it_flag2)
			{
				t4_it_flag2 = 0;
				UartSendStr("-t4 it 2-\r\n");
			}
		}
	}
}

void Port_Init(void)
{
	P0M0 = 0x00;
	P0M1 = 0x00; //P0端口初始化为双向口
	P1M0 = 0x00;
	P1M1 = 0x00; //P1端口初始化为双向口
	P2M0 = 0x00;
	P2M1 = 0x00; //P2端口初始化为双向口
	P3M0 = 0x00;
	P3M1 = 0x00; //P3端口初始化为双向口
	P5M0 = 0x00;
	P5M0 = 0x00; //P5端口初始化为双向口
}

void PWMA_Config(void)
{
	//先将MOS管选通信号拉低，防止误导通
	PWM1 = 0;
	PWM1_L = 0;
	PWM2 = 0;
	PWM2_L = 0;
	PWM3 = 0;
	PWM3_L = 0;
	
	//配置选通引脚为推挽输出，M0相应bit为1，M1相应bit为0
	P1M0 |= 0x3f; //0x3f = 0B0011 1111，bit0~bit5置1
	P1M1 &= ~0x3f; //~0x3f = 0B1100 0000,bit0~bit5置0
	
	PWMA_PSCR = 3;	//PWMA_PSCR为PWMA的16位预分频器寄存器，可以16位数据读写，f_ck_int = f_ck_psc/(PSCR[15:0]+1)
	PWMA_DTR = 24;	//PWMA_DTR位PWMA的死区寄存器，设置死区持续时间，本程序死区互补功能未开
	
	PWMA_ARR = 0xff;	//PWMA_ARR为PWMA的16位自动重载寄存器
	PWMA_CCER1 = 0;	//捕获/比较使能寄存器1，配置极性及输出使能
	PWMA_CCER2 = 0;	//捕获/比较使能寄存器2，配置极性及输出使能
	PWMA_SR1 = 0;		//状态寄存器1，中断标记
	PWMA_SR2 = 0;		//状态寄存器2，重复捕获标记
	PWMA_ENO = 0;		//输出使能寄存器
	PWMA_PS = 0;		//功能脚切换
	PWMA_IER = 0;		//中断使能寄存器
	
	PWMA_CCMR1 = 0x68;	//捕获/比较模式寄存器1，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR1 = 0x00;			//捕获/比较寄存器，16位，当前比较值
	PWMA_CCER1 |= 0x01;	//捕获/比较使能寄存器1，原0x05 = 0b0000 0101,现改为0x01，使能OC1输出
	PWMA_PS |= 0;				//PWMA IO选择
	
	PWMA_CCMR2 = 0x68;	//捕获/比较模式寄存器2，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR2 = 0x00;			//捕获/比较寄存器，16位，当前比较值，与PWMA_ARR比较
	PWMA_CCER1 |= 0x10;	//捕获/比较使能寄存器1，原0x50 = 0b0101 0000，现改为0x10，使能OC2输出
	PWMA_PS |= (0<<2);	//PWMA IO选择
	
	PWMA_CCMR3 = 0x68;	//捕获/比较模式寄存器3，0x68 = 0b0110 1000配置为输出，开启预装载，PWM模式1
	PWMA_CCR3 = 0x00;			//捕获/比较寄存器，16位，当前比较值
	PWMA_CCER2 |= 0x01;	//捕获/比较使能寄存器2，原0x05 = 0b0000 0101,现改为0x01，使能OC3输出
	PWMA_PS |= (0<<4);	//PWMA IO选择
	
	PWMA_BKR = 0x80;	//使能OC和OCN输出
	PWMA_CR1 = 0x81;	//控制寄存器1，使能自动预装载，边沿对齐，向上计数，使能计数器
	PWMA_EGR = 0x01;	//初始化计数器
}

void ADC_Config(void)
{
	//设置引脚模式为高阻输入，M1相应bit为1，M0相应bit为0
	P1M1 |= 0xc0; //标志位置1
	P1M0 &= ~0xc0; //标志位清0
	
	//设置引脚模式为高阻输入，M1相应bit为1，M0相应bit为0
	P0M1 |= 0x0f;  //标志位置1
	P0M0 &= ~0x0f; //标志位清0
	
	ADC_CONTR = 0x80+6; //ADC控制寄存器，bit7置1打开ADC电源，bit3~bit0为通道选择位，STC8H1K28系列无ADC12~ADC14.STC8H1K08系列无ADC2~ADC7
	ADCCFG = RES_FMT + ADC_SPEED; //ADC配置寄存器，bit5结果格式控制，置0左对齐，置1右对齐，bit3~bit0设置ADC时钟频率，f=Sysclk/2/(speed+1)
	ADCTIM = CSSETUP + CSHOLD + SMPDUTY;	//ADC时序控制寄存器，bit7：T_setup | bit6~bit5：T_hold | bit4~bit0：T_duty
}

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

void Timer0_Config(void)
{
	TMOD &= ~0x03;		//定时器0/1模式寄存器，~0x03 = 0b1111 1100，bit1~bit0清零，定时器0配置为16位自动重载模式
	AUXR &= ~(1<<7);	//辅助寄存器1，bit7置0，定时器0时钟为CPU时钟12分频，24MHz/12 = 2MHz
	
	//12分频下，定时器周期=(65536-[TH0,TL0])*12/FOSC,如4ms定时，这周期= 0.004 = 1/250
	TH0 = (65536UL - FOSC / 12 / 250)>>8;	//定时器0计数寄存器
	TL0 = (u8)(65536UL - FOSC / 12 / 250); //定时器0计数寄存器
	TR0 = 1;	//启动定时器0
	ET0 = 1;	//使能定时器0中断
}

void Timer3_Config(void)
{
	T4T3M &= 0xf0;	//定时器3/4控制寄存器，高4位定时器4，低4位定时器3。低4位清零，定时器3停止计数，用作定时器，12分频，关闭时钟输出
	T3H = 0;	//定时器3计数寄存器
	T3L = 0;	//定时器3计数寄存器
	
	T3T4PIN = 0x01;	//定时器3/4功能引脚切换寄存器，bit0选择位，置1选择P0.0，P0.1，P0.2，P0.3
	IE2 |= (1<<5);	//中断使能寄存器2，bit5为定时器3的溢出中断允许位，置1开中断
	T4T3M |= (1<<3);	//定时器3/4控制寄存器，bit3置1，定时器3开始计数
}

void Timer4_Config(void)
{
	T4T3M &= 0x0f; //定时器3/4控制寄存器，高4位定时器4，低4位定时器3。低4位清零，定时器4停止计数，用作定时器，12分频，关闭时钟输出
	T4H = 0;	//定时器4计数寄存器
	T4L = 0;	//定时器4计数寄存器
	
	//T4H = (u8)((65536UL - 40*2) >> 8);	//测试用，定时器4计数频率为2MHz，40us要用80次
	//T4L = (u8)(65536UL - 40*2);	//测试用
	
	T3T4PIN = 0x01;	//定时器3/4功能引脚切换寄存器，bit0选择位，置1选择P0.0，P0.1，P0.2，P0.3
	IE2 |= (1<<6);	//中断使能寄存器2，bit6为定时器4的溢出中断允许位，置1开中断
	//T4T3M |=  (1<<7);	//开始计数，测试用
}

u16 Get_ADCRes(u8 ch)
{
	u8 i = 255; //ADC转换超时计数，模糊计时
	ADC_RES = 0;	//ADC转换结果寄存器，清零
	ADC_RESL = 0; //ADC转换结果寄存器L，清零
	ADC_CONTR = 0x80|ADC_START|ch; //选择ADC通道并开始转换
	_nop_();
	while(i != 0)
	{
		i--;
		if((ADC_CONTR & ADC_FLAG) != 0)	break;	//等待ADC结束
	}
	ADC_CONTR &= ~ADC_FLAG;	//清除转换完成标志位
	return	((u16)ADC_RES>>8 + (u16)ADC_RESL); //返回读取结果
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

void Uart_Config(void)
{
	SCON = 0x50;
	T2L = BRT;
	T2H = BRT>>8;
	AUXR = 0x15;
	wptr = 0;
	busy = 0;
}

void UartSend(char dat)
{
	while(busy);
	busy = 1;
	SBUF = dat;
}

void UartSendStr(char *p)
{
	while(*p)
	{
		UartSend(*p++);
	}
}

void UartSendNum(int num)
{
	char buf[8];
	u8 dat;
	u8 i = 0;
	if(num<0)
	{
		UartSend('-');
		num = -num;
	}
	while(num)
	{
		dat = num%10;
		buf[i] = dat + 0x30;
		num /= 10;
		i++;
	}
	UartSendStr(buf);
}

//比较器中断函数
//检测到过零事件后，根据定时器3的计数值计算离上一次过零事件的时间
//求出8次换相所用时间除以16，计算每半次换相所需时间，即为离下次换相所需时间
void CMP_ISR(void) interrupt 21
{
	u16 phase_time;
	u8 i;
	
	CMPCR1 %= ~0x40; //清除中断标志位
	
	if(demagnetizing_cnt == 0) //消磁后检测过零事件，未消磁不处理比较器中断
	{
		T4T3M &= ~(1<<3); //Timer3停止计数
		if(t3_flag)
		{
			t3_flag = 0;
			phase_time = 0x1fff;
		}
		else
		{
			phase_time = (((u16)T3H<<8)+(u16)T3L)>>1;
			phase_time &= 0x1ffff;
		}
		T3H = 0;
		T3L = 0;
		T4T3M |= (1<<3);
		
		phase_time_tmp[time_index] = phase_time;
		if(++time_index >=8) time_index = 0;
		for(phase_time = 0,i = 0;i<8;i++)
		{
			phase_time += phase_time_tmp[i];
		}
		phase_time >>= 4;
		debug_phase_time = phase_time;
		if(phase_time > 40 && phase_time < 1000)
		{
			cmp_it_flag = 1;
			timeout = 125;
		}
		if(phase_time > 40) phase_time -= 40;
		else phase_time = 20;
		
		T4T3M &= ~(1<<7);
		phase_time = phase_time << 1;
		phase_time = 0 - phase_time;
		T4H = (u8)(phase_time >> 8);
		T4L = (u8)phase_time;
		T4T3M |= (1<<7);
		demagnetizing_cnt = 1;
	}
}

void Timer0_ISR(void) interrupt 1
{
	//PWM1 = !PWM1; //定时器0中断测试，翻转PWM1引脚电平
	t0_flag = 1;
}

void Timer3_ISR(void) interrupt 19
{
	//PWM2 = !PWM2; //定时器3中分段测试，翻转PWM2引脚电平
	t3_flag = 1;
}

//电机启动后利用定时器4来换相和消磁，消磁原理简化成短延时，此处设为40us延时，电流越大，消磁时间越长
//电机换相后，断电的那一相，由于线圈的存在会产生自感电动势，若不消磁，会引起比较器在非感应电动势过零时刻中断
//此处根据修改demagnetzing_cnt消磁计数值来跳过因消磁引起的比较器中断
void Timer4_ISR(void) interrupt 20
{
	T4T3M &= ~(1<<7); //定时器3/4控制寄存器bit7置0，定时器4停止计数
	
	if(demagnetizing_cnt == 1)	//判断消磁计数值，为1为需要消磁
	{
		demagnetizing_cnt = 2;	//将消磁计数值改为2，先换相，再进行消磁
		if(m_running)
		{
			t4_it_flag1 = 1;
			if(++step >= 6) step = 0;
			Motor_Step();
		}
		
		T4H = (u8)((65536UL - 40*2) >> 8);	//40us计数，(65536 - t *2)，t单位us，高8位放入T4H，低8位放入T4L
		T4L = (u8)(65536UL - 40*2);	//40us计数
		T4T3M |=  (1<<7);	//Timer4开始计数
	}
	else if(demagnetizing_cnt == 2)
	{
		t4_it_flag2 = 1;
		demagnetizing_cnt = 0;
	}
}
