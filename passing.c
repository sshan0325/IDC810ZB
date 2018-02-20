/* Includes ------------------------------------------------------------------*/
#include "passing.h"
#include "usart.h"
#include "RF_KEY.h"
#include "subfunction.h"

/* Private variables ---------------------------------------------------------*/
//Seungshin Using
////////////////// UART CH 1 ////////////////////
extern unsigned char U1_Rx_Buffer[U1_RX_BUFFER_SIZE] ;
extern unsigned char    U1_Rx_DataPosition;
extern unsigned char    U1_Rx_Count ;
////////////////// UART CH 2 ////////////////////
extern unsigned char    U2_Rx_Buffer[U2_RX_BUFFER_SIZE]; 
unsigned char               U2_Tx_Buffer[128] = {0} ;  
extern unsigned char    U2_Rx_DataPosition;
extern unsigned char    U2_Rx_DataSavePosition;

///////////////// DEVICE STATE /////////////////
unsigned char Key_Reg_RQST_Flag = RESET;

///////////////// Flag /////////////////////////////
extern unsigned char RFKey_Detected;

//Need to Check
extern unsigned char U2_Rx_Compli_Flag ;
unsigned char Temp_buffer[17] ={0};
extern unsigned char U2_Rx_Count ;
unsigned char TX_CMD = 0x00 ;
unsigned char RF_Data_Confirm_Flag = RESET;
unsigned char Key_Reg_End_Flag = RESET;
unsigned char RF_DATA_RQST_Flag = RESET;
unsigned char Reg_Mode_Start_Flag = RESET;
extern unsigned char Key_Reg_End_Button_Flag ;
extern unsigned char RF_Key_CNT;
unsigned char Tx_LENGTH = 22;
extern unsigned char Reg_key_Value_Receive_Flag ;
unsigned char U1_Tx_Buffer[128]= {0};
unsigned char Key_Reg_Timeout_flag = RESET;
extern unsigned int Key_Reg_Timeout_CNT ;
extern unsigned char Reg_Fail_Flag;
extern unsigned char RF_Communi_Fail ;
unsigned char Key_Reg_U1_Send_Flag = RESET;
unsigned char Reg_Compli_Flag = RESET;
unsigned char Key_Save_Flag = RESET;
unsigned char U1_Paket_Type = 0x00;
unsigned char RF_Comm_Time_Out_Flag = RESET;
unsigned char Status_Value_Clear_Flag = RESET;
extern unsigned char RF_Key_Data[128];
extern unsigned char CNT ;
unsigned char CMD_Buffer[8] = { RF_STATUS_RQST , RF_STATUS_CLR_RQST , RF_DATA_RQST , RF_DATA_CONFIRM_RQST ,
                                                REG_MODE_START_RQST , REG_KEY_DATA_RQST , REG_MODE_END_RQST , EQUIP_INFOR_RQST};
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


// 10 ����Ʈ ���۽ð� 1.04 ms /

///////////////////////////////////////////////////////////////////////////////
/******************** ���е�  ��Ŷ ó��  �Լ� *******************/
//////////////////////////////////////////////////////////////////////////////
void Packet_handler(void)       
{
    while(U2_Rx_Count>2)
    {

        if(U2_Rx_Buffer[U2_Rx_DataPosition] == STX && (U2_Rx_Buffer[U2_Rx_DataPosition+1] == RF_Camera_ID))
        {
              if ( U2_Rx_Buffer[U2_Rx_DataPosition+2] <=  U2_Rx_Count )
              {
                  if( PacketValidation() == VALID) 
                  {
                      CMD();
                      Delay(12);                              //  idle time Delay 
                      Response();
                      #ifdef  U2_DATA_MONITOR_1
                      printf ("\r\nRx Data Count :  %d :    , Data Position : %d    , DataSavePosition : %d", U2_Rx_Count, U2_Rx_DataPosition, U2_Rx_DataSavePosition);
                      #endif
                  }
                  
                  U2_Rx_Count-=U2_Rx_Buffer[U2_Rx_DataPosition+2];
                  U2_Rx_DataPosition+=U2_Rx_Buffer[U2_Rx_DataPosition+2];
              }
              
              else 
              {
                  break;
              }                              
        }
        else
        {
            U2_Rx_Count--;
            U2_Rx_DataPosition++;       
        }
    }        
}


