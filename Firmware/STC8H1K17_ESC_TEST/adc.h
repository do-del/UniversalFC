#ifndef __ADC_H
#define __ADC_H

#define ADC_START (1<<6)	//AD从控制寄存器bit6，ADC转换启动控制位，写1开始ADC转换
#define ADC_FLAG (1<<5)	//ADC控制寄存器bit5，ADC转换结束标志位
#define ADC_SPEED 1			//ADC配置寄存器bit3~bit0，设置ADC工作时钟频率
#define RES_FMT (1<<5)	//ADC配置寄存器bit5，置1配置ADC结果右对齐，置0配置ADC结果左对齐
#define CSSETUP (0<<7)	//ADCTIM时序控制寄存器通道选择时间控制位bit7，占用一个时钟数
#define CSHOLD (1<<5)		//ADCTIM时序控制寄存器通道保持时间控制位bit6~bit5，占用2个时钟数
#define SMPDUTY 20			//ADCTIM时序控制寄存器通道采样时间控制位bit4~bit0，占用21个时钟数，不得小于10

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

/*
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
*/

#endif