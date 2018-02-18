/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include "stdio.h"
#include "usart.h"
#include "platform_config.h"
#include "subfunction.h"

/* Private define ------------------------------------------------------------*/
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

PUTCHAR_PROTOTYPE
{
  USART_SendData(USART1, (uint8_t) ch);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)    {}
  return ch;
}

/* Private variables ---------------------------------------------------------*/
/*************************** USART 1 ******************************/
unsigned char U1_Rx_Count = 0;
unsigned char U1_Rx_DataSavePosition = 0;
unsigned char U1_Rx_DataPosition = 0;
unsigned char U1_Rx_Buffer[U1_RX_BUFFER_SIZE] = {0};
/*************************** USART 2 ******************************/
unsigned char U2_Rx_Count = 0 ;
unsigned char U2_Rx_Compli_Flag = RESET;
unsigned char U2_Rx_Buffer[U2_RX_BUFFER_SIZE] = {0};
unsigned char U2_Rx_Buffer_R[U2_RX_BUFFER_SIZE] = {0};
extern unsigned char U2_Tx_Buffer[128] ;

/*************************** FLAG *********************************/
extern unsigned char RFKey_Detected,Received_RF_KeyData_Count,RF_Key_CNT ;
unsigned int RF_KeyDATA_EraseTime_CNT = 0;
extern unsigned char Key_Reg_End_Button_Flag ;
extern unsigned char RF_Comm_Time_Out_Flag;
volatile unsigned char RF_Comm_Time_Out_Flag_CNT = 0;
extern unsigned char Key_Reg_Timeout_flag;
unsigned int Key_Reg_Timeout_CNT = 0;
unsigned char CNT = 0;
//Need to Check

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



void TIM14_IRQHandler(void) //10ms
{
    if (TIM_GetITStatus(TIM14, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
          
        if(RFKey_Detected)                     RF_KeyDATA_EraseTime_CNT++;
        if(RF_KeyDATA_EraseTime_CNT == 200)  // 1 s  
        {
              RF_KeyDATA_EraseTime_CNT = 0;
              RFKey_Detected=RESET;
              Received_RF_KeyData_Count=0;
              RF_Key_CNT=0;
        }          
        
        if(RF_Comm_Time_Out_Flag)       RF_Comm_Time_Out_Flag_CNT++;
        if(RF_Comm_Time_Out_Flag_CNT == 10)  // 100ms, RF 모듈이 응답해야 하는 최소 시간
        {
              RF_Comm_Time_Out_Flag = RESET;
              RF_Comm_Time_Out_Flag_CNT = 0;
              
              U2_Tx_Buffer[5] |= 0x80;
        }        
        
        if(Key_Reg_Timeout_flag)             Key_Reg_Timeout_CNT ++;
        if(Key_Reg_Timeout_CNT == 30000) // 5분 
        {
              Key_Reg_Timeout_CNT = 0;
              Key_Reg_End_Button_Flag = SET;  // 타임 아웃시 종료 버튼 눌렸다고 강제 설정
              Key_Reg_Timeout_flag = RESET;
        }
    }
}

void USART2_IRQHandler(void)     
{
        if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
        {
                U2_Rx_Buffer_R[U2_Rx_Count++] = USART_ReceiveData(USART2);  
                if(  U2_Rx_Buffer_R[0] != STX )            {  U2_Rx_Count = 0; }
                if(  (U2_Rx_Count == 2) && (U2_Rx_Buffer_R[1] != RF_Camera_ID) )   { U2_Rx_Count = 0; }
                if( U2_Rx_Buffer_R[2] == U2_Rx_Count ) 
                {
                      for (unsigned char tmp=0 ; tmp<U2_Rx_Count; tmp++)
                      {
                          U2_Rx_Buffer[tmp]=U2_Rx_Buffer_R[tmp];
                      }                  
                      U2_Rx_Compli_Flag = SET ; 
                      U2_Rx_Count = 0;
#if 1                      
                      printf ("\r\n[System                ] U2_Rx_Buffer_R : ");      
                      for (unsigned char tmp=0 ; tmp<U2_Rx_Buffer_R[2] ; tmp++)
                      {
                        printf ("  %x ",U2_Rx_Buffer_R[tmp]) ;
                      }                    
                      printf ("\r\n[System                ] U2_Rx_Buffer   : ");      
                      for (unsigned char tmp=0 ; tmp<U2_Rx_Buffer_R[2] ; tmp++)
                      {
                        printf ("  %x ",U2_Rx_Buffer[tmp]) ;
                      }                         
#endif
                }
        } // end of RX if
      
        if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
        {

        } // end of TX if
    
} 


void USART1_IRQHandler(void)     
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {    
            U1_Rx_Buffer[U1_Rx_DataSavePosition] = USART_ReceiveData(USART1);  
            U1_Rx_Count++;
            U1_Rx_DataSavePosition++;
    }

    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {

    }
}


