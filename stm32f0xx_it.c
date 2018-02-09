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
unsigned char U2_Rx_Buffer[128] = {0};
extern unsigned char U2_Tx_Buffer[128] ;

/*************************** TIMER ********************************/
unsigned int    SystemTime_Tick=0;
unsigned char Timer14_CNT = 0;



/*************************** FLAG *********************************/
unsigned int Watch_Dog_Flag_CNT = 0;
unsigned char Usaul_RF_Detec_Erase_Flag = RESET;
unsigned char RF_Detec_Timeout_Flag = RESET;
extern unsigned char Key_Reg_End_Button_Flag ;
extern unsigned char Time_Out_Flag;
extern unsigned char RF_Key_Detec_CNT_Flag ;
volatile unsigned char Time_Out_Flag_CNT = 0;
extern unsigned char Key_Reg_Timeout_flag;
unsigned char Watch_Dog_Flag = SET ;

//Need to Check
unsigned int Key_Reg_Timeout_CNT = 0;
unsigned int RF_Detec_Timeout_CNT = 0;
extern unsigned char g_WatchdogEvent ;
unsigned char CNT = 0;

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
    SystemTime_Tick++;

    if (TIM_GetITStatus(TIM14, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
          
        Timer14_CNT ++;
        
        if(RF_Detec_Timeout_Flag)           RF_Detec_Timeout_CNT++;
        if(Watch_Dog_Flag)                      Watch_Dog_Flag_CNT ++;
        if(Time_Out_Flag)                       Time_Out_Flag_CNT++;
        if(Key_Reg_Timeout_flag)             Key_Reg_Timeout_CNT ++;
        

        if(Time_Out_Flag_CNT == 5)  // 50ms, RF 모듈이 응답해야 하는 최소 시간
        {
              Time_Out_Flag = RESET;
              Time_Out_Flag_CNT = 0;
              
              U2_Tx_Buffer[5] |= 0x80;
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
            
             #ifdef Consol_LOG 
             int tmp=0;
             printf ("\r\n[System                ] Watch_Dog Occured / DataPosition : %d  -  \r\n",U1_Rx_DataPosition);     
             for (tmp=U1_Rx_DataPosition ; tmp<U1_Rx_DataPosition+17 ; tmp++)
             {
               printf ("%x, ",U1_Rx_Buffer[tmp]) ;
             }             
             printf ("\r\n");
             #endif           
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
                U2_Rx_Buffer[U2_Rx_Count++] = USART_ReceiveData(USART2);  
                if(  U2_Rx_Buffer[0] != STX )            {  U2_Rx_Count = 0; }
                if(  (U2_Rx_Count == 2) && (U2_Rx_Buffer[1] != RF_Camera_ID) )   { U2_Rx_Count = 0; }
                if( U2_Rx_Buffer[2] == U2_Rx_Count ) 
                {
                      U2_Rx_Compli_Flag = SET ; 
                      U2_Rx_Count = 0;
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


