#ifndef __SYS_H
#define __SYS_H

#define FOSC 11059200UL

void Delay_n_ms(unsigned int dly) // N ms��ʱ����
{
	unsigned int j;
	do
	{
		j = FOSC / 10000;
		while(--j)	;
	}while(--dly);
}

#endif
