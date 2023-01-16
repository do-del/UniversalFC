#ifndef __PWM_H
#define __PWM_H

void PwmSetHighUs(unsigned int ch0_us,unsigned int ch1_us,unsigned int ch2_us)
{
	unsigned int count = 256 - ch0_us / (20000>>8);
	CCAP0L = count;	//PCAģ��ģʽ����ֵ/�Ƚ�ֵ�Ĵ�����8λ
	CCAP0H = count;	//PCAģ��ģʽ����ֵ/�Ƚ�ֵ�Ĵ�����8λ
	count = 256 - ch1_us / (20000>>8);
	CCAP1L = count;	//PCAģ��ģʽ����ֵ/�Ƚ�ֵ�Ĵ�����8λ
	CCAP1H = count;	//PCAģ��ģʽ����ֵ/�Ƚ�ֵ�Ĵ�����8λ
	count = 256 - ch2_us / (20000>>8);
	CCAP2L = count;	//PCAģ��ģʽ����ֵ/�Ƚ�ֵ�Ĵ�����8λ
	CCAP2H = count;	//PCAģ��ģʽ����ֵ/�Ƚ�ֵ�Ĵ�����8λ
}

void PwmInit()
{
	CCON = 0x00;	//PCA�������Ĵ������������жϱ�־��ֹͣPCA����������жϱ�־
	CMOD = 0x04;	//PCAģʽ�Ĵ�����PCAʱ��ԴΪϵͳʱ��
	CL = 0x00;		//PCA�������Ĵ�������8λ
	CH = 0x00;		//PCA�������Ĵ�������8λ
	
	CCAPM0 = 0x42;	//PCAģ��ģʽ���ƼĴ����������Ƚϣ�����PWM
	PCA_PWM0 = 0x00;	//PCAģ��PWMģʽ���ƼĴ�����0x80Ϊ6λPWM
	
	CCAPM1 = 0x42;
	PCA_PWM1 = 0x00;	//0x40Ϊ7λPWM
	
	CCAPM2 = 0x42;
	PCA_PWM2 = 0x00;	//0xc0Ϊ10λPWM
	
	PwmSetHighUs(500,500,500);
	
	CR = 1;	//����PCA��ʱ��
}

#endif
