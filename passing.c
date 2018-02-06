/* Includes ------------------------------------------------------------------*/
#include "passing.h"
#include "usart.h"
#include "RF_KEY.h"
#include "subfunction.h"

/* Private variables ---------------------------------------------------------*/
//Seungshin Using
/* UART  Ch1-------------------------------------------------------------*/
extern unsigned char U1_Rx_Buffer[U1_RX_BUFFER_SIZE] ;



///////////////// UART /////////////////
extern unsigned char    U1_Rx_DataPosition;

//Have to Check
unsigned char Call_Button_Flag ;
//Need to Check
extern unsigned char Rx_Buffer[128]; 
extern unsigned char Rx_Compli_Flag ;
unsigned char Temp_buffer[17] ={0};

unsigned char Tx_Buffer[128] = {0} ;  
unsigned char Data_Check = 0;

extern unsigned char Rx_Count ;


unsigned char TX_CMD = 0x00 ;


extern unsigned char U1_Rx_Flag ;

unsigned char Cognition_Disable_Flag = RESET;
unsigned char Key_Reg_RQST_Flag = RESET;
unsigned char RF_Data_Confirm_Flag = RESET;
unsigned char Key_Reg_End_Flag = RESET;
unsigned char RF_DATA_RQST_Flag = RESET;
unsigned char Reg_Mode_Start_Flag = RESET;

extern unsigned char Key_Reg_End_Button_Flag ;
extern unsigned char RF_Key_CNT;

extern unsigned char U1_Rx_Count ;
unsigned char Tx_LENGTH = 22;

unsigned char CMD_Buffer[8] = { RF_STATUS_RQST , RF_STATUS_CLR_RQST , RF_DATA_RQST , RF_DATA_CONFIRM_RQST ,
                                                REG_MODE_START_RQST , REG_KEY_DATA_RQST , REG_MODE_END_RQST , EQUIP_INFOR_RQST};
unsigned char Rx_LENGTH = 0x00;
unsigned char KEY_Number_to_Confirm = 0;
extern unsigned char Reg_key_Value_Receive_Flag ;
extern unsigned char Usual_RF_Detec_Flag;
unsigned char U1_Tx_Buffer[128]= {0};
unsigned char Key_Reg_Timeout_flag = RESET;
extern unsigned int Key_Reg_Timeout_CNT ;


extern unsigned char Reg_Fail_Flag;
extern unsigned char RF_Communi_Fail ;
unsigned char Reg_Fail_Bit_Clear_Flag = RESET;

unsigned char Key_Reg_U1_Send_Flag = RESET;

unsigned char Reg_Compli_Flag = RESET;
unsigned char Key_Save_Flag = RESET;

unsigned char U1_Paket_Type = 0x00;
extern unsigned char RF_Key_Detec_CNT_Flag ;
unsigned char Time_Out_Flag = RESET;

unsigned char Status_Value_Clear_Flag = RESET;
extern unsigned char U1_71_Buffer[128];
unsigned char U1_Tx_Flag =  RESET;
unsigned char Device_Info_Flag = RESET;

extern unsigned char g_WatchdogEvent;

unsigned char Watch_Dog_init_Flag = SET;
extern unsigned int Watch_Dog_Flag_CNT;
extern unsigned char CNT ;
extern unsigned char Time_Out_Flag_CNT;





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
                Rx_LENGTH = Rx_Buffer[2];
                Passing();
                if(Watch_Dog_init_Flag)                 //  �ʱ� ��� ���۽� 1ȸ ��ġ�� ���� 
                {
                    Watch_Dog_init_Flag = RESET;
                    WatchDog_Init(); 
                }

                if(Data_Check == 2) 
                {
                   Data_Check = 0;
                   CMD();
                   if((RF_Key_Detec_CNT_Flag))  // ���� Ű �νĽ� 
                    {
                          RF_Key_Detec_CNT_Flag = RESET;
                          Delay(12);                              //  idle time Delay 
                          Response();
                    }
                    
                    if((Status_Value_Clear_Flag)||(Reg_Mode_Start_Flag)||(Key_Reg_RQST_Flag)||(Key_Reg_End_Flag)||
                      (RF_DATA_RQST_Flag)||(RF_Data_Confirm_Flag)|| (Device_Info_Flag) ) // �� �ܿ� ��ɵ�
                    {  
                          Device_Info_Flag = RESET;  
                          Delay(12);                              //  idle time Delay 
                        Response();
                    }
                }
                
          }
          
}


