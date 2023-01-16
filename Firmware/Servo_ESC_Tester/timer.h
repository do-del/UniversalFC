/*
void Timer0_Set_Period(unsigned int _us)
{
	unsigned int count = 65536 - (unsigned int)(FOSC / 12 /1000000 *_us);
	TR0 = 0;
	TL0 = count;
	TH0 = count>>8;
	TR0 = 1;
}
*/

void Timer0Init()
{
	unsigned int count = 65536- FOSC / 12 / 12800;
	TMOD = 0x00;	//��ʱ��ģʽ�Ĵ�����ģʽ0
	TL0 = count;
	TH0 = count>>8;
	TR0 = 1;			//��ʼ����
}
