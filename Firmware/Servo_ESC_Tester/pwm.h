#ifndef __PWM_H
#define __PWM_H

void PwmSetHighUs(unsigned int ch0_us,unsigned int ch1_us,unsigned int ch2_us)
{
	unsigned int count = 256 - ch0_us / (20000>>8);
	CCAP0L = count;	//PCA模块模式捕获值/比较值寄存器低8位
	CCAP0H = count;	//PCA模块模式捕获值/比较值寄存器高8位
	count = 256 - ch1_us / (20000>>8);
	CCAP1L = count;	//PCA模块模式捕获值/比较值寄存器低8位
	CCAP1H = count;	//PCA模块模式捕获值/比较值寄存器高8位
	count = 256 - ch2_us / (20000>>8);
	CCAP2L = count;	//PCA模块模式捕获值/比较值寄存器低8位
	CCAP2H = count;	//PCA模块模式捕获值/比较值寄存器高8位
}

void PwmInit()
{
	CCON = 0x00;	//PCA控制器寄存器，清除溢出中断标志，停止PCA技术，清除中断标志
	CMOD = 0x04;	//PCA模式寄存器，PCA时钟源为系统时钟
	CL = 0x00;		//PCA计数器寄存器，低8位
	CH = 0x00;		//PCA计数器寄存器，高8位
	
	CCAPM0 = 0x42;	//PCA模块模式控制寄存器，开启比较，开启PWM
	PCA_PWM0 = 0x00;	//PCA模块PWM模式控制寄存器，0x80为6位PWM
	
	CCAPM1 = 0x42;
	PCA_PWM1 = 0x00;	//0x40为7位PWM
	
	CCAPM2 = 0x42;
	PCA_PWM2 = 0x00;	//0xc0为10位PWM
	
	PwmSetHighUs(500,500,500);
	
	CR = 1;	//启动PCA计时器
}

#endif
