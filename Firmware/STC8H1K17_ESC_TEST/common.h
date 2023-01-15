#ifndef __COMMON_H
#define __COMMON_H

u8 step; //电机各相通电序号，六步换相，范围0~5
bit m_starting;	//电机正在启动标志
bit m_running;	//电机正在运行标志
u8 demagnetizing_cnt;	//消磁计数值，1为需要消磁，2为正在消磁，0为已经消磁，初始化为0，电机运行后比较器中断检测过零事件后置1（期间设置换相时间），在定时器4中断换相后进行消磁

u8 timeout;
u8	PWM_Value;	// 决定PWM占空比的值
u8	PWM_Set;	//目标PWM设置
#define	D_START_PWM 40

#endif