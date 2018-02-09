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
              Packet_handler();                          // �� �е� ��Ŷ �ڵ鷯
              RF_Packet_Erase_handler();           // Ÿ�� �ƿ��� RF ��� ��Ŷ ���� �ڵ鷯
              Key_Polling();                               // ȣ�� ��ư ���� �Լ�
              WatchDogTask();                          // ��ġ�� �̺�Ʈ ���� �Լ�
        }

}
//////////////////////////////////////////////////////////////////////////////////////////////

