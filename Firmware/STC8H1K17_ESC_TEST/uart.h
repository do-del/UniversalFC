#ifndef __UART_H
#define __UART_H

/*
#define BRT (65536-(FOSC/115200+2)/4) //������ת����ֵ
#define RX_LEN 16 //���ڽ������ݻ��泤��

bit busy;		//���ڷ�����ɱ�־
char wptr;	//���ڽ������ݳ���
char buffer[RX_LEN]; //���ڽ������ݻ���

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