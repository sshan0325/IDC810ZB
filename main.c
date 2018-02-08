/* Includes ---------------------------------------------------------------*/
#include "platform_config.h"
#include "stdio.h"
#include "usart.h"
#include "subfunction.h"
    

/* Private variables ---------------------------------------------------------*/
//Seungshin Using
/* UART  Ch1 ------------------------------------------------------------*/
extern unsigned char U1_Rx_Buffer[U1_RX_BUFFER_SIZE];
extern unsigned char U1_Rx_Count;    

/* UART  Ch2 ------------------------------------------------------------*/
extern unsigned char    U2_Tx_Buffer[128];

//Need to Check
unsigned char Button_Flag = RESET;


extern unsigned char Tx_LENGTH;
extern unsigned char RF_DATA_RQST_Flag;
extern unsigned char Usual_RF_Detec_Flag;
extern unsigned char RF_Key_CNT ;
extern unsigned char Usaul_RF_Detec_Erase_Flag;
extern unsigned char RF_Key_Detec_CNT_Flag ;


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
            
            U2_Tx_Buffer[5] = 0x00;
                  
             Tx_LENGTH = 7;
             RF_DATA_RQST_Flag = RESET;
             Usual_RF_Detec_Flag = RESET;
      }
}

//////////////////////////////////////////////////////////////////////////////////////////////
/***************************** Main Function *********************************/
//////////////////////////////////////////////////////////////////////////////////////////////
int main(void)
{  
        SysInit();
        
        #ifdef Consol_LOG        
        printf ("\r\n[System                     ] Power On\r\n");     
        printf ("\r\n[System                     ] System Init Done.\r\n");     
        #endif
        
        if (SysTick_Config(SystemCoreClock / 1000))                   while (1);

        while (1)
        {
              RF_Key_Paket_handler();                // RF 모듈 패킷 핸들러
              Packet_handler();                          // 월 패드 패킷 핸들러
              RF_Packet_Erase_handler();           // 타임 아웃시 RF 모듈 패킷 삭제 핸들러
              Key_Polling();                               // 호출 버튼 감지 함수
              WatchDogTask();                          // 왓치독 이베트 셋팅 함수
        }

}
//////////////////////////////////////////////////////////////////////////////////////////////

