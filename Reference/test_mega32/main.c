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


uint8_t  MotorAddress = MOTOR_ADDRESS;	//本电调地址


uint8_t  PWM = 0;			//当前PWM值
uint8_t  MaxPWM = 255;		//最大PWM值
uint8_t  PwmSet = 0;		//SIO/I2C/PPM收到PWM设置信号
uint8_t  PwmSetValue = 0;	//SIO/I2C/PPM收到PWM设置的值


uint16_t Strom = 0;			//当前电流
uint8_t  Strom_max = 0;		//最大电流
uint8_t  MiddleStrom = 0;	//平均电流
uint16_t LeakStrom = 0;		//场效应管全关时的漏电流


volatile uint8_t Phase = 0;		//当前相位
uint8_t  MotorStopping = 1;		//电机静止状态标志
uint8_t  MotorStartCmd = 0;		//电机启动命令标志
uint8_t  StromConvertCmd = 1;	//电流测量命令标志
uint16_t CntCommutation = 0;	//换相累积次数
uint16_t MotorRunning;			//当前电机是否正在转动标志
uint16_t MotorRpm = 0;			//当前转速
uint8_t  InterT2Cnt = 0;		//每6次换相的定时器T2溢出次数(单位为64us)


uint8_t  EstimateTime = 1;		//运行时普通延时计数	
uint16_t MinRpmPulse = 0;		//最小转速定时器




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
	//延时100ms,若收到命令则退出延时状态
	while(!CheckDelay(MinRpmPulse)) {
		if(GetCommandPWMValue()) break;
	}

	//LEDGRN_ON;
	PWM = 0;

	ACMP_MULTI_ENABLE	//模拟比较器的“多路复用器”使能
	ADMUX = 1;

	MinRpmPulse = SetDelay(10);

	//电机开机测试(发出音调)
	//if(!GetCommandPWMValue()) MotorTest();
	
	//启动测试
	//MotorStartUp(10);	
	ALLPIN_AND_PWM_OFF;
	PWM = 0;
	SetPWM(PWM);

	
//-----------------------主循环-------------------------
	while(1) {

		//1. 获取USART/I2C/PPM的命令PWM值
		PWM = GetCommandPWMValue();

		//2. 当前相位和刚才相位不同，电机一定为运转状态
		if(Phase != lastPhase) {
			MotorStopping = 0;
			EstimateTime = 0;
			MinRpmPulse = SetDelay(250);
			lastPhase = Phase;
		}

		//3. 根据命令的PWM值决定开机还是停机(若在运转中收到PWM=0的命令,起码1.5s后才停机)
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

		//4. 再次确认PWM值正确并设置
		if(MotorStopping) PWM = 0;
		SetPWM(PWM);

		//5. 定时抛出转速/电流大小到串口(每500ms)
		if(CheckDelay(SendRpmTimer)) {
			SendRpmTimer = SetDelay(500);
			TxBuffer[0] = MOTOR_ADDRESS; TxBuffer[1] = PWM;
			TxBuffer[2] = MotorRpm / 0x100; TxBuffer[3] = MotorRpm % 0x100;
			TxBuffer[4] = MiddleStrom;
			uart_sendbuffer(7);
		}

		//++++++++++++++++++++++定期例程EstimateTimer+++++++++++++++++++++++++++++++++
		if(!EstimateTime++) {

			//(1)检查电机是否在停机状态
			if(MotorStopping) {
				//LEDGRN_ON;
				FastStromConvert();
			}	

			//(2)定时检查电流大小(每50ms)
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

			//(3)定时检查转速和换相情况(每10ms)
			if(CheckDelay(SpeedMeassureTimer)) {
				uint32_t cnt;
				
				SpeedMeassureTimer = SetDelay(10);
				DISABLE_SENSE_INT;	//关比较器中断
					MotorRunning = CntCommutation;
					CntCommutation = 0;
					cnt = InterT2Cnt;
				ENABLE_SENSE_INT;	//开比较器中断 				
				StromConvertCmd = 1;
				//计算转速
				if(MotorRunning)	MotorRpm = 60000000 / (cnt * 64 * 7);
				else 				MotorRpm = 0;
			}
	
			//(4)若马达已停转一段时间,看MotorStartCmd标志觉得是否要重新启动电机
			if((CheckDelay(MinRpmPulse) && MotorRunning == 0) || MotorStartCmd) {
				MotorStopping = 1;
				DISABLE_SENSE_INT;
				MinRpmPulse = SetDelay(100);
				
				//.若是电机启动命令
				if(MotorStartCmd) {
					LEDRED_OFF;
					Strom_max = 0;
					MotorStartCmd = 0;
					
					//..启动电机,若返回1说明电机已转起来
					if(MotorStartUp(10)) {
						LEDGRN_ON;
						MotorStopping = 0;
						Phase--;		//启动返回后Phase=2
						PWM = 1;		
						SetPWM(PWM);	//设置PWM为1让电机进入自由惯性旋转状态
						
						SENSE_TOGGLE_INT;	//使比较器接管换相工作
						ENABLE_SENSE_INT;	//???
						
						//...延时20毫秒
						MinRpmPulse = SetDelay(20);
						while(!CheckDelay(MinRpmPulse));

						//...以PWM为15运行300ms,目前换相工作已由比较器中断接管；
						//...若期间电流太大说明由于某种原因被卡死，下次循环再尝试重新启动
						PWM = 15;
						SetPWM(PWM);
						MinRpmPulse = (300);
						while(!CheckDelay(MinRpmPulse)) {
							if(Strom > LIMIT_STROM / 2) {
								ALLPIN_AND_PWM_OFF;	//急停
								LedRedBlink(8);    //红LED闪8次
								MotorStartCmd = 1;
							}
						}

						//...50ms后再测量转速(而非常规的10ms)
						SpeedMeassureTimer = SetDelay(50);
						lastPhase = 7;
						

					//..否则说明电机启动不成功,下次循环再重新尝试
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

