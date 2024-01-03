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
 
//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		512  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define SG90_TASK_PRIO		4
//�����ջ��С	
#define SG90_STK_SIZE 		512  
//������
TaskHandle_t SG90Task_Handler;
//������
void SG90_task(void *pvParameters);

//�������ȼ�
#define LCD_TASK_PRIO		3
//�����ջ��С	
#define LCD_STK_SIZE 		512  
//������
TaskHandle_t LCDTask_Handler;
//������
void LCD_task(void *pvParameters);


//�������ȼ�
#define RFID_TASK_PRIO		3
//�����ջ��С	
#define RFID_STK_SIZE 		512  
//������
TaskHandle_t RFIDTask_Handler;
//������
void RFID_task(void *pvParameters);

//�������ȼ�
#define AS608_TASK_PRIO		3
//�����ջ��С	
#define AS608_STK_SIZE 		512  
//������
TaskHandle_t AS608Task_Handler;
//������
void AS608_task(void *pvParameters);

//�������ȼ�
#define ESP8266_TASK_PRIO		3
//�����ջ��С	
#define ESP8266_STK_SIZE 		512  
//������
TaskHandle_t ESP8266Task_Handler;
//������
void ESP8266_task(void *pvParameters);








EventGroupHandle_t EventGroupHandler;	//�¼���־����

#define EVENTBIT_0	(1<<0)				//�¼�λ
#define EVENTBIT_1	(1<<1)
#define EVENTBIT_2	(1<<2)
#define EVENTBIT_ALL	(EVENTBIT_0|EVENTBIT_1|EVENTBIT_2)


 const  u8* kbd_menu[15]={"coded"," : ","lock","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//������
 u8 sg90flag;
 u8 rfidflag;
 u8 key;
 u8 err=0;

 
 
 
 
int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	usart3_init(115200);  //��ʼ������3������Ϊ115200
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();							//��ʼ������
	TIM14_PWM_Init(40000-1,42-1);
	set_Angle(0);
	LCD_Init();							//��ʼ��LCD
    tp_dev.init();			//��ʼ�������� 
	RC522_Init();
	usart2_init(57600);//��ʼ������2,������ָ��ģ��ͨѶ
	PS_StaGPIO_Init();	//��ʼ��FR��״̬����
    AS608_Init();
	BEEP_Init();
	
	esp8266_start_trans();
	
	if(!(tp_dev.touchtype&0x80))//����ǵ�����
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
				TP_Adjust();  	 //��ĻУ׼ 
				TP_Save_Adjdata();//����У׼����
				break;				
			}
		}
   }
	
	AS608_load_keyboard(0,170,(u8**)kbd_menu);//�����������
	 
	
	Chinese_Show_one(65,0,0,16,0);
	Chinese_Show_one(85,0,2,16,0);
	Chinese_Show_one(105,0,4,16,0);
	Chinese_Show_one(125,0,6,16,0);
	Chinese_Show_one(145,0,8,16,0);
	Chinese_Show_one(165,0,10,16,0);
	
	
	
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
	BaseType_t xReturn;
    taskENTER_CRITICAL();           //�����ٽ���
	
	
	EventGroupHandler=xEventGroupCreate();
	if(NULL!=EventGroupHandler)
		printf("EventGroupHandler�¼������ɹ�\r\n");
		
	xReturn=xTaskCreate((TaskFunction_t )SG90_task,             
                (const char*    )"SG90_task",           
                (uint16_t       )SG90_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )SG90_TASK_PRIO,        
                (TaskHandle_t*  )&SG90Task_Handler);  
	if(xReturn==pdPASS)
		printf("SG90_TASK_PRIO���񴴽��ɹ�\r\n");
	
	
    
    xReturn=xTaskCreate((TaskFunction_t )LCD_task,             
                (const char*    )"LCD_task",           
                (uint16_t       )LCD_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LCD_TASK_PRIO,        
                (TaskHandle_t*  )&LCDTask_Handler); 
   if(xReturn==pdPASS)
		 printf("LCD_TASK_PRIO���񴴽��ɹ�\r\n");	

   xReturn=xTaskCreate((TaskFunction_t )RFID_task,             
                (const char*    )"RFID_task",           
                (uint16_t       )RFID_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )RFID_TASK_PRIO,        
                (TaskHandle_t*  )&RFIDTask_Handler); 
   if(xReturn==pdPASS)
		 printf("RFID_TASK_PRIO���񴴽��ɹ�\r\n");

   xReturn=xTaskCreate((TaskFunction_t )AS608_task,             
                (const char*    )"AS608_task",           
                (uint16_t       )AS608_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )AS608_TASK_PRIO,        
                (TaskHandle_t*  )&AS608Task_Handler); 
   if(xReturn==pdPASS)
		 printf("AS608_TASK_PRIO���񴴽��ɹ�\r\n");
	 
	  xReturn=xTaskCreate((TaskFunction_t )ESP8266_task,             
                (const char*    )"ESP8266_task",           
                (uint16_t       )ESP8266_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )ESP8266_TASK_PRIO,        
                (TaskHandle_t*  )&ESP8266Task_Handler); 
   if(xReturn==pdPASS)
		 printf("ESP8266_TASK_PRIO���񴴽��ɹ�\r\n");
	 
	 
	 
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}


void SG90_task(void * pvParameters)
{
	 volatile EventBits_t EventValue;	
	while(1)
	{
		
		
		   EventValue=xEventGroupWaitBits(EventGroupHandler,EVENTBIT_ALL,pdTRUE,pdFALSE,portMAX_DELAY);
		   
				  printf("�����¼��ɹ�\r\n");
		      set_Angle(180);
			    delay_xms(1000);
			    delay_xms(1000);
			    set_Angle(0);
			    LCD_ShowString(80,150,260,16,16,"              ");

			vTaskDelay(100); //��ʱ10ms��Ҳ����10��ʱ�ӽ���
				
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
				   printf("����������ȷ\r\n");
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
					printf("�����������\r\n");
					LCD_ShowString(80,150,260,16,16,"password error");
					err++;
					if(err==3)
					{
					  vTaskSuspend(SG90Task_Handler);
						printf("����������\r\n");
						LCD_ShowString(0,100,260,16,16,"Task has been suspended");
					}
					
				}					
			vTaskDelay(100); //��ʱ10ms��Ҳ����10��ʱ�ӽ���
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
				printf("ʶ�𿨺ųɹ�\r\n");
				
			
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
			  printf("ʶ�𿨺�ʧ��\r\n");
				err++;
					if(err==3)
					{
					  vTaskSuspend(SG90Task_Handler);
						printf("����������\r\n");
						LCD_ShowString(0,100,260,16,16,"Task has been suspended");
					}
				
			
			}
			
	   vTaskDelay(100); //��ʱ10ms��Ҳ����10��ʱ�ӽ���
	 }

}

void AS608_task(void *pvParameters)
{
  while(1)
	{
		if(PS_Sta)	 //���PS_Sta״̬���������ָ����
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
			   printf("ָ��ʶ��ɹ�");
				 
				 
			
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
			  printf("ָ��ʶ��ʧ��");
				err++;
					if(err==3)
					{
					  vTaskSuspend(SG90Task_Handler);
						printf("����������\r\n");
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
			   printf("���ųɹ�\r\n");
				 xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);
				 memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));
			}				
				
			if(!strstr((const char*)USART3_RX_BUF,"on"))
			{
			  printf("�������\r\n");
				memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));
			}				
			
			
			
			USART3_RX_STA=0;
//			continue;
		}
		vTaskDelay(100);	
	}


}
