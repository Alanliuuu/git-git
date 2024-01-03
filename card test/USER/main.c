#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "pwm.h"
#include "event_groups.h"
#include "lcd.h"
#include "touch.h"
#include "rc52.h"
#include "usart2.h"
#include "AS608.h"
#include "beep.h"
#include "esp8266.h"
#include "usart3.h"
#include "string.h"
 
//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		512  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define SG90_TASK_PRIO		4
//任务堆栈大小	
#define SG90_STK_SIZE 		512  
//任务句柄
TaskHandle_t SG90Task_Handler;
//任务函数
void SG90_task(void *pvParameters);

//任务优先级
#define LCD_TASK_PRIO		3
//任务堆栈大小	
#define LCD_STK_SIZE 		512  
//任务句柄
TaskHandle_t LCDTask_Handler;
//任务函数
void LCD_task(void *pvParameters);


//任务优先级
#define RFID_TASK_PRIO		3
//任务堆栈大小	
#define RFID_STK_SIZE 		512  
//任务句柄
TaskHandle_t RFIDTask_Handler;
//任务函数
void RFID_task(void *pvParameters);

//任务优先级
#define AS608_TASK_PRIO		3
//任务堆栈大小	
#define AS608_STK_SIZE 		512  
//任务句柄
TaskHandle_t AS608Task_Handler;
//任务函数
void AS608_task(void *pvParameters);

//任务优先级
#define ESP8266_TASK_PRIO		3
//任务堆栈大小	
#define ESP8266_STK_SIZE 		512  
//任务句柄
TaskHandle_t ESP8266Task_Handler;
//任务函数
void ESP8266_task(void *pvParameters);








EventGroupHandle_t EventGroupHandler;	//事件标志组句柄

#define EVENTBIT_0	(1<<0)				//事件位
#define EVENTBIT_1	(1<<1)
#define EVENTBIT_2	(1<<2)
#define EVENTBIT_ALL	(EVENTBIT_0|EVENTBIT_1|EVENTBIT_2)


 const  u8* kbd_menu[15]={"coded"," : ","lock","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//按键表
 u8 sg90flag;
 u8 rfidflag;
 u8 key;
 u8 err=0;

 
 
 
 
int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组4
	delay_init(168);					//初始化延时函数
	uart_init(115200);     				//初始化串口
	usart3_init(115200);  //初始化串口3波特率为115200
	LED_Init();		        			//初始化LED端口
	KEY_Init();							//初始化按键
	TIM14_PWM_Init(40000-1,42-1);
	set_Angle(0);
	LCD_Init();							//初始化LCD
    tp_dev.init();			//初始化触摸屏 
	RC522_Init();
	usart2_init(57600);//初始化串口2,用于与指纹模块通讯
	PS_StaGPIO_Init();	//初始化FR读状态引脚
    AS608_Init();
	BEEP_Init();
	
	esp8266_start_trans();
	
	if(!(tp_dev.touchtype&0x80))//如果是电阻屏
	{
		LCD_ShowString(0,30,200,16,16,"Adjust the LCD ?");
		POINT_COLOR=BLUE;
		LCD_ShowString(0,60,200,16,16,"yes:KEY1 no:KEY0");	
		while(1)
		{		
			key=KEY_Scan(0);
			if(key==KEY0_PRES)
				break;
			if(key==KEY1_PRES)
			{
				LCD_Clear(WHITE);
				TP_Adjust();  	 //屏幕校准 
				TP_Save_Adjdata();//保存校准参数
				break;				
			}
		}
   }
	
	AS608_load_keyboard(0,170,(u8**)kbd_menu);//加载虚拟键盘
	 
	
	Chinese_Show_one(65,0,0,16,0);
	Chinese_Show_one(85,0,2,16,0);
	Chinese_Show_one(105,0,4,16,0);
	Chinese_Show_one(125,0,6,16,0);
	Chinese_Show_one(145,0,8,16,0);
	Chinese_Show_one(165,0,10,16,0);
	
	
	
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
	BaseType_t xReturn;
    taskENTER_CRITICAL();           //进入临界区
	
	
	EventGroupHandler=xEventGroupCreate();
	if(NULL!=EventGroupHandler)
		printf("EventGroupHandler事件创建成功\r\n");
		
	xReturn=xTaskCreate((TaskFunction_t )SG90_task,             
                (const char*    )"SG90_task",           
                (uint16_t       )SG90_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )SG90_TASK_PRIO,        
                (TaskHandle_t*  )&SG90Task_Handler);  
	if(xReturn==pdPASS)
		printf("SG90_TASK_PRIO任务创建成功\r\n");
	
	
    
    xReturn=xTaskCreate((TaskFunction_t )LCD_task,             
                (const char*    )"LCD_task",           
                (uint16_t       )LCD_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LCD_TASK_PRIO,        
                (TaskHandle_t*  )&LCDTask_Handler); 
   if(xReturn==pdPASS)
		 printf("LCD_TASK_PRIO任务创建成功\r\n");	

   xReturn=xTaskCreate((TaskFunction_t )RFID_task,             
                (const char*    )"RFID_task",           
                (uint16_t       )RFID_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )RFID_TASK_PRIO,        
                (TaskHandle_t*  )&RFIDTask_Handler); 
   if(xReturn==pdPASS)
		 printf("RFID_TASK_PRIO任务创建成功\r\n");

   xReturn=xTaskCreate((TaskFunction_t )AS608_task,             
                (const char*    )"AS608_task",           
                (uint16_t       )AS608_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )AS608_TASK_PRIO,        
                (TaskHandle_t*  )&AS608Task_Handler); 
   if(xReturn==pdPASS)
		 printf("AS608_TASK_PRIO任务创建成功\r\n");
	 
	  xReturn=xTaskCreate((TaskFunction_t )ESP8266_task,             
                (const char*    )"ESP8266_task",           
                (uint16_t       )ESP8266_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )ESP8266_TASK_PRIO,        
                (TaskHandle_t*  )&ESP8266Task_Handler); 
   if(xReturn==pdPASS)
		 printf("ESP8266_TASK_PRIO任务创建成功\r\n");
	 
	 
	 
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}


