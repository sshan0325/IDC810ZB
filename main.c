  /**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-May-2012
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  **/

/* Includes ---------------------------------------------------------------*/
#include "platform_config.h"
#include "stdio.h"
#include "usart.h"
    

/* Private variables ---------------------------------------------------------*/
//Seungshin Using
/* UART  Ch1-------------------------------------------------------------*/
extern unsigned char U1_Rx_Buffer[U1_RX_BUFFER_SIZE];
extern unsigned char U1_Rx_Count;    




//Need to Check
unsigned char Call_Button ;
unsigned char Key_Reg_End_Button_Flag = RESET;
unsigned char LED_ON_Flag = RESET;
unsigned char Button_Flag = RESET;
unsigned char Call_Button_Flag = RESET;
unsigned char g_WatchdogEvent = RESET;
static __IO uint32_t TimingDelay;
extern unsigned char Tx_LENGTH;
extern unsigned char RF_DATA_RQST_Flag;
extern unsigned char Usual_RF_Detec_Flag;
extern unsigned char Reg_Mode_Start_Flag;
extern unsigned char Timer14_100ms_Flag ;
extern unsigned char Key_Reg_RQST_Flag ;
extern unsigned char Tx_Buffer[128];
extern unsigned char RF_Key_CNT ;
extern unsigned char Usaul_RF_Detec_Erase_Flag;
extern unsigned char RF_Key_Detec_CNT_Flag ;

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{  
        SysInit();
        
        #ifdef Consol_LOG        
        printf ("\r\n[System                     ] Power On\r\n");     
        printf ("\r\n[System                     ] System Init Done.\r\n");     
        #endif
        
        if (SysTick_Config(SystemCoreClock / 1000))
        { 
              /* Capture error */ 
              while (1);
        }
        

        
        while (1)
        {
              RF_Key_Paket_handler();                // RF 모듈 패킷 핸들러
              Packet_handler();                          // 월 패드 패킷 핸들러
              RF_Packet_Erase_handler();           // 타임 아웃시 RF 모듈 패킷 삭제 핸들러
              Key_Polling();                               // 호출 버튼 감지 함수
              WatchDogTask();                          // 왓치독 이베트 셋팅 함수
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


///////////////////////////////////////////////////////////////////////////////
/******************** 호출 버튼 제어 함수 *******************/
/////////////////////////////////////////////////////////////////////////////
void Key_Polling(void)
{
        if(Timer14_100ms_Flag == SET)           // 100ms 마다 호출 버튼 눌림 체크
        {
          
                 Timer14_100ms_Flag = RESET;
                
                 Call_Button = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3);
                 
    
                 
                 if((Call_Button == Bit_RESET) && (Button_Flag == RESET))  //  호출 버튼 눌림 
                 {
                       Button_Flag = SET;
                           
                       
                       if(Reg_Mode_Start_Flag == SET)  // 등록 모드 시작시 호출 버튼 누름
                       {
                         
                             Tx_Buffer[5] |= 0x01;
                             
                             GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_SET);
                                
                             TIM_SetCompare1(TIM3,100);   // LED ON , Buzzer 1회 발생
                              Delay(80);
                             TIM_SetCompare1(TIM3,0);
                       }    
                         
                       
                       
                       if(Key_Reg_RQST_Flag == RESET)  //  평상시 호출시
                       {
                              GPIO_WriteBit(GPIOB,GPIO_Pin_14,(BitAction) Bit_SET);   // LED ON
                              Tx_Buffer[5] |= 0x01;
                             
                              Call_Button_Flag = SET;
                              LED_ON_Flag = SET;
                       }    
                         
                       
                       if((Key_Reg_RQST_Flag == SET) && ( Reg_Mode_Start_Flag == RESET ))       // 등록 모드 중 등록 모드  종료시 호출 버튼 누름 
                       { 
                        
           
                               Key_Reg_End_Button_Flag = SET;
                        
                 
                               GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);
                         
                        
                       }
                       
                 } // end of 버튼 눌림 
                 
                 if((Call_Button == Bit_SET) && (Button_Flag == SET))  // 호출 버튼 누르고 땜
                 {
                   
                        Button_Flag = RESET;
                 
                 }
        
          
        } // end of if 100ms
        
} // end of Key_Polling()


///////////////////////////////////////////////////////////////////////////////////////////////
/******************** 타임 아웃시 RF 모듈 패킷 삭제 함수 *******************/
////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Packet_Erase_handler(void)     
{
  
      if(Usaul_RF_Detec_Erase_Flag == SET)
      {
            Usaul_RF_Detec_Erase_Flag = RESET;
            
            for(unsigned char i = 0 ; i < (RF_Key_CNT * 17) ; i ++)             // RF 패킷 초기화 
            {
              U1_Rx_Buffer[i] = 0;
            }
            
            RF_Key_CNT = 0;                                     // RF 관련 카운트 및 플래그 초기화
            U1_Rx_Count = 0;
            
            Tx_Buffer[5] = 0x00;
                  
             Tx_LENGTH = 7;
             RF_DATA_RQST_Flag = RESET;
             Usual_RF_Detec_Flag = RESET;
      }
      
      

}

///////////////////////////////////////////////////////////////////////////////////////////////
/******************** 왓치독 이벤트 셋팅 함수 *******************/
////////////////////////////////////////////////////////////////////////////////////////////////
void  WatchDogTask(void)
{

      if ( g_WatchdogEvent )
      {
            g_WatchdogEvent = RESET;
            
            //Watch_Dog_Falg = SET;
            WatchDog_Reset();
       }
      
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