////////////////////////////////////////////////////////////////////////////////
/******************** ���е�  ��Ŷ �˻� �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////
void Passing(void)   
{

          if( CMD_Check( Rx_Buffer, sizeof(CMD_Buffer)/sizeof(unsigned char)))    
                                                                                            {   Data_Check ++; }
           
           
          if( Rx_Buffer[(Rx_LENGTH-1)] == Check_Checksum())        {   Data_Check ++; }
           
   
          
          if(Data_Check != 2 )
          {
                        
                        
                  Data_Check = 0;
                
                  Rx_Count =0;
                  Rx_Compli_Flag = RESET ;
     
                            
                  for(unsigned char i = 0; i<92 ; i++ )
                  {
                    Rx_Buffer[i] = 0;
                    Tx_Buffer[i] = 0;
                  }
           }
 
} // end of passing 


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
        Checksum ^= Rx_Buffer[i];
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
         Checksum ^= Tx_Buffer[i];
         Checksum ++;
      }
      
   
    
       return Checksum;
  
}


////////////////////////////////////////////////////////////////////////////////////////
/******************** ���е�  ��Ŷ�� ���� ���� ó�� �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////////////
void Response(void)                                                          
{
  int tmp=0;
      GPIO_WriteBit(GPIOB,  GPIO_Pin_0 , (BitAction) Bit_SET);  // 485 Trans pin Enable 
          
                Tx_Buffer[0] = STX ;
                Tx_Buffer[1] = RF_Camera_ID ;
                Tx_Buffer[2] = Tx_LENGTH ;
                Tx_Buffer[3] = TX_CMD;
                Tx_Buffer[4] = Rx_Buffer[4] ;
            
                Tx_Buffer[Tx_LENGTH-1] = Make_Checksum() ;
          
          
    /************* ���� RF ������ �ν� �� ������Ŷ ���� ��ƾ **************/
          if((RF_DATA_RQST_Flag == SET))
          {
            
                Tx_LENGTH = ( 16 * RF_Key_CNT ) + 7 ;
                
       
                  for(unsigned char i = 6 ; i < ( Tx_LENGTH - 1 ) ; i ++ ) 
                  {                                                                 // ��Ŷ ���� 23 üũ�� ������ ����  [23-1] = [22] ( üũ�� �ڸ�) (<) �̹Ƿ� üũ�� �ڸ��ٷ� ��!
                      Tx_Buffer[i] = U1_71_Buffer[i-6] ;
                  }
                  
                  Tx_Buffer[2] = Tx_LENGTH;                            // Tx_LENGTH �������� �ְ� ���ۿ� ���� ������ ���� �ʵ�� ��
                  
                  Tx_Buffer[Tx_LENGTH-1] = Make_Checksum();
             

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
                    Tx_Buffer[i] = U1_Rx_Buffer[U1_Rx_DataPosition-RF_KEY_PACKET_SIZE+i-5] ;                            //16���� ����,  UART 1���� ������ ������ �ʵ� [0] �� ������ ����
                }

                Tx_Buffer[2] = Tx_LENGTH;                                         // Tx_LENGTH �������� �ְ� ���ۿ� ���� ������ ���� �ʵ�� ��
                
                Tx_Buffer[Tx_LENGTH-1] = Make_Checksum();

                        
             
                RF_Key_CNT = 0;
                       
                Reg_key_Value_Receive_Flag = RESET;
                
