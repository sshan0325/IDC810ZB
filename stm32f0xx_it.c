/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-May-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
//#include "main.h"
#include "stdio.h"
#include "platform_config.h"
/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*************************** USART 2 ******************************/
unsigned char Rx_Count = 0 ;
unsigned char Rx_Compli_Flag = RESET;
unsigned char Rx_Buffer[128] = {0};



/*************************** TIMMER 14 ******************************/
unsigned char Timer14_CNT = 0;
unsigned char Timer14_100ms_Flag = RESET;
extern unsigned char LED_ON_Flag ;
unsigned int LED_ON_CNT = 0;
extern unsigned char Key_Reg_Timeout_flag;
//unsigned int Key_Reg_Timeout_CNT = 0;
unsigned int RF_Detec_Timeout_CNT = 0;
unsigned char Usaul_RF_Detec_Erase_Flag = RESET;
unsigned char RF_Detec_Timeout_Flag = RESET;
/*************************** USART 1 ******************************/


unsigned char U1_Rx_Buffer[128] = {0};
unsigned char U1_Rx_Count = 0;
unsigned char U1_Rx_Compli_Flag = RESET;

unsigned char RF_Key_CNT = 0;
unsigned char Reg_key_Value_Receive_Flag = RESET;
unsigned char Usual_RF_Detec_Flag = RESET;

extern unsigned char Key_Reg_End_Button_Flag ;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  TimingDelay_Decrement();
}


extern unsigned char Tx_Buffer[128] ;
//unsigned char Timer14_Delay_CNT = 0;
//extern unsigned char Response_Delay_Flag ;
extern unsigned char Time_Out_Flag;
extern unsigned char RF_Key_Detec_CNT_Flag ;
volatile unsigned char Time_Out_Flag_CNT = 0;

unsigned int Watch_Dog_Flag_CNT = 0;
extern unsigned char g_WatchdogEvent ;
unsigned char CNT = 0;
unsigned char Watch_Dog_Flag = SET ;

unsigned int Key_Reg_Timeout_CNT = 0;

void TIM14_IRQHandler(void) //10ms
{
    if (TIM_GetITStatus(TIM14, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
          
        Timer14_CNT ++;
        
        if(LED_ON_Flag)                         LED_ON_CNT++;
        if(RF_Detec_Timeout_Flag)           RF_Detec_Timeout_CNT++;
        if(Watch_Dog_Flag)                      Watch_Dog_Flag_CNT ++;
        if(Time_Out_Flag)                       Time_Out_Flag_CNT++;
        if(Key_Reg_Timeout_flag)             Key_Reg_Timeout_CNT ++;
        
        
        
        
        if(Timer14_CNT == 10 )   //100ms KEY POOLING
        {
            Timer14_100ms_Flag = SET;
            Timer14_CNT = 0;       
        }
        
        
   
        if(LED_ON_CNT == 200)    // 호출  버튼 누르고 20초 후 LED OFF 
        {
              LED_ON_CNT = 0;
              LED_ON_Flag = RESET;
              GPIO_WriteBit(GPIOB,GPIO_Pin_14,(BitAction) Bit_RESET);
         
        }
        
        if(RF_Detec_Timeout_CNT == 3000)  // 30 초, 키 인식후 데이터 삭제
        {
              RF_Detec_Timeout_CNT = 0;
              Usaul_RF_Detec_Erase_Flag = SET;
              
              RF_Detec_Timeout_Flag = RESET;
        }
        
        if(Time_Out_Flag_CNT == 5)  // 50ms, RF 모듈이 응답해야 하는 최소 시간
        {
              Time_Out_Flag = RESET;
              Time_Out_Flag_CNT = 0;
              
              Tx_Buffer[5] |= 0x80;
              RF_Key_Detec_CNT_Flag = SET;

        }
        
           
        if(Key_Reg_Timeout_CNT == 30000) // 5분 
        {
              Key_Reg_Timeout_CNT = 0;
              
              Key_Reg_End_Button_Flag = SET;  // 타임 아웃시 종료 버튼 눌렸다고 강제 설정
              Key_Reg_Timeout_flag = RESET;
        }
        
        if(Watch_Dog_Flag_CNT == 300)     // 3초, 
        {
          
            RF_Key_Detec_CNT_Flag = SET;
            Time_Out_Flag_CNT = 0;
        }
        
        if(Watch_Dog_Flag_CNT == 2000) // 20 초  중간에 통신 끊어 질때 대비 
        {
          
            Watch_Dog_Flag_CNT = 0;      
             
            g_WatchdogEvent = SET;  // 20 초마다 왓치독  이벤트 셋팅 
            
            CNT ++;
            
            if(CNT == 90)  //-> 30분마다 왓치독 리셋( 20초 * 30 : 10분, )
            {
                  CNT = 0;
                  g_WatchdogEvent = RESET;
                  Watch_Dog_Flag = RESET;
               
            }
            
          
        }
    }
    
}

void USART2_IRQHandler(void)     // 월패트 통신 인터럽트
{
        if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
        {
                Rx_Buffer[Rx_Count++] = USART_ReceiveData(USART2);  
                if(  Rx_Buffer[0] != STX )            {  Rx_Count = 0; }
                if(  (Rx_Count == 2) && (Rx_Buffer[1] != RF_Camera_ID) )   { Rx_Count = 0; }
                if( Rx_Buffer[2] == Rx_Count ) 
                {
                      Rx_Compli_Flag = SET ; 
                      Rx_Count = 0;
                }
        } // end of RX if
      
        if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
        {

        } // end of TX if
    
}  // end of UART IQR




unsigned char U1_Rx_71_Compli_Flag = RESET;

void USART1_IRQHandler(void)     // RF 모듈 통신 인터럽트
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {    
            U1_Rx_Buffer[U1_Rx_Count++] = USART_ReceiveData(USART1);  
            if((U1_Rx_Buffer[0] != 0x71)  && (U1_Rx_Buffer[0] != 0xCA) && (U1_Rx_Buffer[0] != 0xBA) && (U1_Rx_Buffer[0] != 0xDA))
            { // 쓰레기값 처리
                  U1_Rx_Count = 0 ;
            }
             
            if((U1_Rx_Buffer[0] == 0x71) && ((RF_Key_CNT * 17) == U1_Rx_Count))
            {  // 평상 시 키 인식시
                  U1_Rx_71_Compli_Flag = SET;
            }
               
            if((U1_Rx_Buffer[0] == 0xCA) || (U1_Rx_Buffer[0] == 0xBA) || (U1_Rx_Buffer[0] == 0xDA))
            {    // 키등록시  , 평상시 키인식시 개수 정보 패킷     
                  if((U1_Rx_Count % 17 ) == 0) //////////////////////////////////////
                  {   
                        U1_Rx_Compli_Flag = SET; 
                  }
            }
    }

    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {

    }
}

PUTCHAR_PROTOTYPE
{
  USART_SendData(USART1, (uint8_t) ch);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
  {}
  return ch;
}
