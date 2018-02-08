/* Includes ------------------------------------------------------------------*/
#include "passing.h"
#include "usart.h"
#include "RF_KEY.h"
#include "subfunction.h"

/* Private variables ---------------------------------------------------------*/
//Seungshin Using
/* UART  Ch1-------------------------------------------------------------*/
extern unsigned char U1_Rx_Buffer[U1_RX_BUFFER_SIZE] ;



////////////////// UART Ch1 ///////////////////////
extern unsigned char    U1_Rx_DataPosition;
////////////////// UART Ch2 ///////////////////////
extern unsigned char    U2_Rx_Buffer[128]; 
extern unsigned char    U2_Tx_Buffer[128] ;
///////////////// DEVICE STATE /////////////////
unsigned char Key_Reg_RQST_Flag = RESET;

extern unsigned char RF_Key_Data[128];


//Have to Check
unsigned char Call_Button_Flag ;

//FLAG
unsigned char Cognition_Disable_Flag = RESET;
unsigned char RF_Data_Confirm_Flag = RESET;
unsigned char Key_Reg_End_Flag = RESET;
unsigned char RF_DATA_RQST_Flag = RESET;
unsigned char Reg_Mode_Start_Flag = RESET;
extern unsigned char Key_Reg_End_Button_Flag ;
extern unsigned char Rx_Compli_Flag ;
extern unsigned char Reg_key_Value_Receive_Flag ;
extern unsigned char Usual_RF_Detec_Flag;
unsigned char Key_Reg_Timeout_flag = RESET;
extern unsigned char Reg_Fail_Flag;
unsigned char Reg_Fail_Bit_Clear_Flag = RESET;
unsigned char Key_Reg_U1_Send_Flag = RESET;
unsigned char Reg_Compli_Flag = RESET;
unsigned char Key_Save_Flag = RESET;
unsigned char U1_Tx_Flag =  RESET;
extern unsigned char U1_Rx_Flag ;
unsigned char Status_Value_Clear_Flag = RESET;
unsigned char Device_Info_Flag = RESET;
unsigned char Watch_Dog_init_Flag = SET;
extern unsigned char Time_Out_Flag_CNT;
extern unsigned char RF_Key_Detec_CNT_Flag ;
unsigned char Time_Out_Flag = RESET;
//Need to Check
unsigned char Temp_buffer[17] ={0};
extern unsigned char Rx_Count ;
unsigned char TX_CMD = 0x00 ;
extern unsigned char RF_Key_CNT;
extern unsigned char U1_Rx_Count ;
unsigned char Tx_LENGTH = 22;
unsigned char Rx_LENGTH = 0x00;
unsigned char KEY_Number_to_Confirm = 0;
extern unsigned int Key_Reg_Timeout_CNT ;
extern unsigned char RF_Communi_Fail ;
unsigned char U1_Paket_Type = 0x00;
extern unsigned char g_WatchdogEvent;
extern unsigned int Watch_Dog_Flag_CNT;
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
          if(Rx_Compli_Flag == SET)
          {
                Rx_LENGTH = U2_Rx_Buffer[2];
                //Passing();
                if(Watch_Dog_init_Flag)                 //  �ʱ� ��� ���۽� 1ȸ ��ġ�� ���� 
                {
                    Watch_Dog_init_Flag = RESET;
                    WatchDog_Init(); 
                }

                if( PacketValidation() == VALID) 
                {
                   CMD();
                   if((RF_Key_Detec_CNT_Flag))  // ���� Ű �νĽ� 
                    {
                          Delay(12);                              //  idle time Delay 
                          Response();
                          RF_Key_Detec_CNT_Flag = RESET;
                    }
                    
                    else if((Status_Value_Clear_Flag)||(Reg_Mode_Start_Flag)||(Key_Reg_RQST_Flag)||(Key_Reg_End_Flag)||
                      (RF_DATA_RQST_Flag)||(RF_Data_Confirm_Flag)|| (Device_Info_Flag) ) // �� �ܿ� ��ɵ�
                    {  
                          Delay(12);                              //  idle time Delay 
                          Response();
                          Device_Info_Flag = RESET;                            
                    }
                   
                   else
                   {
                          #ifdef Consol_LOG        
                          //printf ("\r\n[System                ] No Response case is Existed\r\n");     
                          #endif                     
                   }
                }
          }
}