#if 1                  
                printf ("\r\n");
                printf ("RF Key CNT : %d \r\n ",RF_Key_CNT) ;
                printf ("Tx Length : %d \r\n ",Tx_LENGTH) ;
                
                for (tmp=U1_Rx_DataPosition-RF_KEY_PACKET_SIZE ; tmp<U1_Rx_DataPosition-RF_KEY_PACKET_SIZE+17 ; tmp++)
                {
                  printf ("%x, ",U1_Rx_Buffer[tmp]) ;
                }
                printf ("\r\n");                  
                for (tmp=0 ; tmp<Tx_LENGTH ; tmp++)
                {
                  printf ("%x, ",Tx_Buffer[tmp]) ;
                }
#endif                       
          }
  
          
          
          USART2_TX();
          
          
          
      /*************  ���� ��Ŷ �۽� ��  ���� ó��  ��ƾ  **************/     
          if(Reg_Fail_Flag == SET)                 // ��ϸ�忡��  �̹� ��ϵ� Ű�� ��� �� �÷��� ������ ��Ʈ �缳��
          {
                Reg_Fail_Flag = RESET;
            
                Tx_Buffer[5] &= 0xFB;
           }
    
           if( RF_Communi_Fail == SET)          // ��� ��忡��  ��� ���� �� �÷��� ������ ��Ʈ �缳��
           {
                RF_Communi_Fail = RESET;
              
                 Tx_Buffer[5] &= 0xF7;
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
           USART_SendData(USART2,Tx_Buffer[i]);  
           while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); // wait for trans
      }

        g_WatchdogEvent = SET;   // ��Žÿ� ��� �̺�Ʈ ���� 
        Watch_Dog_Flag_CNT = 0; // 
        
        GPIO_WriteBit(GPIOB,  GPIO_Pin_0 , (BitAction) Bit_RESET);  // Receive Pin Enable
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);     
 
}


////////////////////////////////////////////////////////////////////////////////////
/******************** �� ��ɾ ����  �Լ�  *******************/
///////////////////////////////////////////////////////////////////////////////////

