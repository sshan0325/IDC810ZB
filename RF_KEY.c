/* Includes ------------------------------------------------------------------*/
#include "RF_KEY.h"
#include "usart.h"
#include "subfunction.h"

/* Private variables ---------------------------------------------------------*/
unsigned char RF_Key_CNT = 0;

/* UART  Ch1-------------------------------------------------------------*/
extern unsigned char    U1_Rx_Buffer[U1_RX_BUFFER_SIZE] ;
extern unsigned char    U1_Rx_Count;
extern unsigned char    U1_Rx_DataPosition;

/* UART  Ch2-------------------------------------------------------------*/
extern unsigned char U2_Tx_Buffer[128];  
extern unsigned char U2_Rx_Buffer[128];  

/* FLAG -----------------------------------------------------------------*/
unsigned char Reg_key_Value_Receive_Flag = RESET;
unsigned char Usual_RF_Detec_Flag = RESET;
extern unsigned char RF_Detec_Timeout_Flag ;
unsigned char value_1 = 6;
unsigned char Reg_Fail_Flag = RESET;
unsigned char RF_Communi_Fail = RESET;
extern unsigned char Time_Out_Flag ;
extern unsigned char Time_Out_Flag_CNT;
extern unsigned char Key_Reg_RQST_Flag;
extern unsigned char RF_DATA_RQST_Flag;

//Need to Check
unsigned char RF_Packet_1_4 = 0x00;
unsigned char RF_Packet_5 = 0x00;
unsigned char RF_Data_Check = RESET;
unsigned char RF_Key_Data[128] = {0};



/* Private functions ---------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** RF ��� ��Ŷ ���� ó�� �Լ�   *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Key_Packet_handler(void)
{
    if(U1_Rx_Count >= RF_KEY_PACKET_SIZE)
    {
        #ifdef U1_DATA_MONITOR
        int tmp=0;
        printf ("         [RF -> CAM] / Position : %d  -  ",U1_Rx_DataPosition) ;            
        for (tmp=U1_Rx_DataPosition ; tmp<U1_Rx_DataPosition+17 ; tmp++)
        {
          printf ("%x  ",U1_Rx_Buffer[tmp]) ;
        }
        printf ("\r\n");
        #endif              
        
        switch (U1_Rx_Buffer[U1_Rx_DataPosition])
        {
        case RF_KEY_CHECK:
                RF_Key_CNT = U1_Rx_Buffer[U1_Rx_DataPosition+2];
            
                U2_Tx_Buffer[6] =  RF_Key_CNT; //U1_DA_Buffer[2];  // Ű ���� ���� 
                
                Time_Out_Flag = RESET;     // RF ��� Ÿ�� �ƿ� �ʱ�ȭ
                Time_Out_Flag_CNT = 0;
                
                U1_Rx_Count -= RF_KEY_PACKET_SIZE;
                U1_Rx_DataPosition = (U1_Rx_DataPosition+RF_KEY_PACKET_SIZE);
                break;
                
        case RF_KEY_RECOGNITION:   
                #ifdef Consol_LOG                      
                printf ("\r\n[RF Key Comm           ] RF_KEY Recognition Done(Key Count : %d)",RF_Key_CNT); 
                #endif                      
                
                Usual_RF_Detec_Flag = SET;
                RF_Data_Save(/*RF_Key_CNT*/ 1 ,&U1_Rx_Buffer[U1_Rx_DataPosition]);  // Ű ������ ���� �Լ� 
                RF_Detec_Timeout_Flag = SET;                  // ���� Ű�ν� Ÿ�� �ƿ� �÷��� ����      

                U1_Rx_Count -= RF_KEY_PACKET_SIZE;
                U1_Rx_DataPosition = (U1_Rx_DataPosition+RF_KEY_PACKET_SIZE);                    
                break;
                
        case RF_KEY_REG_SUCC:
                if(Key_Reg_RQST_Flag == SET) 
                {
                    Reg_key_Value_Receive_Flag = SET;  
                    
                    BuzzerRun(100, 2,100,20);
                    
                    #ifdef Consol_LOG                      
                    printf ("\r\n[RF Key Comm           ] RF_KEY Regist Success \r\n"); 
                    #endif                          
                    
                    U1_Rx_Count -= RF_KEY_PACKET_SIZE;
                    U1_Rx_DataPosition = (U1_Rx_DataPosition+RF_KEY_PACKET_SIZE);
                }
                else 
                {
                    U1_Rx_Count -= RF_KEY_PACKET_SIZE;
                    U1_Rx_DataPosition = (U1_Rx_DataPosition+RF_KEY_PACKET_SIZE);                  
                }
                break;                         
              
        case RF_KEY_REG_FAIL:
                if(Key_Reg_RQST_Flag == SET) 
                {
                    Reg_key_Value_Receive_Flag = RESET;

                    #ifdef Consol_LOG                      
                    printf ("\r\n[RF Key Comm           ] RF_KEY Regist fail\r\n"); 
                    #endif       
                    
                    if(U1_Rx_Buffer[U1_Rx_DataPosition+6] == 0x01) // �̹� ��ϵ� Ű�� ���(������ 4ȸ �߻�)
                    {
                          U2_Tx_Buffer[5] |= 0x04;   // ��ϵ� Ű��� �÷��� ���� �� ������ ����
                          Reg_Fail_Flag = SET;
                            
                          BuzzerRun(100, 4, 70, 15);
                    }
                    else if(U1_Rx_Buffer[U1_Rx_DataPosition+6] == 0x02)  // ��� ���� ���� ���
                    {
                          U2_Tx_Buffer[5] |= 0x08;     // ��� ���ж�� �÷��� ���� �� ������ ����
                          RF_Communi_Fail = SET;
                    }                  

                    U1_Rx_Count -= RF_KEY_PACKET_SIZE;
                    U1_Rx_DataPosition = (U1_Rx_DataPosition+RF_KEY_PACKET_SIZE);                    
                }
                else 
                {
                    U1_Rx_Count -= RF_KEY_PACKET_SIZE;
                    U1_Rx_DataPosition = (U1_Rx_DataPosition+RF_KEY_PACKET_SIZE);                  
                }                
                break;
        default:
            U1_Rx_Count --;
            U1_Rx_DataPosition ++;              
            break;
        }
             
    }
} 
  
    
/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** �νĵ� ����ƮŰ ������ Ȯ�� �Լ�   *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////    
    