////////////////////////////////////////////////////////////////////////////////
/******************** ���е�  ��Ŷ �˻� �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////
unsigned char PacketValidation(void)   
{
    unsigned char Result=0;
    unsigned char Rx_Length=0;
                
    Rx_Length = U2_Rx_Buffer[U2_Rx_DataPosition+2];

    for(unsigned char i = 0 ; i < 8 ; i ++)
    {
        if(U2_Rx_Buffer[U2_Rx_DataPosition+3] == CMD_Buffer[i])  
        {
            Result ++; 
        }
    }
     
    if( U2_Rx_Buffer[U2_Rx_DataPosition+(Rx_Length-1)] == Check_Checksum())        
    {
        Result ++; 
    }
    else
    {
    }

    if(Result != VALID )
    {
          #ifdef DataValication_Check_LOG 
          printf ("\r\n[System                ] U2_Rx_Data is Invalid!!!  Result : %d      ", Result);
          for (unsigned char tmp=0 ; tmp<U2_Rx_Buffer[U2_Rx_DataPosition+2] ; tmp++)
          {
            printf ("%x  ",U2_Rx_Buffer[U2_Rx_DataPosition+tmp]) ;
          }          
          printf ("\r\nCMD Code is : %x  ",U2_Rx_Buffer[U2_Rx_DataPosition+3]) ;          
          printf ("\r\nChecksum : %x / Received Data : %x", Check_Checksum(), U2_Rx_Buffer[U2_Rx_DataPosition+Rx_Length-1]) ;
          #endif                                   
    }
    else
    {
          #ifdef DataValication_Check_LOG 
          printf ("\r\n[System                ] U2_Rx_Data is Valid!!!  Result : %d      ", Result);
          for (unsigned char tmp=0 ; tmp<U2_Rx_Buffer[U2_Rx_DataPosition+2] ; tmp++)
          {
            printf ("%x  ",U2_Rx_Buffer[U2_Rx_DataPosition+tmp]) ;
          }          
          printf ("\r\nCMD Code is : %x  ",U2_Rx_Buffer[U2_Rx_DataPosition+3]) ;          
          printf ("\r\nChecksum : %x / Received Data : %x", Check_Checksum(), U2_Rx_Buffer[U2_Rx_DataPosition+Rx_Length-1]) ;
          #endif        
    }
    
    return Result;
 }


///////////////////////////////////////////////////////////////////////////
/******************** ��ɾ� �˻� �Լ� *******************/
//////////////////////////////////////////////////////////////////////////
unsigned char CMD_Check(unsigned char *CMD, unsigned char CNT)    
{
  char CMD_Check = 0;
  for(char i = 0 ; i < CNT ; i ++)
  {
      if(CMD[3] == CMD_Buffer[i])         CMD_Check++;
  }

  if(CMD_Check)
    return 1;
  else
    return 0;
}
//////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////
/******************** ���е�  ��Ŷ�� ���� ���� ó�� �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////////////
void Response(void)                                                          
{
    GPIO_WriteBit(GPIOB,  GPIO_Pin_0 , (BitAction) Bit_SET);  // 485 Trans pin Enable 

    U2_Tx_Buffer[0] = STX ;
    U2_Tx_Buffer[1] = RF_Camera_ID ;
    U2_Tx_Buffer[2] = Tx_LENGTH ;
    U2_Tx_Buffer[3] = TX_CMD;
    U2_Tx_Buffer[4] = U2_Rx_Buffer[U2_Rx_DataPosition+4] ;
    U2_Tx_Buffer[Tx_LENGTH-1] = Make_Checksum() ;

    /************* ���� RF ������ �ν� �� ������Ŷ ���� ��ƾ **************/
    if((RF_DATA_RQST_Flag == SET))
    {
          Tx_LENGTH = ( 16 * RF_Key_CNT ) + 7 ;
           for(unsigned char i = 6 ; i < ( Tx_LENGTH - 1 ) ; i ++ ) 
           {                                                                 // ��Ŷ ���� 23 üũ�� ������ ����  [23-1] = [22] ( üũ�� �ڸ�) (<) �̹Ƿ� üũ�� �ڸ��ٷ� ��!
               U2_Tx_Buffer[i] = RF_Key_Data[i-6] ;
           }
           U2_Tx_Buffer[2] = Tx_LENGTH;                            // Tx_LENGTH �������� �ְ� ���ۿ� ���� ������ ���� �ʵ�� ��
           U2_Tx_Buffer[Tx_LENGTH-1] = Make_Checksum();

           RF_DATA_RQST_Flag = RESET; // 15.05.20 ������.
           RF_Key_CNT = 0;
           Reg_key_Value_Receive_Flag = RESET;
    }
          
    /*************  ����ƮŰ ��Ͻ� ������Ŷ ���� ��ƾ  **************/          
    if((Reg_key_Value_Receive_Flag == SET))
   {
        Tx_LENGTH = ( 16 * RF_Key_CNT ) + 7 ;
        for(unsigned char i = 6 ; i < ( Tx_LENGTH - 1 ) ; i ++ )    // 22
        {                                                                              // ��Ŷ ���� 23 üũ�� ������ ����  [23-1] = [22] ( üũ�� �ڸ�) (<) �̹Ƿ� üũ�� �ڸ��ٷ� ��!
            U2_Tx_Buffer[i] = U1_Rx_Buffer[U1_Rx_DataPosition-RF_KEY_PACKET_SIZE+i-5] ;                            //16���� ����,  UART 1���� ������ ������ �ʵ� [0] �� ������ ����
        }
        U2_Tx_Buffer[2] = Tx_LENGTH;                                         // Tx_LENGTH �������� �ְ� ���ۿ� ���� ������ ���� �ʵ�� ��
        U2_Tx_Buffer[Tx_LENGTH-1] = Make_Checksum();

        RF_Key_CNT = 0;
        Reg_key_Value_Receive_Flag = RESET;
   }

   USART2_TX();

   #ifdef  U2_DATA_MONITOR_1
   printf ("\r\nU2  Tx Data(Length : %d) :", Tx_LENGTH);
   for (unsigned char tmp=0 ; tmp< Tx_LENGTH ; tmp ++)
   {
       printf ("%x  ", U2_Tx_Buffer[tmp]);
   }
   #endif      
          
          
   /*************  ���� ��Ŷ �۽� ��  ���� ó��  ��ƾ  **************/     
   if(Reg_Fail_Flag == SET)                 // ��ϸ�忡��  �̹� ��ϵ� Ű�� ��� �� �÷��� ������ ��Ʈ �缳��
  {
        Reg_Fail_Flag = RESET;
        U2_Tx_Buffer[5] &= 0xFB;
   }
   if( RF_Communi_Fail == SET)          // ��� ��忡��  ��� ���� �� �÷��� ������ ��Ʈ �缳��
   {
        RF_Communi_Fail = RESET;
        U2_Tx_Buffer[5] &= 0xF7;
   }

   U2_Rx_Compli_Flag = RESET;
   CNT = 0;
}




