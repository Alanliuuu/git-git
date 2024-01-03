#ifndef __BASIC_TIM_H
#define	__BASIC_TIM_H

#include "stm32f4xx.h"

#define BASIC_TIM           		TIM6
#define BASIC_TIM_CLK       		RCC_APB1Periph_TIM6

#define BASIC_TIM_IRQn					TIM3_IRQn
#define BASIC_TIM_IRQHandler    TIM6_DAC_IRQHandler

uint8_t get_sec_tick_flag(void);             //获取过秒标志
uint32_t get_t1s_tc_value(void);						//获取过秒后时钟tick个数
__weak void on_t100us_event(void);            //100us周期处理函数
__weak void on_t1ms_event(void);             //1ms周期处理函数
__weak void on_t1s_event(void);              //1s周期处理函数

void TIM3_Int_Init(uint16_t arr,uint16_t psc);//arr10-1,psc840-1

#endif /* __BASIC_TIM_H */