void CMD(void)
{

        switch(Rx_Buffer[3])
        {
          
          /***************** 0x11 ���� �� ��û ****************/
                case RF_STATUS_RQST:  // 0x11 ���� �� ��û 
                { 
                  
                      Tx_LENGTH = 8 ; 
                      TX_CMD = RF_STAUS_RSPN ;
         
                    
                               
                                 
                      if((RF_Data_Confirm_Flag == SET) || (RF_DATA_RQST_Flag == SET)|| (Status_Value_Clear_Flag == SET) 
                        || (Reg_Mode_Start_Flag == SET) || (Key_Reg_RQST_Flag == SET) ||  (Key_Reg_End_Flag == SET) ) 
                      {                                                         //  �� ��� ���� �� �ٽ� ���� ��Ŷ ���ö� ���� ��Ŷ ���� 
   
                             Clear_Tx_Buffer();
                            
                             if(Call_Button_Flag == SET)                // RF ��� �̾��� �� ���� Ŭ����� ���� ȣ�� �ȵǴ� ���� ����
                             {
                                    Call_Button_Flag = RESET;
                                    Tx_Buffer[5] |= 0x01;
                             }
                            
                             if(Key_Reg_RQST_Flag)                      // ��� ��� �� ��� �� ������ RF ��⸦ ��� ��� �����Ŵ
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
                                 U1_Tx_Buffer[2] = Rx_Buffer[5];
                                 U1_Tx_Buffer[3] = Rx_Buffer[6];
            
                                 USART1_TX();
                     
                                Time_Out_Flag = SET; //  Ÿ�� �ƿ�
                     
                                
                       }
                       
             

                }
                break;
                
                
          /***************** 0x12  ���°� ���� ��û ****************/      
                case RF_STATUS_CLR_RQST:  // 0x12  ���°� ���� ��û
                {
                  
                      Tx_LENGTH = 7 ; 
          
                      Status_Value_Clear_Flag = SET;
                      
             
                      
                      TX_CMD = RF_STAUS_CLR_RSPN;
          
    
                      
                      if((Rx_Buffer[5] & 0x80 ) == 0x80)  // ���� �� ���� ��Ŷ�� ���� ��Ʈ Ŭ���� 
                      {
                        
                              Tx_Buffer[5] &= 0x7F;
                              
                      }
                      
                      if((Rx_Buffer[5] & 0x01 ) == 0x01)
                      {
                        
                              Tx_Buffer[5] &= 0xFE;
                              
                      }
                      
                      
                              
                      
                }
                break;
                
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                
                /***************** 0x21 RF ������ ��û  ****************/      
                case RF_DATA_RQST:      // 0x21 RF ������ ��û 
                {
                  
                      RF_Key_CNT = Rx_Buffer[5];  // ��û�� ������ ��Ŷ ������ ������ ���� 
                      
                      TX_CMD = RF_DATA_RSPN; //  ���� ����Ʈ Ű �νĽ� 
                      
                      
                      Tx_Buffer[5] = RF_Key_CNT;
                      
                      RF_DATA_RQST_Flag = SET;
                  
                      
                      if(Usual_RF_Detec_Flag == RESET)
                      {
                            Tx_Buffer[5] = 0x00;
                            RF_Key_CNT = 0;
                            Tx_LENGTH = 7;
                            RF_DATA_RQST_Flag = RESET;
                      }
                     
                       
                }
                break;
                
                
                /***************** 0x22  ����ƮŰ ������ Ȯ��  ****************/      
                case RF_DATA_CONFIRM_RQST:  // 0x22  ����ƮŰ ������ Ȯ��
                {
                  
                        U1_Rx_Count = 0;
                        KEY_Number_to_Confirm = Rx_Buffer[5];                   // ��û ���� ����
                        
                    
                          
                        RF_Data_Confirm(KEY_Number_to_Confirm);                 // ���� ������ Ȯ�� �Լ�
                          
                        TX_CMD = RF_DATA_CONFIRM_RSPN ;                       
                     
                    
                        Tx_Buffer[5] = KEY_Number_to_Confirm;
                        Tx_LENGTH = 9;
                          
                         RF_DATA_RQST_Flag = RESET;  
                         
                         RF_Data_Confirm_Flag = SET;
                         
                         Usual_RF_Detec_Flag = RESET;
                         
                   
                 }
                 break;
                 
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                   
                      
                /***************** 0x31  ����ƮŰ ��� ��� ����  ****************/  
                case REG_MODE_START_RQST:  // 0x31  ����ƮŰ ��� ��� ����
                {
                  
                        TX_CMD = REG_MODE_START_RSPN ;    
                       
                       
                        Key_Reg_RQST_Flag = SET;
                        Tx_LENGTH = 7;
                        
                        if(Call_Button_Flag == SET)                     // �߰��� ȣ�� ��ư ���� ����
                        {
                          
                            Tx_Buffer[5] = 0x00;
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
                                       Temp_buffer[i] = Rx_Buffer[i];
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
                                     
                                    Tx_Buffer[5] = 0x01;
                            
                                  
                                    
                                    Reg_Compli_Flag = SET; 
                                           
                             }
                            
                            
                            if(Key_Reg_End_Button_Flag == SET)             // ��� �����ư ���� �� ���� ��Ŷ ������!!
                            {
                                  Reg_key_Value_Receive_Flag = RESET;
                    
                         
                                  Tx_LENGTH = 7;
                                  Tx_Buffer[5] |= 0x02;
                                 
                                  Key_Reg_End_Button_Flag = RESET;  // -> Ŭ���� ���� �ٽ� ���ϱ� 
                            }
                            
                          
                      } // END of  if (��� Ű�� �޾����� )
                      
                      
                      
                      
                      
                      
                      if(Reg_key_Value_Receive_Flag == RESET)  // ��� Ű��  �ȹ޾����� 
                      {
                          
                             if((!(Key_Info_Compare())) && (Reg_Compli_Flag == SET))  // ��Ŷ ������ ������  ���� Ű���� ������ 
                             {
                                    RF_Key_CNT = 1;
                                    Tx_LENGTH = 23;
                                     
                                    Tx_Buffer[5] = 0x01;
                                    
                                    Reg_key_Value_Receive_Flag = SET;
                             }
                          

                             if(Reg_Compli_Flag == RESET)
                             {
                          
                                      if(Key_Reg_End_Button_Flag == RESET)              // ��� �����ư  �ȴ����� ��
                                      {
                                        
                              
                                               
                                            Tx_LENGTH = 7;
                                            Tx_Buffer[5] &= 0xFC;
                                    
                                    
                                      }
                                      
                                             
                                      if(Key_Reg_End_Button_Flag == SET)             // ��� �����ư ���� ��
                                      {
                         
                                           Tx_LENGTH = 7;
                                           Tx_Buffer[5] |= 0x02;
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
                  
                       TX_CMD = REG_MODE_END_RSPN ;    
                        
                       Tx_LENGTH = 7;
                       
                       Tx_Buffer[5] = 0x01;

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
                 
     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////            
                 
                 
                 /*************************** 0x01 ��� ���� ��û ********************************/
                case EQUIP_INFOR_RQST:              // 0x01 ��� ���� ��û
                {
                  
                      TX_CMD = EQUIP_INFOR_RSPN ; 
                      
                      Tx_LENGTH = 15;
                      
                      Device_Info_Flag = SET;
                      
                      Tx_Buffer[5] = 0x00;
                      Tx_Buffer[6] = 0x00;
                      Tx_Buffer[7] = 0x01;
                      Tx_Buffer[8] = 0x0D;
                      Tx_Buffer[9] = 0x0C;
                      Tx_Buffer[10] = 0x04;
                      Tx_Buffer[11] = 0x01; 
                      Tx_Buffer[12] = 0x00;
                      Tx_Buffer[13] = 0x60;
                    
                }
                 break;
                  
         } //  END of switch 
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////            
                          
             
    
} // END of CMD function
  

///////////////////////////////////////////////////////////////////////////////////////////
/******************** RF ���� ��Ŷ �۽� �ϴ�  �Լ� *******************/
//////////////////////////////////////////////////////////////////////////////////////////
void USART1_TX(void)
{

        U1_Tx_Buffer[0] = U1_Paket_Type;
        U1_Tx_Buffer[6] = Rx_Buffer[5];
        U1_Tx_Buffer[7] = Rx_Buffer[6];
        U1_Tx_Buffer[8] = Rx_Buffer[7];
        U1_Tx_Buffer[9] = Rx_Buffer[8]; // site code
        U1_Tx_Buffer[10] = Rx_Buffer[9];
        U1_Tx_Buffer[11] = Rx_Buffer[10];
        U1_Tx_Buffer[12] = Rx_Buffer[11];
        U1_Tx_Buffer[13] = Rx_Buffer[12];  // dong , ho
        U1_Tx_Buffer[14] = Rx_Buffer[13];  // key no
        U1_Tx_Buffer[15] = 0x00;              // key type
        U1_Tx_Buffer[16] = 0x00;              // dummy
        
        
          for(unsigned char i = 0 ; i < 17 ; i++)
          {
               USART_SendData(USART1,U1_Tx_Buffer[i]);  
                
               while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); // wait for trans
          }




}
       
                  
/******************** ���� Ű ���� �� �Լ� *******************/
unsigned char Key_Info_Compare(void)            //  
{
      char Compare_CNT = 0;
       for(char i = 5 ; i < 14 ; i++)
       {
               if(Temp_buffer[i] == Rx_Buffer[i])
                 Compare_CNT ++;
       }  
      if(Compare_CNT == 9)       
        return 0 ;    
      else                              
        return 1 ;
}
                  