////////////////////////////////////////////////////////////////////////////////////
/******************** �� ��ɾ ����  �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////////
void CMD(void)
{
    unsigned char Requested_CMD;
    Requested_CMD = U2_Rx_Buffer[U2_Rx_DataPosition+3];

    switch(Requested_CMD)
    {
        /***************** 0x11 ���� �� ��û ****************/
        case RF_STATUS_RQST:  // 0x11 ���� �� ��û 
        { 
              Tx_LENGTH = 8 ; 
              TX_CMD = RF_STAUS_RSPN ;

               if(Key_Reg_RQST_Flag)                      // ��� ��� �� ��� �� ������ RF ��⸦ ��� ��� �����Ŵ
               {                                                     // �߰��� ����Ʈ��  ������ ���� �ٽ� ������ 
                      GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);  //  LED OFF
                      U1_Paket_Type = 0xA0;                // ��� ���� RF ��⿡ �˸�
                      USART1_TX();
               }         
               
              U1_Paket_Type = 0xD0;  
              U1_Tx_Buffer[1] = 0xD0;
              U1_Tx_Buffer[2] = U2_Rx_Buffer[U2_Rx_DataPosition+5];
              U1_Tx_Buffer[3] = U2_Rx_Buffer[U2_Rx_DataPosition+6];
              
              USART1_TX();

              RF_Comm_Time_Out_Flag = SET; //  Ÿ�� �ƿ�
               
               if((RF_Data_Confirm_Flag == SET) || (RF_DATA_RQST_Flag == SET)|| (Status_Value_Clear_Flag == SET) 
                  || (Reg_Mode_Start_Flag == SET) || (Key_Reg_RQST_Flag == SET) ||  (Key_Reg_End_Flag == SET) ) 
               {                                                         //  �� ��� ���� �� �ٽ� ���� ��Ŷ ���ö� ���� ��Ŷ ���� 
                     Clear_Tx_Buffer();

                     RF_Data_Confirm_Flag = RESET;             // �� ��ɿ� ���� �÷��� �ʱ�ȭ
                     Key_Reg_End_Flag = RESET;
                     Reg_Mode_Start_Flag = RESET;
                     Key_Reg_RQST_Flag = RESET;
                     Key_Reg_End_Flag = RESET;
                     Status_Value_Clear_Flag = RESET;
                     Key_Reg_U1_Send_Flag = RESET;             // ��� ���� �� �ٽ� ������ �ȵ� 
                     Reg_key_Value_Receive_Flag = RESET;
               } 

               RF_DATA_RQST_Flag = RESET;                        //��, ���̿� Ű�ν� �Ǹ� Ű���� �����°� ����
        }
        break;
        
        
        /***************** 0x12  ���°� ���� ��û ****************/      
        case RF_STATUS_CLR_RQST:  // 0x12  ���°� ���� ��û
        {
              Tx_LENGTH = 7 ; 

              Status_Value_Clear_Flag = SET;

              TX_CMD = RF_STAUS_CLR_RSPN;

              if((U2_Rx_Buffer[U2_Rx_DataPosition+5] & 0x80 ) == 0x80)  // ���� �� ���� ��Ŷ�� ���� ��Ʈ Ŭ���� 
              {
                      U2_Tx_Buffer[5] &= 0x7F;
              }
              
              if((U2_Rx_Buffer[U2_Rx_DataPosition+5] & 0x01 ) == 0x01)
              {
                       U2_Tx_Buffer[5] &= 0xFE;
               }
         }
        break;
        
        /***************** 0x21 RF ������ ��û  ****************/      
        case RF_DATA_RQST:      // 0x21 RF ������ ��û 
        {
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] RF Data is Requested.");     
              printf ("\r\nReceivce U2 Data :");
              for (unsigned char tmp=0 ; tmp< 7 ; tmp ++)
              {
                  printf ("%x  ", U2_Rx_Buffer[U2_Rx_DataPosition+tmp]);
              }              
              #endif                    
              RF_Key_CNT = U2_Rx_Buffer[U2_Rx_DataPosition+5];  // ��û�� ������ ��Ŷ ������ ������ ���� 
              
              TX_CMD = RF_DATA_RSPN; //  ���� ����Ʈ Ű �νĽ� 

              U2_Tx_Buffer[5] = RF_Key_CNT;
              
              RF_DATA_RQST_Flag = SET;

              if(RFKey_Detected == RESET)
              {
                    U2_Tx_Buffer[5] = 0x00;
                    RF_Key_CNT = 0;
                    Tx_LENGTH = 7;
                    RF_DATA_RQST_Flag = RESET;
              }
        }
        break;

        /***************** 0x22  ����ƮŰ ������ Ȯ��  ****************/      
        case RF_DATA_CONFIRM_RQST:  // 0x22  ����ƮŰ ������ Ȯ��
        {
                unsigned char KEY_Number_to_Confirm = 0;
                #ifdef Consol_LOG        
                printf ("\r\n[System                ] RF Data Confirm is Requested.");     
                #endif                    
          
                KEY_Number_to_Confirm = U2_Rx_Buffer[U2_Rx_DataPosition+5];                   // ��û ���� ����
                RF_Data_Confirm(KEY_Number_to_Confirm);                 // ���� ������ Ȯ�� �Լ�
                TX_CMD = RF_DATA_CONFIRM_RSPN ;                       
                U2_Tx_Buffer[5] = KEY_Number_to_Confirm;
                Tx_LENGTH = 9;
                RF_DATA_RQST_Flag = RESET;  
                RF_Data_Confirm_Flag = SET;
                RFKey_Detected = RESET;
         }
         break;
         
        /***************** 0x31  ����ƮŰ ��� ��� ����  ****************/  
        case REG_MODE_START_RQST:  // 0x31  ����ƮŰ ��� ��� ����
        {
                #ifdef Consol_LOG 
                printf ("\r\n[System                ] Regist mode Start Request");     
                #endif          
                TX_CMD = REG_MODE_START_RSPN ;    
                Key_Reg_RQST_Flag = SET;
                Tx_LENGTH = 7;
                
                RF_DATA_RQST_Flag = RESET;     //  ���� ��� ����  ��, ���̿� Ű�ν� �Ǹ� Ű���� �����°� ����
                Reg_Mode_Start_Flag = SET;
         }
         break;  
         
         
        /************************** 0x32  ����ƮŰ ��� ��û ********************************/          
        case REG_KEY_DATA_RQST:  // 0x32  ����ƮŰ ��� ��û
        {
               //#ifdef Consol_LOG        
               //printf ("\r\n[System                ] RF KEY Data is Requested.");     
               //#endif                    
               TX_CMD = REG_KEY_DATA_RSPN ;    
              
               Key_Reg_RQST_Flag = SET;
               Reg_Mode_Start_Flag = RESET;
               
               Key_Reg_Timeout_CNT =0; // ��� ��� Ÿ�Ӿƿ� �ʱ�ȭ 
               Key_Reg_Timeout_flag = SET;  // ��� ��� Ÿ�Ӿƿ� ����                
               
               GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_SET);  // �߰��� ����ī�޶� �������� ���� �ٽ� ������ ��ϸ�带 �����ϱ� ���ؼ� 
             
   
               if(Key_Info_Compare())                   // ���� Ű���� �ٸ��� 
               {
                      Key_Reg_U1_Send_Flag = RESET;
                      Reg_Compli_Flag = RESET;
                      Key_Save_Flag = RESET;
               }

               if(Key_Save_Flag == RESET)  // ���е忡�� ��� ���н� Ű���� ���ϱ����� �ӽ����� ��ƾ
               {
                       for(char i = 5 ; i < 14 ; i++)
                       {
                               Temp_buffer[i] = U2_Rx_Buffer[U2_Rx_DataPosition+i];
                       }
                       
                       Key_Save_Flag = SET;
               }

               if(Key_Reg_U1_Send_Flag == RESET) 
               {
                     Key_Reg_U1_Send_Flag = SET;
                     U1_Paket_Type = 0xC0;                    // ��� ��û RF ��⿡ �˸� 
                     USART1_TX();
                }
            
              if(Reg_key_Value_Receive_Flag == SET)  // ��� Ű�� �޾�����
              { 
                    if(Key_Reg_End_Button_Flag == RESET)             // ��� �����ư  �ȴ����� ��
                    {
                            RF_Key_CNT = 1;
                            Tx_LENGTH = 23;
                             
                            U2_Tx_Buffer[5] = 0x01;

                            Reg_Compli_Flag = SET; 
                     }

                    if(Key_Reg_End_Button_Flag == SET)             // ��� �����ư ���� �� ���� ��Ŷ ������!!
                    {
                          Reg_key_Value_Receive_Flag = RESET;

                          Tx_LENGTH = 7;
                          U2_Tx_Buffer[5] |= 0x02;
                         
                          Key_Reg_End_Button_Flag = RESET;  // -> Ŭ���� ���� �ٽ� ���ϱ� 
                    }
              } // END of  if (��� Ű�� �޾����� )

              if(Reg_key_Value_Receive_Flag == RESET)  // ��� Ű��  �ȹ޾����� 
              {
                     if((!(Key_Info_Compare())) && (Reg_Compli_Flag == SET))  // ��Ŷ ������ ������  ���� Ű���� ������ 
                     {
                            RF_Key_CNT = 1;
                            Tx_LENGTH = 23;
                             
                            U2_Tx_Buffer[5] = 0x01;
                            
                            Reg_key_Value_Receive_Flag = SET;
                     }

                     if(Reg_Compli_Flag == RESET)
                     {
                              if(Key_Reg_End_Button_Flag == RESET)              // ��� �����ư  �ȴ����� ��
                              {
                                    Tx_LENGTH = 7;
                                    U2_Tx_Buffer[5] &= 0xFC;
                              }
                              
                              if(Key_Reg_End_Button_Flag == SET)             // ��� �����ư ���� ��
                              {
                                   Tx_LENGTH = 7;
                                   U2_Tx_Buffer[5] |= 0x02;
                                    Key_Reg_End_Button_Flag = RESET;
                              }
                       }
              }  // END of if (��� Ű�� �ȹ޾����� )
         }// END of case
         break;  

        /*************************** 0x33  ����ƮŰ ��� ��� ����  ********************************/
        case REG_MODE_END_RQST:  // 0x33  ����ƮŰ ��� ��� ���� 
        {
               #ifdef Consol_LOG        
               printf ("\r\n[System                ] RF Key Registrration Mode Stop is Requested.");     
               #endif                    
               TX_CMD = REG_MODE_END_RSPN ;    
               Tx_LENGTH = 7;
               U2_Tx_Buffer[5] = 0x01;
                RF_DATA_RQST_Flag = RESET;     //  ���� ��� ����  ���� Ű�ν� �Ǹ� Ű���� �����°� ���� 

                Key_Reg_End_Flag = SET;
                Key_Reg_RQST_Flag = RESET;
                Reg_Mode_Start_Flag = RESET;
              
                BuzzerRun(100, 1,80,10);

               GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);  //  LED OFF
               
                for(char i = 5 ; i < 14 ; i++)
                {
                        Temp_buffer[i] = 0;
                }  
               
               U1_Paket_Type = 0xA0;  // ��� ���� RF ��⿡ �˸�
               USART1_TX();
         }
         break;  
         
         /*************************** 0x01 ��� ���� ��û ********************************/
        case EQUIP_INFOR_RQST:              // 0x01 ��� ���� ��û
        {
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] Device Information Request.");     
              #endif
              TX_CMD = EQUIP_INFOR_RSPN ; 
              Tx_LENGTH = 15;

              U2_Tx_Buffer[5] = 0x00;
              U2_Tx_Buffer[6] = 0x00;
              U2_Tx_Buffer[7] = 0x01;
              U2_Tx_Buffer[8] = 0x0D;
              U2_Tx_Buffer[9] = 0x0C;
              U2_Tx_Buffer[10] = 0x04;
              U2_Tx_Buffer[11] = 0x01; 
              U2_Tx_Buffer[12] = 0x00;
              U2_Tx_Buffer[13] = 0x60;
        }
        break;
     } //  END of switch 
} // END of CMD function
  


                  
/******************** ���� Ű ���� �� �Լ� *******************/
unsigned char Key_Info_Compare(void)            //  
{
      char Compare_CNT = 0;
       for(char i = 5 ; i < 14 ; i++)
       {
               if(Temp_buffer[i] == U2_Rx_Buffer[U2_Rx_DataPosition+i])
                 Compare_CNT ++;
       }  
      if(Compare_CNT == 9)       
        return 0 ;    
      else                              
        return 1 ;
}
                  
