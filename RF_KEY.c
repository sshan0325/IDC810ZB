/* Includes ------------------------------------------------------------------*/

#include "RF_KEY.h"


/* Private variables ---------------------------------------------------------*/

unsigned char RF_Packet_1_4 = 0x00;
unsigned char RF_Packet_5 = 0x00;


unsigned char RF_Data_Check = RESET;
extern unsigned char Tx_Buffer[128];  //87->5
extern unsigned char Rx_Buffer[128];  //87->5
extern unsigned char U1_Rx_Compli_Flag;


extern unsigned char U1_Rx_Buffer[128] ;
extern unsigned char U1_Rx_Count;


extern unsigned char RF_Key_CNT;
extern unsigned char Reg_key_Value_Receive_Flag ;
extern unsigned char Usual_RF_Detec_Flag ;
extern unsigned char RF_Detec_Timeout_Flag ;

unsigned char value_1 = 6;
unsigned char Reg_Fail_Flag = RESET;
unsigned char RF_Communi_Fail = RESET;


unsigned char RF_Key_Detec_CNT_Flag = RESET;
extern unsigned char Time_Out_Flag ;
extern unsigned char Time_Out_Flag_CNT;

unsigned char U1_71_Buffer[128] = {0};
extern unsigned char U1_Rx_71_Compli_Flag ;
extern unsigned char Key_Reg_RQST_Flag;
/* Private functions ---------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** RF ��� ��Ŷ ���� ó�� �Լ�   *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Key_Paket_handler(void)
{
        if(U1_Rx_Compli_Flag == SET)            // ���� �Ϸ��
        {
                U1_Rx_Compli_Flag = RESET;
                
                /***************** ���� Ű �νĽ� ���� ó�� ��ƾ *********************/
                if(U1_Rx_Buffer[0] == 0xDA)              // ���� RF Ű �ν� ���� ��Ŷ
                {
                              RF_Key_CNT = U1_Rx_Buffer[2];
                              
                              Time_Out_Flag = RESET;     // RF ��� Ÿ�� �ƿ� �ʱ�ȭ
                              Time_Out_Flag_CNT = 0;
                              
                              if(RF_Key_CNT == 0x00)
                              {
                                  Tx_Buffer[6] =  RF_Key_CNT; //U1_DA_Buffer[2];  // Ű ���� ���� 
                                  RF_Key_Detec_CNT_Flag = SET;
                              }
                              else
                              { 
                                  Tx_Buffer[6] =  RF_Key_CNT; //U1_DA_Buffer[2];  // Ű ���� ���� 
                                  RF_Key_Detec_CNT_Flag = SET;  /////////-------------------->
                              }
                              
                              U1_Rx_Count = 0;
                }
                
                
                /***************** ��� ���� ����Ʈ Ű ������ ó�� ��ƾ  *********************/
                if(Key_Reg_RQST_Flag == SET)              // ��� ��� ���۽�
                {
                          /***************** ��� ���� *********************/
                          if(U1_Rx_Buffer[0] == 0xCA)             // ����ƮŰ ��ϼ��� (������ 2ȸ �߻�)
                          {
                                  Reg_key_Value_Receive_Flag = SET;  
                                  U1_Rx_Count = 0;
                                  
                                  TIM_SetCompare1(TIM3,100);
                                  GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);
                                  Delay(100);
                                  TIM_SetCompare1(TIM3,0);
                                  GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_SET);
                                  Delay(20);
                                  TIM_SetCompare1(TIM3,100);
                                  GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);
                                  Delay(100);
                                  TIM_SetCompare1(TIM3,0);
                                  GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_SET);
                          }

                          /***************** ��� ���� *********************/        
                          if(U1_Rx_Buffer[0] == 0xBA)             //����Ʈ Ű ��� ���� 
                          {
                                  Reg_key_Value_Receive_Flag = RESET;

                                  if(U1_Rx_Buffer[6] == 0x01) // �̹� ��ϵ� Ű�� ���(������ 4ȸ �߻�)
                                  {
                                        Tx_Buffer[5] |= 0x04;   // ��ϵ� Ű��� �÷��� ���� �� ������ ����
                                        Reg_Fail_Flag = SET;
                                          
                                        TIM_SetCompare1(TIM3,100);
                                        Delay(70);
                                        TIM_SetCompare1(TIM3,0);
                                        Delay(15);
                                        TIM_SetCompare1(TIM3,100);
                                        Delay(70);
                                        TIM_SetCompare1(TIM3,0);
                                        Delay(15);
                                        TIM_SetCompare1(TIM3,100);
                                        Delay(70);
                                        TIM_SetCompare1(TIM3,0);
                                        Delay(15);
                                        TIM_SetCompare1(TIM3,100);
                                        Delay(70);
                                        TIM_SetCompare1(TIM3,0);
                                  }
                                  
                                  if(U1_Rx_Buffer[6] == 0x02)  // ��� ���� ���� ���
                                  {
                                        Tx_Buffer[5] |= 0x08;     // ��� ���ж�� �÷��� ���� �� ������ ����
                                        RF_Communi_Fail = SET;
                                  }

                                  U1_Rx_Count = 0;
                          } // END of if (��� ����)
        
                
                }// ��� ��� ���� �÷��� ���� 
                      
        } // end of if
        
        
        /***************** ��� �� Ű �νĽ� ����ƮŰ ������ ó�� ��ƾ  *********************/
        if(U1_Rx_71_Compli_Flag == SET)
        {
                U1_Rx_71_Compli_Flag = RESET;
              
                if(U1_Rx_Buffer[0] == 0x71)                  // ���� Ű �νĽ� 
                {
                          RF_Key_Detec_CNT_Flag = SET;

                          if(RF_Key_CNT > 5)                 // �ν� Ű ������ 5�� �̻��� ���
                          {
                                U1_Rx_Count = 0;                // �迭 ī��Ʈ �ʱ�ȭ ���ϸ� �ý��� �ٿ�
                                RF_Key_CNT = 5;  
                          }
                          Usual_RF_Detec_Flag = SET;
                          RF_Data_Save(RF_Key_CNT,U1_Rx_Buffer);  // Ű ������ ���� �Լ� 
                          RF_Detec_Timeout_Flag = SET;                  // ���� Ű�ν� Ÿ�� �ƿ� �÷��� ����
                          
                          U1_Rx_Count = 0 ;
                }   
        }
  
  
} // end fo RF_Key_Paket_handler()
  
    
/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** �νĵ� ����ƮŰ ������ Ȯ�� �Լ�   *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////    
    
void RF_Data_Confirm(unsigned char CNT)  // �νĵ� Ű ������ Ȯ�� �Լ� 
{
    
        for(unsigned char i = 1 ; i <= CNT ; i++ )
        { 
                if(i == 1 )       { value_1 = 6 ; }
                if(i == 2 )       { value_1 = 22 ; }
                if(i == 3 )       { value_1 = 38 ; }
                if(i == 4 )       { value_1 = 54 ; }
                if(i == 5 )       { value_1 = 70 ; }
                  
                for(unsigned char j = value_1 ; j <= (value_1 + 15) ;j++)
                {
                  
                      if(Rx_Buffer[j] == U1_71_Buffer[j-6])          { RF_Data_Check ++;  }
                  
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
    
        Tx_Buffer[6] = RF_Packet_1_4;           // �˻� ��� ���۵����Ϳ� ����
        Tx_Buffer[7] = RF_Packet_5;
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
                  
                U1_71_Buffer[i] = U1_Rx[j];
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
                Tx_Buffer[i] = 0x00;
        }
} 
