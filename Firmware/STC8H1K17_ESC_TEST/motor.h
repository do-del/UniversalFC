#ifndef __MOTOR_H
#define __MOTOR_H

#define delay_200ns() do{_nop_();_nop_();_nop_();_nop_();}while(0) //����MOS���ֲ��������ʱ�䣬�ֲ��õ�MOS�ܵ�ͨ�ض�ʱ�����Ϊ55ns���˴�����Ϊ200ns��ʱ
#define delay_dead() delay_200ns()

void	Delay_n_ms(u8 dly)	// N ms��ʱ����
{
	u16	j;
	do
	{
		j = FOSC / 10000;
		while(--j)	;
	}while(--dly);
}

void delay_us(u8 us)	//N us��ʱ����
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

//���ຯ��
//���ݵ��ͨ����ţ�������ͨ��
void Motor_Step(void)
{
	switch(step)
	{
		case 0: //AB��ͨ�磬�ڼ�C���Ӧ�綯���ɸ������仯��A�Ϲܵ�ͨ��B�¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x00; //�ر�����PWM������ر�����һ��C���Ϲܵ�ͨ�����ȹضϣ�Ȼ����A���Ϲ�
			PWM1_L = 0; //A���¹ܹض�
			PWM3_L = 0; //C���¹ܹض�
			delay_dead(); //�ӳ٣���ֹAC���Ϲ�ͬʱ��ͨ
			PWMA_ENO = 0x01; //��A���Ϲ�PWM
			PWM2_L = 1;	//B���¹ܵ�ͨ
			ADC_CONTR = 0x80+10; //ѡ��ADC10����P0.2������
			if(m_running) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		case 1:	//AC��ͨ�磬�ڼ�B���Ӧ�綯�����������仯��A���Ϲܵ�ͨ��C���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x01;	//A���Ϲܵ�ͨ
			PWM1_L = 0;	//A���¹ܹض�
			PWM2_L = 0; //B���¹ܹض�
			delay_dead(); //�ӳ٣���ֹBC���¹�ͬʱ��ͨ
			PWM3_L = 1;	//C���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 9;	//ѡ��ADC9��P0.1������
			if(m_running) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
			else CMPCR1 = 0x8c;	//�������ʱ�رձȽ����ж�
			break;
		case 2:	//BC��ͨ�磬�ڼ�A���Ӧ�綯���ɸ������仯��B���Ϲܵ�ͨ��C���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x00; //�ر�����PWM������ر�����һ��A���Ϲܵ�ͨ�����ȹضϣ�Ȼ����ܿ���B���Ϲ�
			PWM1_L = 0;	//A���¹ܹض�
			PWM2_L = 0; //B���¹ܹض�
			delay_dead();
			PWMA_ENO = 0x04;	//��B���Ϲ�PWM
			PWM3_L = 1;	//C���¹ܵ�ͨ
			ADC_CONTR = 0x80+8;		//ѡ��ADC8��P0.0������
			if(m_running) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		case 3: //BA��ͨ�磬�ڼ�C���Ӧ�綯�����������仯��B���Ϲܵ�ͨ��A���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x04;	//��B���Ϲܣ��ر�A��C���Ϲ�
			PWM2_L = 0;	//B���¹ܹض�
			PWM3_L = 0; //C���¹ܹض�
			delay_dead();	//������ʱ����AC���¹�����
			PWM1_L = 1;	//A���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 10;	//ѡ��ADC10��P0.2������
			if(m_running) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
			else CMPCR1 = 0x8c;	//�������ʱ�رձȽ����ж�
			break;
		case 4: //CA��ͨ�磬�ڼ�B���Ӧ�綯���ɸ������仯��C���Ϲܵ�ͨ��A���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x00;	//�ر������Ϲ�PWM����ֹBC���Ϲ�ͬʱ��ͨ
			PWM2_L = 0;	//B���¹ܹض�
			PWM3_L = 0;	//C���¹ܹض�
			delay_dead();	//������ʱ����BC���Ϲ�����
			PWMA_ENO = 0x10;	//ʹ��C���Ϲ�PWM
			PWM1_L = 1;	//A���¹ܵ�ͨ
			ADC_CONTR = 0x80+9;	//ADC9��P0.1������
			if(m_running) CMPCR1 = 0x8c + 0x10; //�Ƚ����½����ж�ʹ��
			else CMPCR1 = 0x8c;	//�������ʱ�رձȽ����ж�
			break;
		case 5:	//CB��ͨ�磬�ڼ�A���Ӧ�綯�����������仯��C���Ϲܵ�ͨ��B���¹ܵ�ͨ������MOS�ܹض�
			PWMA_ENO = 0x10;	//ʹ��C���Ϲ�PWM
			PWM1_L = 0;	//A���¹ܹض�
			PWM3_L = 0;	//C���¹ܹض�
			delay_dead();	//������ʱ����AB���¹�����
			PWM2_L = 1;	//B���¹ܵ�ͨ
			ADC_CONTR = 0x80 + 8;	//ADC8��P0.0������
			if(m_running) CMPCR1 = 0x8c + 0x20; //�Ƚ����������ж�ʹ��
			else CMPCR1 = 0x8c; //�������ʱ�رձȽ����ж�
			break;
		default:
			break;
	}
}

void Motor_Start(void)
{
	u16 timer,i;
	CMPCR1 = 0x8C;	// �رȽ����ж�

	PWM_Value  = D_START_PWM;	// ��ʼռ�ձ�, ���ݵ����������
	PWMA_CCR1L = PWM_Value;
	PWMA_CCR2L = PWM_Value;
	PWMA_CCR3L = PWM_Value;
	step = 0;
	Motor_Step();
	Delay_n_ms(50);
	//Delay_n_ms(250);// ��ʼλ��
	timer = 200;	//���ȵ������

	while(1)
	{
		for(i=0; i<timer; i++)	delay_us(100);  //���ݵ����������, ���ת�ٵȵȵ������������ٶ�
		timer -= timer /16;
		if(++step >= 6)	step = 0;
		Motor_Step();
		if(timer < 40)	return;
	}
}

#endif
