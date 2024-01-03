/**
  ******************************************************************************
  * @file    bsp_basic_tim.c
  * @author  STMicroelectronics
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   基本定时器定时范例
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_basic_tim.h"

/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"

static		uint16_t t1s;
static 		uint8_t  ucTime_1s_flag = 0;
static 		__IO uint32_t TimingDelay;


 /**
  * @brief  基本定时器 TIMx,x[6,7]中断优先级配置
  * @param  无
  * @retval 无
  */
//static void TIMx_NVIC_Configuration(void)
//{
//    NVIC_InitTypeDef NVIC_InitStructure; 
//    // 设置中断组为0
//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);		
//		// 设置中断来源
//    NVIC_InitStructure.NVIC_IRQChannel = BASIC_TIM_IRQn; 	
//		// 设置抢占优先级
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	 
//	  // 设置子优先级
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//}

/*
 * 注意：TIM_TimeBaseInitTypeDef结构体里面有5个成员，TIM6和TIM7的寄存器里面只有
 * TIM_Prescaler和TIM_Period，所以使用TIM6和TIM7的时候只需初始化这两个成员即可，
 * 另外三个成员是通用定时器和高级定时器才有.
 *-----------------------------------------------------------------------------
 * TIM_Prescaler         都有
 * TIM_CounterMode			 TIMx,x[6,7]没有，其他都有（基本定时器）
 * TIM_Period            都有
 * TIM_ClockDivision     TIMx,x[6,7]没有，其他都有(基本定时器)
 * TIM_RepetitionCounter TIMx,x[1,8]才有(高级定时器)
 *-----------------------------------------------------------------------------
 */


void TIM3_Int_Init(u16 arr,u16 psc)//arr10-1,psc840-1
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向下计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	TIM_Cmd(TIM2,ENABLE); //使能定时器2
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4; //抢占优先级4
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0; //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}




void TIM3_IRQHandler(void)
{
	uint32_t ulReturn;
	ulReturn=taskENTER_CRITICAL_FROM_ISR();//进入临界段
	if ( TIM_GetITStatus(TIM3,TIM_IT_Update)==SET ) 
	{
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
		static uint16_t usTimeCnt_1ms = 0;
		static uint8_t  ucTimeCnt_100us = 0;
	
		if (TimingDelay != 0x00) 
     { 
       TimingDelay--;
     }
	
		t1s++;
		if(t1s > 10000)
			t1s=0;
         	
		ucTimeCnt_100us++;
		if(10 == ucTimeCnt_100us)
		{
			ucTimeCnt_100us = 0;
			usTimeCnt_1ms++;
			if(1000 == usTimeCnt_1ms)
			{
				usTimeCnt_1ms = 0;
				ucTime_1s_flag = 1;
			
				on_t1s_event();
			}
			on_t1ms_event();
		}
		on_t100us_event();
		
	}
		taskEXIT_CRITICAL_FROM_ISR(ulReturn);//退出临界段
}	


/*****************************************************************
  * 函数名称：   获取过秒标志
  * 参    数：   无
  * 反 馈 值：   0 : 未过秒；1：已过秒 
  *	注    意：	 
******************************************************************/
uint8_t get_sec_tick_flag(void)
{
	if(ucTime_1s_flag)
	{
		ucTime_1s_flag = 0;
		return 1;
	}
	return 0;
}



/*****************************************************************
  * 函数名称：   获取过秒后时钟tick个数
  * 参    数：   无
  * 反 馈 值：   过秒后时钟tick个数
  *	注    意：	
******************************************************************/
uint32_t get_t1s_tc_value(void)
{
	return t1s;
}


/*****************************************************************
  * 函数名称：   100us周期处理函数
  * 参    数：   无
  * 反 馈 值：   无
  *	注    意：	1、被嘀嗒时钟中断调用，弱定义函数
******************************************************************/
__weak void on_t100us_event(void)
{
	return;
}


/*****************************************************************
  * 函数名称：   1ms周期处理函数
  * 参    数：   无
  * 反 馈 值：   无
  *	注    意：	1、被嘀嗒时钟中断调用，弱定义函数，外部重新定义
******************************************************************/
__weak void on_t1ms_event(void)
{
	return;
}
/*********************************************END OF FILE**********************/