void SG90_task(void * pvParameters)
{
	 volatile EventBits_t EventValue;	
	while(1)
	{
		
		
		   EventValue=xEventGroupWaitBits(EventGroupHandler,EVENTBIT_ALL,pdTRUE,pdFALSE,portMAX_DELAY);
		   
				  printf("接收事件成功\r\n");
		      set_Angle(180);
			    delay_xms(1000);
			    delay_xms(1000);
			    set_Angle(0);
			    LCD_ShowString(80,150,260,16,16,"              ");

			vTaskDelay(100); //延时10ms，也就是10个时钟节拍
				
	}	
}


void LCD_task(void * pvParameters)
{
	while(1)
	{
		 
			  if(sg90flag==1||GET_NUM())
				{
					 BEEP=1;
					 delay_xms(100);
					 BEEP=0;
				   printf("密码输入正确\r\n");
					 LCD_ShowString(80,150,260,16,16,"password match");
		       xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);
					 			 
				}
        else
				{
					 BEEP=1;
					 delay_xms(50);
					 BEEP=0;
					 delay_xms(50);
					 BEEP=1;
					 delay_xms(50);
					 BEEP=0;
					delay_xms(50);
					 BEEP=1;
					 delay_xms(50);
					 BEEP=0;
					printf("密码输入错误\r\n");
					LCD_ShowString(80,150,260,16,16,"password error");
					err++;
					if(err==3)
					{
					  vTaskSuspend(SG90Task_Handler);
						printf("舵机任务挂起\r\n");
						LCD_ShowString(0,100,260,16,16,"Task has been suspended");
					}
					
				}					
			vTaskDelay(100); //延时10ms，也就是10个时钟节拍
	}	
}

void RFID_task(void * pvParameters)
{
	
//	 rfidflag=shibieka();
   while(1)
	 {
	    if(rfidflag==1||shibieka())
			{
				   BEEP=1;
					 delay_xms(100);
					 BEEP=0;
				 Chinese_Show_two(30,50,16,16,0);
	       Chinese_Show_two(50,50,18,16,0);
	       Chinese_Show_two(70,50,20,16,0);
	       Chinese_Show_two(90,50,8,16,0);
	       Chinese_Show_two(110,50,10,16,0);
	       
				xEventGroupSetBits(EventGroupHandler,EVENTBIT_1);
				printf("识别卡号成功\r\n");
				
			
			}
			else if(shibieka()==0)
			{
				BEEP=1;
					 delay_xms(50);
					 BEEP=0;
					 delay_xms(50);
					 BEEP=1;
					 delay_xms(50);
					 BEEP=0;
					delay_xms(50);
					 BEEP=1;
					 delay_xms(50);
					 BEEP=0;
				Chinese_Show_two(90,50,12,16,0);
	      Chinese_Show_two(110,50,14,16,0);
			  printf("识别卡号失败\r\n");
				err++;
					if(err==3)
					{
					  vTaskSuspend(SG90Task_Handler);
						printf("舵机任务挂起\r\n");
						LCD_ShowString(0,100,260,16,16,"Task has been suspended");
					}
				
			
			}
			
	   vTaskDelay(100); //延时10ms，也就是10个时钟节拍
	 }

}

void AS608_task(void *pvParameters)
{
  while(1)
	{
		if(PS_Sta)	 //检测PS_Sta状态，如果有手指按下
		{
		   if(press_FR()==1)
			{
				 BEEP=1;
				 delay_xms(100);
				 BEEP=0;
				 Chinese_Show_two(30,25,0,16,0);
	       Chinese_Show_two(50,25,2,16,0);
	       Chinese_Show_two(70,25,4,16,0);
	       Chinese_Show_two(90,25,6,16,0);
	       Chinese_Show_two(110,25,8,16,0);
	       Chinese_Show_two(130,25,10,16,0);
				 xEventGroupSetBits(EventGroupHandler,EVENTBIT_2);
			   printf("指纹识别成功");
				 
				 
			
			}
			else if(press_FR()==0)
			{
				BEEP=1;
					 delay_xms(50);
					 BEEP=0;
					 delay_xms(50);
					 BEEP=1;
					 delay_xms(50);
					 BEEP=0;
					delay_xms(50);
					 BEEP=1;
					 delay_xms(50);
					 BEEP=0;
				Chinese_Show_two(110,25,12,16,0);
	      Chinese_Show_two(130,25,14,16,0);
			  printf("指纹识别失败");
				err++;
					if(err==3)
					{
					  vTaskSuspend(SG90Task_Handler);
						printf("舵机任务挂起\r\n");
						LCD_ShowString(0,100,260,16,16,"Task has been suspended");
					}
			
			}
			     
		}
		vTaskDelay(100);
	}
}

void ESP8266_task(void *pvParameters)
{
  
	
  while(1)
	{
	   if(USART3_RX_STA)
		{
			if(strstr((const char*)USART3_RX_BUF,"on"))
			{
				 BEEP=1;
				 delay_xms(100);
				 BEEP=0;
			   printf("开门成功\r\n");
				 xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);
				 memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));
			}				
				
			if(!strstr((const char*)USART3_RX_BUF,"on"))
			{
			  printf("密码错误\r\n");
				memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));
			}				
			
			
			
			USART3_RX_STA=0;
//			continue;
		}
		vTaskDelay(100);	
	}


}
