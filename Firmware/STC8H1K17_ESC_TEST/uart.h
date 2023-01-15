#ifndef __UART_H
#define __UART_H

#define BRT (65536-(FOSC/115200+2)/4) //波特率转计数值
#define RX_LEN 16 //串口接收数据缓存长度

bit busy;		//串口发送完成标志
char wptr;	//串口接收数据长度
char buffer[RX_LEN]; //串口接收数据缓存

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
/*
void UartSendNum(u32 num)
{
	char buf[20];
	u8 dat;
	u8 i = 0;
	
	if(num<0)
	{
		UartSend('-');
		num = -num;
	}
	
	if(num == 0)
	{
		UartSend('0');
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
*/
/*
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
*/

#endif