/* Includes ---------------------------------------------------------------*/
#include "platform_config.h"
#include "stdio.h"
#include "usart.h"
#include "subfunction.h"
#include "RF_KEY.h"    

/* Private variables ---------------------------------------------------------*/


//////////////////////////////////////////////////////////////////////////////////////////////
/***************************** Main Function *********************************/
//////////////////////////////////////////////////////////////////////////////////////////////
int main(void)
{  
        SysInit();
        
        #ifdef Consol_LOG        
        printf ("\r\n[System                ] Power On");     
        printf ("\r\n[System                ] System Init Done.\r\n");     
        #endif
        
        if (SysTick_Config(SystemCoreClock / 1000))                   while (1);

        while (1)
        {
              RF_Key_Packet_handler();              // RF 모듈 패킷 핸들러
              Packet_handler();                          // 월 패드 패킷 핸들러
              Key_Polling();                               // 호출 버튼 감지 함수
              WatchDog_Reset();                        // 왓치독 이베트 셋팅 함수
        }

}
//////////////////////////////////////////////////////////////////////////////////////////////

