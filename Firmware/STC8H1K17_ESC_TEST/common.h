#ifndef __COMMON_H
#define __COMMON_H

u8 step; //�������ͨ����ţ��������࣬��Χ0~5
bit m_starting;	//�������������־
bit m_running;	//����������б�־
u8 demagnetizing_cnt;	//���ż���ֵ��1Ϊ��Ҫ���ţ�2Ϊ�������ţ�0Ϊ�Ѿ����ţ���ʼ��Ϊ0��������к�Ƚ����жϼ������¼�����1���ڼ����û���ʱ�䣩���ڶ�ʱ��4�жϻ�����������

u8 timeout;
u8	PWM_Value;	// ����PWMռ�ձȵ�ֵ
u8	PWM_Set;	//Ŀ��PWM����
#define	D_START_PWM 40

#endif