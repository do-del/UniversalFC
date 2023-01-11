#include <avr/io.h>
#include <avr/interrupt.h>

#include "port.h"
#include "timer0.h"
#include "analog.h"
#include "pwm.h"
#include "uart.h"
#include "acmp.h"
#include "BLMC.h"
#include "twislave.h"


uint8_t  MotorAddress = MOTOR_ADDRESS;	//�������ַ


uint8_t  PWM = 0;			//��ǰPWMֵ
uint8_t  MaxPWM = 255;		//���PWMֵ
uint8_t  PwmSet = 0;		//SIO/I2C/PPM�յ�PWM�����ź�
uint8_t  PwmSetValue = 0;	//SIO/I2C/PPM�յ�PWM���õ�ֵ


uint16_t Strom = 0;			//��ǰ����
uint8_t  Strom_max = 0;		//������
uint8_t  MiddleStrom = 0;	//ƽ������
uint16_t LeakStrom = 0;		//��ЧӦ��ȫ��ʱ��©����


volatile uint8_t Phase = 0;		//��ǰ��λ
uint8_t  MotorStopping = 1;		//�����ֹ״̬��־
uint8_t  MotorStartCmd = 0;		//������������־
uint8_t  StromConvertCmd = 1;	//�������������־
uint16_t CntCommutation = 0;	//�����ۻ�����
uint16_t MotorRunning;			//��ǰ����Ƿ�����ת����־
uint16_t MotorRpm = 0;			//��ǰת��
uint8_t  InterT2Cnt = 0;		//ÿ6�λ���Ķ�ʱ��T2�������(��λΪ64us)


uint8_t  EstimateTime = 1;		//����ʱ��ͨ��ʱ����	
uint16_t MinRpmPulse = 0;		//��Сת�ٶ�ʱ��




extern uint8_t TxBuffer[MAX_SEND_SIZE];
extern uint8_t RxBuffer[MAX_RECV_SIZE];
extern uint8_t twi_rx_buffer[TWI_MAX_RECV_SIZE];
extern volatile uint16_t SIO_Timeout;
extern volatile uint16_t I2C_Timeout;


uint8_t GetCommandPWMValue() {
	static uint8_t pwmval = 0;
	
	if(PwmSet) {
		PwmSet = 0;
		pwmval = PwmSetValue;
	}
	
	return pwmval;
}