////////////////////////////////////////////////////////////////////////////////
/******************** ���е�  ��Ŷ �˻� �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////
unsigned char PacketValidation(void)   
{
    unsigned char Result=0;
    
    if( CMD_Check( U2_Rx_Buffer, sizeof(CMD_Buffer)/sizeof(unsigned char)))    
        Result ++; 
     
    if( U2_Rx_Buffer[(Rx_LENGTH-1)] == Check_Checksum())        
        Result ++; 

    if(Result != VALID )
    {
          Rx_Count =0;
          Rx_Compli_Flag = RESET ;
          for(unsigned char i = 0; i<92 ; i++ )
          {
              U2_Rx_Buffer[i] = 0;
              U2_Tx_Buffer[i] = 0;
           }
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


///////////////////////////////////////////////////////////////////////////
/******************** üũ�� �˻� �Լ� *******************/
//////////////////////////////////////////////////////////////////////////
unsigned char Check_Checksum(void)                      // 
{
  
      unsigned char Checksum = 0x02;
      
      for(unsigned char i = 1 ; i< (Rx_LENGTH -1) ; i++)
      {        
        Checksum ^= U2_Rx_Buffer[i];
        Checksum ++;
        
      }
     
      
        
      return Checksum;

  
}


///////////////////////////////////////////////////////////////////////////
/******************** üũ�� ����� �Լ� *******************/
//////////////////////////////////////////////////////////////////////////
unsigned char Make_Checksum(void)                       //  
{
  
      unsigned char Checksum = 0x02;
      
      for(unsigned int i = 1 ; i< (Tx_LENGTH - 1) ; i++)
      {
         Checksum ^= U2_Tx_Buffer[i];
         Checksum ++;
      }
      
   
    
       return Checksum;
  
}


