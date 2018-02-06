/* Includes ------------------------------------------------------------------*/
#include "subfunction.h"
#include "stm32f0xx_tim.h"
#include "platform_config.h"
#include "usart.h"

/* Private variables ---------------------------------------------------------*/
unsigned char           Key_Polling_Count=0;
unsigned char           Key_State=KEY_RELEASED;
unsigned char           KeyActiveState=KEY_INACTIVE;
unsigned char           g_WatchdogEvent = RESET;
static __IO uint32_t    TimingDelay;

extern unsigned char    Reg_Mode_Start_Flag;
extern unsigned char    Key_Reg_RQST_Flag ;
extern unsigned char    Tx_Buffer[128];
unsigned char           Key_Reg_End_Button_Flag = RESET;

void BuzzerRun(unsigned char Freq, unsigned char BuzzerCount, unsigned char Ontime, unsigned char Offtime)
{
   unsigned char CurrntBuzzerCount;
   
   for (CurrntBuzzerCount = 0 ; CurrntBuzzerCount<BuzzerCount ; CurrntBuzzerCount++)
   {
        TIM_SetCompare1(TIM3,Freq);
        GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);
        Delay(Ontime);
        TIM_SetCompare1(TIM3,0);
        GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_SET);
        Delay(Offtime);     
   }
}

///////////////////////////////////////////////////////////////////////////////
/******************** ȣ�� ��ư ���� �Լ� *******************/
///////////////////////////////////////////////////////////////////////////////
void Key_Polling(void)
{
    /* KEY STATE CHECK -------------------------------------------------------*/
    Check_Key_State();
    
    if (Key_State == KEY_PUSHED && KeyActiveState==KEY_INACTIVE)
    {
      KeyActiveState = KEY_ACTIVE;
      printf ("\r\n[System                     ] Call Button is Pushed\r\n");           
      
      if( Reg_Mode_Start_Flag == SET)  // ��� ��� ���۽� ȣ�� ��ư ����
      {
         Tx_Buffer[5] |= 0x01;
         GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_SET);
             
         BuzzerRun(100, 1,80,10);
         printf ("\r\n[System                     ] KEY Registration Mode Start\r\n");     
      }    
      if((Key_Reg_RQST_Flag == SET) && ( Reg_Mode_Start_Flag == RESET ))       // ��� ��� �� ��� ���  ����� ȣ�� ��ư ���� 
      { 
         Key_Reg_End_Button_Flag = SET;
         GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);
         printf ("\r\n[System                     ] KEY Registration Mode Stop\r\n");     
      }    

      if(Key_Reg_RQST_Flag == RESET)  //  ���� ȣ���
      {
         Tx_Buffer[5] |= 0x01;
         
         printf ("\r\n[System                     ] Calling is Requested\r\n");     
         //Call_Button_Flag = SET;
      }    
    }

    /* Change KEY ACTIVE STATE -> INACTIVE -----------------------------------*/
    if((Key_State == KEY_RELEASED) && (KeyActiveState == KEY_ACTIVE)) 
    {
       KeyActiveState = KEY_INACTIVE;
       printf ("\r\n[System                     ] Call Button is Relesed\r\n");     
    }    
} 

void Check_Key_State(void)
{
    if( !GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3) ) 
    {
      Key_Polling_Count++;
      if (Key_Polling_Count > KEY_POLLING_CHECKCOUNT)
      {
        Key_State = KEY_PUSHED;
      }
    }
    else
    {
      Key_Polling_Count=0;
      Key_State = KEY_RELEASED;
    }
}


void SysInit(void)
{
        RCC_Configuration();
        GPIO_Config();
        NVIC_Config();
        USART_Configuration();
        TIM_Config();  
}


void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}


void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}


/******************** ��ġ�� �̺�Ʈ ���� �Լ� *******************/
void  WatchDogTask(void)
{
      if ( g_WatchdogEvent )
      {
            g_WatchdogEvent = RESET;
            //Watch_Dog_Falg = SET;
            WatchDog_Reset();
       }
}