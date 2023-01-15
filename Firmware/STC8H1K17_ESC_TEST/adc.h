#ifndef __ADC_H
#define __ADC_H

#define ADC_START (1<<6)	//AD�ӿ��ƼĴ���bit6��ADCת����������λ��д1��ʼADCת��
#define ADC_FLAG (1<<5)	//ADC���ƼĴ���bit5��ADCת��������־λ
#define ADC_SPEED 1			//ADC���üĴ���bit3~bit0������ADC����ʱ��Ƶ��
#define RES_FMT (1<<5)	//ADC���üĴ���bit5����1����ADC����Ҷ��룬��0����ADC��������
#define CSSETUP (0<<7)	//ADCTIMʱ����ƼĴ���ͨ��ѡ��ʱ�����λbit7��ռ��һ��ʱ����
#define CSHOLD (1<<5)		//ADCTIMʱ����ƼĴ���ͨ������ʱ�����λbit6~bit5��ռ��2��ʱ����
#define SMPDUTY 20			//ADCTIMʱ����ƼĴ���ͨ������ʱ�����λbit4~bit0��ռ��21��ʱ����������С��10

void ADC_Config(void)
{
	//��������ģʽΪ�������룬M1��ӦbitΪ1��M0��ӦbitΪ0
	P1M1 |= 0xc0; //��־λ��1
	P1M0 &= ~0xc0; //��־λ��0
	
	//��������ģʽΪ�������룬M1��ӦbitΪ1��M0��ӦbitΪ0
	P0M1 |= 0x0f;  //��־λ��1
	P0M0 &= ~0x0f; //��־λ��0
	
	ADC_CONTR = 0x80+6; //ADC���ƼĴ�����bit7��1��ADC��Դ��bit3~bit0Ϊͨ��ѡ��λ��STC8H1K28ϵ����ADC12~ADC14.STC8H1K08ϵ����ADC2~ADC7
	ADCCFG = RES_FMT + ADC_SPEED; //ADC���üĴ�����bit5�����ʽ���ƣ���0����룬��1�Ҷ��룬bit3~bit0����ADCʱ��Ƶ�ʣ�f=Sysclk/2/(speed+1)
	ADCTIM = CSSETUP + CSHOLD + SMPDUTY;	//ADCʱ����ƼĴ�����bit7��T_setup | bit6~bit5��T_hold | bit4~bit0��T_duty
}

/*
u16 Get_ADCRes(u8 ch)
{
	u8 i = 255; //ADCת����ʱ������ģ����ʱ
	ADC_RES = 0;	//ADCת������Ĵ���������
	ADC_RESL = 0; //ADCת������Ĵ���L������
	ADC_CONTR = 0x80|ADC_START|ch; //ѡ��ADCͨ������ʼת��
	_nop_();
	while(i != 0)
	{
		i--;
		if((ADC_CONTR & ADC_FLAG) != 0)	break;	//�ȴ�ADC����
	}
	ADC_CONTR &= ~ADC_FLAG;	//���ת����ɱ�־λ
	return	((u16)ADC_RES>>8 + (u16)ADC_RESL); //���ض�ȡ���
}
*/

#endif