int main() {
	//volatile uint16_t tmp = 0;
	uint8_t lastPhase = 0;
	uint16_t MiddleStromTimer = 0, SpeedMeassureTimer = 0, MotorStopTimer = 0;
	uint16_t SendRpmTimer = 0, ADResTimer = 0;

	Port_Init();
	LEDRED_OFF;	LEDGRN_OFF;

	Uart_Init();
	MCUCSR &= ~0x1F;

	Timer_Init();
	sei();

	PWM_Init();
	
	InitI2C_Slave(0x50);

	//InitPPM();

	volatile uint16_t a = 1, b = 0xFFFE, c = 0;
	c = a - b;

	MinRpmPulse = SetDelay(103);
	MiddleStromTimer = SetDelay(254);
	SpeedMeassureTimer = SetDelay(1005);
	SendRpmTimer = SetDelay(1010);
	ADResTimer   = SetDelay(1050);
	//��ʱ100ms,���յ��������˳���ʱ״̬
	while(!CheckDelay(MinRpmPulse)) {
		if(GetCommandPWMValue()) break;
	}

	//LEDGRN_ON;
	PWM = 0;

	ACMP_MULTI_ENABLE	//ģ��Ƚ����ġ���·��������ʹ��
	ADMUX = 1;

	MinRpmPulse = SetDelay(10);

	//�����������(��������)
	//if(!GetCommandPWMValue()) MotorTest();
	
	//��������
	//MotorStartUp(10);	
	ALLPIN_AND_PWM_OFF;
	PWM = 0;
	SetPWM(PWM);

	
//-----------------------��ѭ��-------------------------
	while(1) {

		//1. ��ȡUSART/I2C/PPM������PWMֵ
		PWM = GetCommandPWMValue();

		//2. ��ǰ��λ�͸ղ���λ��ͬ�����һ��Ϊ��ת״̬
		if(Phase != lastPhase) {
			MotorStopping = 0;
			EstimateTime = 0;
			MinRpmPulse = SetDelay(250);
			lastPhase = Phase;
		}

		//3. ���������PWMֵ������������ͣ��(������ת���յ�PWM=0������,����1.5s���ͣ��)
		if(!PWM) {
			MotorStartCmd = 0;
			EstimateTime = 0;
			if(CheckDelay(MotorStopTimer)) {
				DISABLE_SENSE_INT;
				MotorStopping = 1;
				ALLPIN_AND_PWM_OFF;
			}
		} else {
			if(MotorStopping) MotorStartCmd = 1;
			MotorStopTimer = SetDelay(1500);
		}

		//4. �ٴ�ȷ��PWMֵ��ȷ������
		if(MotorStopping) PWM = 0;
		SetPWM(PWM);

		//5. ��ʱ�׳�ת��/������С������(ÿ500ms)
		if(CheckDelay(SendRpmTimer)) {
			SendRpmTimer = SetDelay(500);
			TxBuffer[0] = MOTOR_ADDRESS; TxBuffer[1] = PWM;
			TxBuffer[2] = MotorRpm / 0x100; TxBuffer[3] = MotorRpm % 0x100;
			TxBuffer[4] = MiddleStrom;
			uart_sendbuffer(7);
		}

		//++++++++++++++++++++++��������EstimateTimer+++++++++++++++++++++++++++++++++
		if(!EstimateTime++) {

			//(1)������Ƿ���ͣ��״̬
			if(MotorStopping) {
				//LEDGRN_ON;
				FastStromConvert();
			}	

			//(2)��ʱ��������С(ÿ50ms)
			if(CheckDelay(MiddleStromTimer)) {
				MiddleStromTimer = SetDelay(50);
				
				if(MiddleStrom < Strom) MiddleStrom++;
				else if(MiddleStrom > Strom) MiddleStrom--;
				
				if(Strom > MAX_STROM) MaxPWM -= MaxPWM / 32;

				if(MiddleStrom > LIMIT_STROM) {
					if(MaxPWM) MaxPWM--;
					LEDRED_ON;
				} else {
					if(MaxPWM < MAX_PWM)  MaxPWM++;
				}
			}

			//(3)��ʱ���ת�ٺͻ������(ÿ10ms)
			if(CheckDelay(SpeedMeassureTimer)) {
				uint32_t cnt;
				
				SpeedMeassureTimer = SetDelay(10);
				DISABLE_SENSE_INT;	//�رȽ����ж�
					MotorRunning = CntCommutation;
					CntCommutation = 0;
					cnt = InterT2Cnt;
				ENABLE_SENSE_INT;	//���Ƚ����ж� 				
				StromConvertCmd = 1;
				//����ת��
				if(MotorRunning)	MotorRpm = 60000000 / (cnt * 64 * 7);
				else 				MotorRpm = 0;
			}
	
			//(4)�������ͣתһ��ʱ��,��MotorStartCmd��־�����Ƿ�Ҫ�����������
			if((CheckDelay(MinRpmPulse) && MotorRunning == 0) || MotorStartCmd) {
				MotorStopping = 1;
				DISABLE_SENSE_INT;
				MinRpmPulse = SetDelay(100);
				
				//.���ǵ����������
				if(MotorStartCmd) {
					LEDRED_OFF;
					Strom_max = 0;
					MotorStartCmd = 0;
					
					//..�������,������1˵�������ת����
					if(MotorStartUp(10)) {
						LEDGRN_ON;
						MotorStopping = 0;
						Phase--;		//�������غ�Phase=2
						PWM = 1;		
						SetPWM(PWM);	//����PWMΪ1�õ���������ɹ�����ת״̬
						
						SENSE_TOGGLE_INT;	//ʹ�Ƚ����ӹܻ��๤��
						ENABLE_SENSE_INT;	//???
						
						//...��ʱ20����
						MinRpmPulse = SetDelay(20);
						while(!CheckDelay(MinRpmPulse));

						//...��PWMΪ15����300ms,Ŀǰ���๤�����ɱȽ����жϽӹܣ�
						//...���ڼ����̫��˵������ĳ��ԭ�򱻿������´�ѭ���ٳ�����������
						PWM = 15;
						SetPWM(PWM);
						MinRpmPulse = (300);
						while(!CheckDelay(MinRpmPulse)) {
							if(Strom > LIMIT_STROM / 2) {
								ALLPIN_AND_PWM_OFF;	//��ͣ
								LedRedBlink(8);    //��LED��8��
								MotorStartCmd = 1;
							}
						}

						//...50ms���ٲ���ת��(���ǳ����10ms)
						SpeedMeassureTimer = SetDelay(50);
						lastPhase = 7;
						

					//..����˵������������ɹ�,�´�ѭ�������³���
					} else { 
						if(GetCommandPWMValue()) MotorStartCmd = 1;
					}					

				}//end if(MotorStart)

			}//end if((CheckDelay(MinRpmPUlse)...

		} //end if(!EstimateTime++)...


		if(SIO_Timeout == 1) {
			SIO_Timeout = 0;
			RxComdAction(); 
		}

		if(I2C_Timeout == 1) {
			//uint8_t i;
			//for(i = 0; i < 2; i++) RxBuffer[i] = twi_rx_buffer[i];
			I2C_Timeout = 0;
			I2CRxComdAction();
		}
	} //end while(1)	
	return 0;
}