void RF_Data_Confirm(unsigned char CNT)  // �νĵ� Ű ������ Ȯ�� �Լ� 
{
    
        for(unsigned char i = 1 ; i <= CNT ; i++ )
        { 
                if(i == 1 )    value_1 = 6 ; 
                if(i == 2 )    value_1 = 22 ; 
                if(i == 3 )    value_1 = 38 ; 
                if(i == 4 )    value_1 = 54 ; 
                if(i == 5 )    value_1 = 70 ; 
                  
                for(unsigned char j = value_1 ; j <= (value_1 + 15) ;j++)
                {
                      if(U2_Rx_Buffer[j] == RF_Key_Data[j-6])          { RF_Data_Check ++;  }
                }
                    
                 
                if((i == 1) && (RF_Data_Check == 16 ))       RF_Packet_1_4 = 0x01;  // 1 ��° ��Ŷ �˻�� ���� ���� �ϱ� ������ �ʱ�ȭ��ƾ�� ���� ��� ��
                if((i == 1) && (RF_Data_Check != 16 ))       RF_Packet_1_4 = 0x02;

                if((i == 2) && (RF_Data_Check == 32 ))       RF_Packet_1_4 |= 0x04;
                if((i == 2) && (RF_Data_Check != 32 ))       RF_Packet_1_4 |= 0x08;

                if((i == 3) && (RF_Data_Check == 48 ))       RF_Packet_1_4 |= 0x10;
                if((i == 3) && (RF_Data_Check != 48 ))       RF_Packet_1_4 |= 0x20;

                if((i == 4) && (RF_Data_Check == 64 ))       RF_Packet_1_4 |= 0x40;
                if((i == 4) && (RF_Data_Check != 64 ))       RF_Packet_1_4 |= 0x80;

                if((i == 5) && (RF_Data_Check == 80 ))       RF_Packet_5 = 0x01;
                if((i == 5) && (RF_Data_Check != 80 ))       RF_Packet_5 = 0x02;

       
            
           
        } // end of for 
    
        U2_Tx_Buffer[6] = RF_Packet_1_4;           // �˻� ��� ���۵����Ϳ� ����
        U2_Tx_Buffer[7] = RF_Packet_5;
        RF_Data_Check = 0;
  
  

} // end of RF_Data_Confirm()
 


/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** �νĵ� ����ƮŰ ������ ���� �Լ�  *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Data_Save(unsigned char CNT, unsigned char *U1_Rx)      // ����ƮŰ ������ ���� �Լ�
{
        
  char j = 0;
  
        for(unsigned char i = 0 ; i < ( CNT * 16 ) ; i++)
        {
                j ++;
               
                if(j == 17)            j = 18 ; 
                if(j == 34)             j = 35;
                if(j == 51)             j = 52;
                if(j == 68)             j = 69;
                  
                RF_Key_Data[i] = U1_Rx[j];
        }
}


 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************** ���� ������ �ʱ�ȭ �Լ� ( Ÿ�Ӿƿ� �߻��� ��� )  *******************/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Clear_Tx_Buffer(void)  // ���� ������ �ʱ�ȭ �Լ�
{
        unsigned char  i= 0;

        for( i = 0 ; i < 92 ; i++)  
        {
                U2_Tx_Buffer[i] = 0x00;
        }
} 
