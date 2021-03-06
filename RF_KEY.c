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
/******************** RF 모듈 패킷 수신 처리 함수   *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Key_Paket_handler(void)
{
        if(U1_Rx_Compli_Flag == SET)            // 수신 완료시
        {
                U1_Rx_Compli_Flag = RESET;
                
                /***************** 평상시 키 인식시 개수 처리 루틴 *********************/
                if(U1_Rx_Buffer[0] == 0xDA)              // 평상시 RF 키 인식 갯수 패킷
                {
                              RF_Key_CNT = U1_Rx_Buffer[2];
                              
                              Time_Out_Flag = RESET;     // RF 모듈 타임 아웃 초기화
                              Time_Out_Flag_CNT = 0;
                              
                              if(RF_Key_CNT == 0x00)
                              {
                                  Tx_Buffer[6] =  RF_Key_CNT; //U1_DA_Buffer[2];  // 키 갯수 전달 
                                  RF_Key_Detec_CNT_Flag = SET;
                              }
                              else
                              { 
                                  Tx_Buffer[6] =  RF_Key_CNT; //U1_DA_Buffer[2];  // 키 갯수 전달 
                                  RF_Key_Detec_CNT_Flag = SET;  /////////-------------------->
                              }
                              
                              U1_Rx_Count = 0;
                }
                
                
                /***************** 등록 모드시 스마트 키 데이터 처리 루틴  *********************/
                if(Key_Reg_RQST_Flag == SET)              // 등록 모드 시작시
                {
                          /***************** 등록 성공 *********************/
                          if(U1_Rx_Buffer[0] == 0xCA)             // 스마트키 등록성공 (부저음 2회 발생)
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

                          /***************** 등록 실패 *********************/        
                          if(U1_Rx_Buffer[0] == 0xBA)             //스마트 키 등록 실패 
                          {
                                  Reg_key_Value_Receive_Flag = RESET;

                                  if(U1_Rx_Buffer[6] == 0x01) // 이미 등록된 키일 경우(부저음 4회 발생)
                                  {
                                        Tx_Buffer[5] |= 0x04;   // 등록된 키라는 플래그 셋팅 및 데이터 저장
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
                                  
                                  if(U1_Rx_Buffer[6] == 0x02)  // 통신 실패 났을 경우
                                  {
                                        Tx_Buffer[5] |= 0x08;     // 통신 실패라는 플래그 셋팅 및 데이터 저장
                                        RF_Communi_Fail = SET;
                                  }

                                  U1_Rx_Count = 0;
                          } // END of if (등록 실패)
        
                
                }// 등록 모드 시작 플래그 세팅 
                      
        } // end of if
        
        
        /***************** 평상 시 키 인식시 스마트키 데이터 처리 루틴  *********************/
        if(U1_Rx_71_Compli_Flag == SET)
        {
                U1_Rx_71_Compli_Flag = RESET;
              
                if(U1_Rx_Buffer[0] == 0x71)                  // 평상시 키 인식시 
                {
                          RF_Key_Detec_CNT_Flag = SET;

                          if(RF_Key_CNT > 5)                 // 인식 키 개수가 5개 이상일 경우
                          {
                                U1_Rx_Count = 0;                // 배열 카운트 초기화 안하면 시스템 다운
                                RF_Key_CNT = 5;  
                          }
                          Usual_RF_Detec_Flag = SET;
                          RF_Data_Save(RF_Key_CNT,U1_Rx_Buffer);  // 키 데이터 저장 함수 
                          RF_Detec_Timeout_Flag = SET;                  // 평상시 키인식 타임 아웃 플래그 셋팅
                          
                          U1_Rx_Count = 0 ;
                }   
        }
  
  
} // end fo RF_Key_Paket_handler()
  
    
/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** 인식된 스마트키 데이터 확인 함수   *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////    
    
void RF_Data_Confirm(unsigned char CNT)  // 인식된 키 데이터 확인 함수 
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
                    
                 
                if((i == 1) && (RF_Data_Check == 16 ))       RF_Packet_1_4 = 0x01;  // 1 번째 패킷 검사시 값을 대입 하기 때문에 초기화루틴은 따로 없어도 됨
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
    
        Tx_Buffer[6] = RF_Packet_1_4;           // 검사 결과 전송데이터에 저장
        Tx_Buffer[7] = RF_Packet_5;
        RF_Data_Check = 0;
  
  

} // end of RF_Data_Confirm()
 


/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** 인식된 스마트키 데이터 저장 함수  *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////
void RF_Data_Save(unsigned char CNT, unsigned char *U1_Rx)      // 스마트키 데이터 저장 함수
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
/******************** 전송 데이터 초기화 함수 ( 타임아웃 발생시 사용 )  *******************/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Clear_Tx_Buffer(void)  // 전송 데이터 초기화 함수
{
        unsigned char  i= 0;

        for( i = 0 ; i < 92 ; i++)  
        {
                Tx_Buffer[i] = 0x00;
        }
} 