////////////////////////////////////////////////////////////////////////////////////////
/******************** ���е�  ��Ŷ�� ���� ���� ó�� �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////////////
void Response(void)                                                          
{
    RS485TX_ENABLE; 
          
    U2_Tx_Buffer[0] = STX ;
    U2_Tx_Buffer[1] = RF_Camera_ID ;
    U2_Tx_Buffer[2] = Tx_LENGTH ;
    U2_Tx_Buffer[3] = TX_CMD;
    U2_Tx_Buffer[4] = U2_Rx_Buffer[4] ;
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
    
     U1_Tx_Flag= RESET;
     Rx_Compli_Flag = RESET;
     CNT = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** ���е� ��Ŷ�� ���� ���� ��Ŷ �۽�  �Լ�  *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////
void USART2_TX(void)            //���� ī�޶� -> ���е� ���� �Լ� 
{
      for(unsigned char i = 0 ; i < Tx_LENGTH ; i++)
      {
           USART_SendData(USART2, U2_Tx_Buffer[i]);  
           while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); // wait for trans
           
           #ifdef Consol_LOG 
           if (i==5 && Reg_key_Value_Receive_Flag != SET && RF_DATA_RQST_Flag != SET && U2_Tx_Buffer[i]!=0)
           {
              if ( (U2_Tx_Buffer[i]&0x80) == 0x80 )
                printf ("\r\n[System                ] RF Communication Error\r\n");     
              if ( (U2_Tx_Buffer[i]&0x01) == 0x01 )
                printf ("\r\n[System                ] call Buttom is Pushed and shared to WallPAD\r\n");     
           }
           #endif                
      }
      

     
      g_WatchdogEvent = SET;   // ��Žÿ� ��� �̺�Ʈ ���� 
      Watch_Dog_Flag_CNT = 0; // 
       
      RS485TX_DISABLE; 
      USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);     
 
}


////////////////////////////////////////////////////////////////////////////////////
/******************** �� ��ɾ ����  �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////////

void CMD(void)
{
    unsigned char U1_Tx_Buffer[128]= {0};
     
    switch(U2_Rx_Buffer[3])
    {
        /***************** 0x11 ���� �� ��û ****************/
        case RF_STATUS_RQST:  // 0x11 ���� �� ��û 
        { 
              //#ifdef Consol_LOG        
              //printf ("\r\n[System                ] RF Status is requested \r\n");     
              //#endif
              
              Tx_LENGTH = 8 ; 
              TX_CMD = RF_STAUS_RSPN ;
                    
              if((RF_Data_Confirm_Flag == SET) || (RF_DATA_RQST_Flag == SET)|| (Status_Value_Clear_Flag == SET) 
                || (Reg_Mode_Start_Flag == SET) || (Key_Reg_RQST_Flag == SET) ||  (Key_Reg_End_Flag == SET) ) 
              {                                                         //  �� ��� ���� �� �ٽ� ���� ��Ŷ ���ö� ���� ��Ŷ ���� 
                     Clear_Tx_Buffer();
                     if(Call_Button_Flag == SET)                // RF ��� �̾��� �� ���� Ŭ����� ���� ȣ�� �ȵǴ� ���� ����
                     {
                            Call_Button_Flag = RESET;
                            U2_Tx_Buffer[5] |= 0x01;
                     }
                    
                     if(Key_Reg_RQST_Flag)                     // ��� ��� �� ��� �� ������ RF ��⸦ ��� ��� �����Ŵ
                     {                                                     // �߰��� ����Ʈ��  ������ ���� �ٽ� ������ 
                            GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);  //  LED OFF
                            U1_Paket_Type = 0xA0;                // ��� ���� RF ��⿡ �˸�
                            USART1_TX();
                     }

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
    
              if(U1_Tx_Flag == RESET)                             // RF ��� ������ ���ι� ������1���� ������ �ϴ� ��ƾ
              {
                        U1_Tx_Flag = SET;
                        U1_Paket_Type = 0xD0;  
                        U1_Tx_Buffer[1] = 0xD0;
                        U1_Tx_Buffer[2] = U2_Rx_Buffer[5];
                        U1_Tx_Buffer[3] = U2_Rx_Buffer[6];
    
                        USART1_TX();
             
                        Time_Out_Flag = SET; //  Ÿ�� �ƿ�
               }
        }
        break;
        
        
        /***************** 0x12  ���°� ���� ��û ****************/      
        case RF_STATUS_CLR_RQST:  // 0x12  ���°� ���� ��û
        {
              //#ifdef Consol_LOG        
              //printf ("\r\n[System                ] RF Status clear is Requested \r\n");     
              //#endif          
              Tx_LENGTH = 7 ; 

              Status_Value_Clear_Flag = SET;

              TX_CMD = RF_STAUS_CLR_RSPN;

              if((U2_Rx_Buffer[5] & 0x80 ) == 0x80)  // ���� �� ���� ��Ŷ�� ���� ��Ʈ Ŭ���� 
              {
                      U2_Tx_Buffer[5] &= 0x7F;
              }
              
              if((U2_Rx_Buffer[5] & 0x01 ) == 0x01)
              {
                       U2_Tx_Buffer[5] &= 0xFE;
               }
         }
        break;
        
        /***************** 0x21 RF ������ ��û  ****************/      
        case RF_DATA_RQST:      // 0x21 RF ������ ��û 
        {
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] RF Data is Requested \r\n");     
              #endif                    
              RF_Key_CNT = U2_Rx_Buffer[5];  // ��û�� ������ ��Ŷ ������ ������ ���� 
              
              TX_CMD = RF_DATA_RSPN; //  ���� ����Ʈ Ű �νĽ� 

              U2_Tx_Buffer[5] = RF_Key_CNT;
              
              RF_DATA_RQST_Flag = SET;

              if(Usual_RF_Detec_Flag == RESET)
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
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] RF Data Confirm is Requested \r\n");     
              #endif                    
              U1_Rx_Count = 0;
              KEY_Number_to_Confirm = U2_Rx_Buffer[5];                   // ��û ���� ����
              
          
                
              RF_Data_Confirm(KEY_Number_to_Confirm);                 // ���� ������ Ȯ�� �Լ�
                
              TX_CMD = RF_DATA_CONFIRM_RSPN ;                       
           
          
              U2_Tx_Buffer[5] = KEY_Number_to_Confirm;
              Tx_LENGTH = 9;
                
               RF_DATA_RQST_Flag = RESET;  
               
               RF_Data_Confirm_Flag = SET;
               
               Usual_RF_Detec_Flag = RESET;
               
           
         }
         break;
         
        /***************** 0x31  ����ƮŰ ��� ��� ����  ****************/  
        case REG_MODE_START_RQST:  // 0x31  ����ƮŰ ��� ��� ����
        {
                #ifdef Consol_LOG 
                printf ("\r\n[System                ] Regist mode Start Request\r\n");     
                #endif          
                TX_CMD = REG_MODE_START_RSPN ;    
               
                Key_Reg_RQST_Flag = SET;
                Tx_LENGTH = 7;
                
                if(Call_Button_Flag == SET)                     // �߰��� ȣ�� ��ư ���� ����
                {
                    U2_Tx_Buffer[5] = 0x00;
                    Call_Button_Flag = RESET;
                }
                 
                RF_DATA_RQST_Flag = RESET;     //  ���� ��� ����  ��, ���̿� Ű�ν� �Ǹ� Ű���� �����°� ����
                Reg_Mode_Start_Flag = SET;
                
                U1_Rx_Count = 0;                        // ��� ��� ���� ��, Ű �ν� �Ǹ�  U1_Rx_Count �����Ǽ� CA�� BA�� ���ް� ��
         }
         break;  
         
         
        /************************** 0x32  ����ƮŰ ��� ��û ********************************/          
        case REG_KEY_DATA_RQST:  // 0x32  ����ƮŰ ��� ��û
        {
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] RF KEY Data is Requested \r\n");     
              #endif                    
              TX_CMD = REG_KEY_DATA_RSPN ;    
              
              Key_Reg_RQST_Flag = SET;
              Reg_Mode_Start_Flag = RESET;
               
              Key_Reg_Timeout_flag = RESET;
              Key_Reg_Timeout_CNT =0; // ��� ��� Ÿ�Ӿƿ� �ʱ�ȭ 

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
                              Temp_buffer[i] = U2_Rx_Buffer[i];
                      }
                      
                      Key_Save_Flag = SET;
              }

              if(Key_Reg_U1_Send_Flag == RESET) 
              {
                   Key_Reg_U1_Send_Flag = SET;
                    U1_Paket_Type = 0xC0;                    // ��� ��û RF ��⿡ �˸� 
                    USART1_TX();
               }
            
              U1_Rx_Count = 0 ;  // ��� Ű�� ���� ���� ���� Ű�� ������ �� ���� 

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
               Key_Reg_Timeout_flag = SET;  // ��� ��� Ÿ�Ӿƿ� ���� 
             
         }// END of case
         break;  

        /*************************** 0x33  ����ƮŰ ��� ��� ����  ********************************/
        case REG_MODE_END_RQST:  // 0x33  ����ƮŰ ��� ��� ���� 
        {
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] RF Status clear is Requested \r\n");     
              #endif                    
              TX_CMD = REG_MODE_END_RSPN ;    
              Tx_LENGTH = 7;
              U2_Tx_Buffer[5] = 0x01;
              RF_DATA_RQST_Flag = RESET;     //  ���� ��� ����  ���� Ű�ν� �Ǹ� Ű���� �����°� ���� 

               Key_Reg_End_Flag = SET;
               Key_Reg_RQST_Flag = RESET;
               Reg_Mode_Start_Flag = RESET;
              
               TIM_SetCompare1(TIM3,40);
               Delay(80);
               TIM_SetCompare1(TIM3,0); 

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
              printf ("\r\n[System                ] Device Information Request \r\n");     
              #endif
              TX_CMD = EQUIP_INFOR_RSPN ; 
              
              Tx_LENGTH = 15;
              
              Device_Info_Flag = SET;
              
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
  

    int count=0;

