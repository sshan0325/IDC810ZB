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
              RF_Key_Packet_handler();              // RF ��� ��Ŷ �ڵ鷯
              printf ("\r\n[System                ] RF_Key_Packet_handler Done.\r\n");     
              Packet_handler();                          // �� �е� ��Ŷ �ڵ鷯
              printf ("\r\n[System                ] Packet_handler Done.\r\n");     
              Key_Polling();                               // ȣ�� ��ư ���� �Լ�
              printf ("\r\n[System                ] Key_Polling Done.\r\n");     
              WatchDog_Reset();                        // ��ġ�� �̺�Ʈ ���� �Լ�
              printf ("\r\n[System                ] WatchDog_Reset Done.\r\n");     
        }

}
//////////////////////////////////////////////////////////////////////////////////////////////