///////////////////////////////////////////////////////////////////////////////////////////
/******************** RF ���� ��Ŷ �۽� �ϴ�  �Լ� *******************/
//////////////////////////////////////////////////////////////////////////////////////////
void USART1_TX(void)
{
    int tmp=0;

    unsigned char U1_Tx_Buffer[128]= {0};
    
    U1_Tx_Buffer[0] = U1_Paket_Type;
    U1_Tx_Buffer[6] = 0x96; //U2_Rx_Buffer[5];
    U1_Tx_Buffer[7] = 0x97; //U2_Rx_Buffer[6];
#if 1    
    U1_Tx_Buffer[8] = U2_Rx_Buffer[7];
#else    
    if (count==0)
    {
      U1_Tx_Buffer[8] = 0x05; //U2_Rx_Buffer[7];
      count++;
    }
    else if(count==1)
    {
      U1_Tx_Buffer[8] = 0x7f; //U2_Rx_Buffer[7];
      count++;
    }
    else
    {
      U1_Tx_Buffer[8] = 0x81; //U2_Rx_Buffer[7];
      count=0;
    }
#endif    
    U1_Tx_Buffer[9] = U2_Rx_Buffer[8]; // site code
    U1_Tx_Buffer[10] = U2_Rx_Buffer[9];
    U1_Tx_Buffer[11] = U2_Rx_Buffer[10];
    U1_Tx_Buffer[12] = U2_Rx_Buffer[11];
    U1_Tx_Buffer[13] = U2_Rx_Buffer[12];  // dong , ho
    U1_Tx_Buffer[14] = U2_Rx_Buffer[13];  // key no
    U1_Tx_Buffer[15] = 0x00;              // key type
    U1_Tx_Buffer[16] = 0x00;              // dummy

    for(unsigned char i = 0 ; i < 17 ; i++)
    {
         USART_SendData(USART1,U1_Tx_Buffer[i]);  
          
         while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); // wait for trans
    }

#if 1
        #ifdef Consol_LOG 
        printf ("\r\n");
        printf ("[CAM -> RF]  : ") ;      
        for (tmp=0 ; tmp<17 ; tmp++)
        {
          printf ("%x, ",U1_Tx_Buffer[tmp]) ;
        }
        printf ("\r\n");
        #endif
#endif  
}
       
                  
/******************** ���� Ű ���� �� �Լ� *******************/
unsigned char Key_Info_Compare(void)            //  
{
      char Compare_CNT = 0;
       for(char i = 5 ; i < 14 ; i++)
       {
               if(Temp_buffer[i] == U2_Rx_Buffer[i])
                 Compare_CNT ++;
       }  
      if(Compare_CNT == 9)       
        return 0 ;    
      else                              
        return 1 ;
}
                  
