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
extern unsigned char    U2_Rx_Buffer[128]; 
unsigned char               U2_Tx_Buffer[128] = {0} ;  


///////////////// DEVICE STATE /////////////////
unsigned char Key_Reg_RQST_Flag = RESET;

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
unsigned char CMD_Buffer[8] = { RF_STATUS_RQST , RF_STATUS_CLR_RQST , RF_DATA_RQST , RF_DATA_CONFIRM_RQST ,
                                                REG_MODE_START_RQST , REG_KEY_DATA_RQST , REG_MODE_END_RQST , EQUIP_INFOR_RQST};
unsigned char KEY_Number_to_Confirm = 0;
extern unsigned char Reg_key_Value_Receive_Flag ;
extern unsigned char Usual_RF_Detec_Flag;
unsigned char U1_Tx_Buffer[128]= {0};
unsigned char Key_Reg_Timeout_flag = RESET;
extern unsigned int Key_Reg_Timeout_CNT ;
extern unsigned char Reg_Fail_Flag;
extern unsigned char RF_Communi_Fail ;
unsigned char Key_Reg_U1_Send_Flag = RESET;
unsigned char Reg_Compli_Flag = RESET;
unsigned char Key_Save_Flag = RESET;
unsigned char U1_Paket_Type = 0x00;
unsigned char Time_Out_Flag = RESET;
unsigned char Status_Value_Clear_Flag = RESET;
extern unsigned char RF_Key_Data[128];
unsigned char U1_Tx_Flag =  RESET;
unsigned char Watch_Dog_init_Flag = SET;
extern unsigned char CNT ;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


// 10 바이트 전송시간 1.04 ms /

///////////////////////////////////////////////////////////////////////////////
/******************** 월패드  패킷 처리  함수 *******************/
//////////////////////////////////////////////////////////////////////////////
void Packet_handler(void)       
{
          if(U2_Rx_Compli_Flag == SET)
          {
                if(Watch_Dog_init_Flag)                 //  초기 통신 시작시 1회 왓치독 셋팅 
                {
                    Watch_Dog_init_Flag = RESET;
                    WatchDog_Init(); 
                }

                if( PacketValidation() == VALID) 
                {
                    CMD();
                    Delay(12);                              //  idle time Delay 
                    Response();                   
                }
                U2_Rx_Compli_Flag=RESET;
          }
}


////////////////////////////////////////////////////////////////////////////////
/******************** 월패드  패킷 검사 함수  *******************/
///////////////////////////////////////////////////////////////////////////////
unsigned char PacketValidation(void)   
{
    unsigned char Result=0;
    unsigned char Rx_Length=0;
    
    Rx_Length = U2_Rx_Buffer[2];
    
    if( CMD_Check( U2_Rx_Buffer, sizeof(CMD_Buffer)/sizeof(unsigned char)))   
    {
        Result ++; 
    }
     
    if( U2_Rx_Buffer[(Rx_Length-1)] == Check_Checksum())        
    {
        Result ++; 
    }

    if(Result != VALID )
    {
          U2_Rx_Count =0;
          U2_Rx_Compli_Flag = RESET ;
          for(unsigned char i = 0; i<92 ; i++ )
          {
              U2_Rx_Buffer[i] = 0;
              U2_Tx_Buffer[i] = 0;
          }
     }
    return Result;
 }


///////////////////////////////////////////////////////////////////////////
/******************** 명령어 검사 함수 *******************/
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
/******************** 월패드  패킷에 대한 응답 처리 함수  *******************/
///////////////////////////////////////////////////////////////////////////////////////
void Response(void)                                                          
{
    GPIO_WriteBit(GPIOB,  GPIO_Pin_0 , (BitAction) Bit_SET);  // 485 Trans pin Enable 

    U2_Tx_Buffer[0] = STX ;
    U2_Tx_Buffer[1] = RF_Camera_ID ;
    U2_Tx_Buffer[2] = Tx_LENGTH ;
    U2_Tx_Buffer[3] = TX_CMD;
    U2_Tx_Buffer[4] = U2_Rx_Buffer[4] ;
    U2_Tx_Buffer[Tx_LENGTH-1] = Make_Checksum() ;

    /************* 평상시 RF 데이터 인식 시 응답패킷 저장 루틴 **************/
    if((RF_DATA_RQST_Flag == SET))
    {
          Tx_LENGTH = ( 16 * RF_Key_CNT ) + 7 ;
           for(unsigned char i = 6 ; i < ( Tx_LENGTH - 1 ) ; i ++ ) 
           {                                                                 // 패킷 길이 23 체크섭 전까지 버퍼  [23-1] = [22] ( 체크섬 자리) (<) 이므로 체크섬 자리바로 앞!
               U2_Tx_Buffer[i] = RF_Key_Data[i-6] ;
           }
           U2_Tx_Buffer[2] = Tx_LENGTH;                            // Tx_LENGTH 변수에만 넣고 버퍼에 넣지 않으면 값이 않들어 감
           U2_Tx_Buffer[Tx_LENGTH-1] = Make_Checksum();

           RF_DATA_RQST_Flag = RESET; // 15.05.20 전광식.
           RF_Key_CNT = 0;
           Reg_key_Value_Receive_Flag = RESET;
    }
          
    /*************  스마트키 등록시 응답패킷 저장 루틴  **************/          
    if((Reg_key_Value_Receive_Flag == SET))
   {
        Tx_LENGTH = ( 16 * RF_Key_CNT ) + 7 ;
        for(unsigned char i = 6 ; i < ( Tx_LENGTH - 1 ) ; i ++ )    // 22
        {                                                                              // 패킷 길이 23 체크섭 전까지 버퍼  [23-1] = [22] ( 체크섬 자리) (<) 이므로 체크섬 자리바로 앞!
            U2_Tx_Buffer[i] = U1_Rx_Buffer[U1_Rx_DataPosition-RF_KEY_PACKET_SIZE+i-5] ;                            //16까지 저장,  UART 1에서 들어오는 데이터 필드 [0] 은 버리고 전송
        }
        U2_Tx_Buffer[2] = Tx_LENGTH;                                         // Tx_LENGTH 변수에만 넣고 버퍼에 넣지 않으면 값이 않들어 감
        U2_Tx_Buffer[Tx_LENGTH-1] = Make_Checksum();

        RF_Key_CNT = 0;
        Reg_key_Value_Receive_Flag = RESET;
   }

   USART2_TX();
          
          
          
   /*************  응답 패킷 송신 후  예외 처리  루틴  **************/     
   if(Reg_Fail_Flag == SET)                 // 등록모드에서  이미 등록된 키일 경우 시 플래그 데이터 비트 재설정
  {
        Reg_Fail_Flag = RESET;
        U2_Tx_Buffer[5] &= 0xFB;
   }
   if( RF_Communi_Fail == SET)          // 등록 모드에서  통신 실패 시 플래그 데이터 비트 재설정
   {
        RF_Communi_Fail = RESET;
        U2_Tx_Buffer[5] &= 0xF7;
   }

   U1_Tx_Flag= RESET;
   U2_Rx_Compli_Flag = RESET;
   CNT = 0;
}




////////////////////////////////////////////////////////////////////////////////////
/******************** 각 명령어별 동작  함수  *******************/
///////////////////////////////////////////////////////////////////////////////////
void CMD(void)
{
    unsigned char Requested_CMD;
    Requested_CMD = U2_Rx_Buffer[3];

      
    switch(Requested_CMD)
    {
        /***************** 0x11 상태 값 요청 ****************/
        case RF_STATUS_RQST:  // 0x11 상태 값 요청 
        { 
              Tx_LENGTH = 8 ; 
              TX_CMD = RF_STAUS_RSPN ;

               if(Key_Reg_RQST_Flag)                      // 등록 모드 중 평상 시 폴링시 RF 모듈를 등록 모드 종료시킴
               {                                                     // 중간에 월패트가  꺼지고 나서 다시 켰을때 
                      GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);  //  LED OFF
                      U1_Paket_Type = 0xA0;                // 등록 종료 RF 모듈에 알림
                      USART1_TX();
               }         
               
               if(U1_Tx_Flag == RESET)                             // RF 모듈 폴링시 메인문 돌동안1번만 보내게 하는 루틴
               {
                      U1_Tx_Flag = SET;
                      U1_Paket_Type = 0xD0;  
                      U1_Tx_Buffer[1] = 0xD0;
                      U1_Tx_Buffer[2] = U2_Rx_Buffer[5];
                      U1_Tx_Buffer[3] = U2_Rx_Buffer[6];
  
                      USART1_TX();
           
                      Time_Out_Flag = SET; //  타임 아웃
                }
               
               if((RF_Data_Confirm_Flag == SET) || (RF_DATA_RQST_Flag == SET)|| (Status_Value_Clear_Flag == SET) 
                  || (Reg_Mode_Start_Flag == SET) || (Key_Reg_RQST_Flag == SET) ||  (Key_Reg_End_Flag == SET) ) 
               {                                                         //  각 기능 구현 중 다시 평상시 패킷 들어올때 평상시 패킷 전송 
                     Clear_Tx_Buffer();

                     RF_Data_Confirm_Flag = RESET;             // 각 기능에 대한 플래그 초기화
                     Key_Reg_End_Flag = RESET;
                     Reg_Mode_Start_Flag = RESET;
                     Key_Reg_RQST_Flag = RESET;
                     Key_Reg_End_Flag = RESET;
                     Status_Value_Clear_Flag = RESET;
                     Key_Reg_U1_Send_Flag = RESET;             // 등록 실패 후 다시 재등록이 안됨 
                     Reg_key_Value_Receive_Flag = RESET;
               } 

               RF_DATA_RQST_Flag = RESET;                        //전, 사이에 키인식 되면 키값이 들어오는것 방지
        }
        break;
        
        
        /***************** 0x12  상태값 해제 요청 ****************/      
        case RF_STATUS_CLR_RQST:  // 0x12  상태값 해제 요청
        {
              Tx_LENGTH = 7 ; 

              Status_Value_Clear_Flag = SET;

              TX_CMD = RF_STAUS_CLR_RSPN;

              if((U2_Rx_Buffer[5] & 0x80 ) == 0x80)  // 상태 값 해제 패킷에 따라 비트 클리어 
              {
                      U2_Tx_Buffer[5] &= 0x7F;
              }
              
              if((U2_Rx_Buffer[5] & 0x01 ) == 0x01)
              {
                       U2_Tx_Buffer[5] &= 0xFE;
               }
         }
        break;
        
        /***************** 0x21 RF 데이터 요청  ****************/      
        case RF_DATA_RQST:      // 0x21 RF 데이터 요청 
        {
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] RF Data is Requested.");     
              #endif                    
              RF_Key_CNT = U2_Rx_Buffer[5];  // 요청한 데이터 패킷 갯수만 보내기 위함 
              
              TX_CMD = RF_DATA_RSPN; //  평상시 스마트 키 인식시 

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

        /***************** 0x22  스마트키 데이터 확인  ****************/      
        case RF_DATA_CONFIRM_RQST:  // 0x22  스마트키 데이터 확인
        {
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] RF Data Confirm is Requested.");     
              #endif                    
          
                //U1_Rx_Count = 0;
                KEY_Number_to_Confirm = U2_Rx_Buffer[5];                   // 요청 갯수 저장
                RF_Data_Confirm(KEY_Number_to_Confirm);                 // 전송 데이터 확인 함수
                TX_CMD = RF_DATA_CONFIRM_RSPN ;                       
                U2_Tx_Buffer[5] = KEY_Number_to_Confirm;
                Tx_LENGTH = 9;
                RF_DATA_RQST_Flag = RESET;  
                RF_Data_Confirm_Flag = SET;
                Usual_RF_Detec_Flag = RESET;
         }
         break;
         
        /***************** 0x31  스마트키 등록 모드 시작  ****************/  
        case REG_MODE_START_RQST:  // 0x31  스마트키 등록 모드 시작
        {
                #ifdef Consol_LOG 
                printf ("\r\n[System                ] Regist mode Start Request");     
                #endif          
                TX_CMD = REG_MODE_START_RSPN ;    
                Key_Reg_RQST_Flag = SET;
                Tx_LENGTH = 7;
                
                RF_DATA_RQST_Flag = RESET;     //  동록 모드 시작  전, 사이에 키인식 되면 키값이 들어오는것 방지
                Reg_Mode_Start_Flag = SET;
                
                //U1_Rx_Count = 0;                        // 등록 모드 시작 전, 키 인식 되면  U1_Rx_Count 증가되서 CA나 BA를 못받게 됨
         }
         break;  
         
         
        /************************** 0x32  스마트키 등록 요청 ********************************/          
        case REG_KEY_DATA_RQST:  // 0x32  스마트키 등록 요청
        {
              //#ifdef Consol_LOG        
              //printf ("\r\n[System                ] RF KEY Data is Requested.");     
              //#endif                    
               TX_CMD = REG_KEY_DATA_RSPN ;    
              
               Key_Reg_RQST_Flag = SET;
               Reg_Mode_Start_Flag = RESET;
               
               Key_Reg_Timeout_flag = RESET;
               Key_Reg_Timeout_CNT =0; // 등록 모드 타임아웃 초기화 

               GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_SET);  // 중간에 현관카메라가 꺼져지고 나서 다시 켰을때 등록모드를 유지하기 위해서 
             
   
               if(Key_Info_Compare())                   // 이전 키값과 다르면 
               {
                      Key_Reg_U1_Send_Flag = RESET;
                      Reg_Compli_Flag = RESET;
                      Key_Save_Flag = RESET;
               }

               if(Key_Save_Flag == RESET)  // 월패드에서 등록 실패시 키정보 비교하기위해 임시저장 루틴
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
                     U1_Paket_Type = 0xC0;                    // 등록 요청 RF 모듈에 알림 
                     USART1_TX();
                }
            
                //U1_Rx_Count = 0 ;  // 등록 키값 전달 전에 평상시 키값 들어오는 것 방지 

              if(Reg_key_Value_Receive_Flag == SET)  // 등록 키값 받았을때
              { 
                    if(Key_Reg_End_Button_Flag == RESET)             // 등록 종료버튼  안눌렀을 시
                    {
                            RF_Key_CNT = 1;
                            Tx_LENGTH = 23;
                             
                            U2_Tx_Buffer[5] = 0x01;

                            Reg_Compli_Flag = SET; 
                     }

                    if(Key_Reg_End_Button_Flag == SET)             // 등록 종료버튼 누를 시 받은 패킷 지워야!!
                    {
                          Reg_key_Value_Receive_Flag = RESET;

                          Tx_LENGTH = 7;
                          U2_Tx_Buffer[5] |= 0x02;
                         
                          Key_Reg_End_Button_Flag = RESET;  // -> 클리어 시점 다시 정하기 
                    }
              } // END of  if (등록 키값 받았을때 )

              if(Reg_key_Value_Receive_Flag == RESET)  // 등록 키값  안받았을때 
              {
                     if((!(Key_Info_Compare())) && (Reg_Compli_Flag == SET))  // 패킷 못가져 갔을때  이전 키값과 같을때 
                     {
                            RF_Key_CNT = 1;
                            Tx_LENGTH = 23;
                             
                            U2_Tx_Buffer[5] = 0x01;
                            
                            Reg_key_Value_Receive_Flag = SET;
                     }

                     if(Reg_Compli_Flag == RESET)
                     {
                              if(Key_Reg_End_Button_Flag == RESET)              // 등록 종료버튼  안눌렀을 시
                              {
                                    Tx_LENGTH = 7;
                                    U2_Tx_Buffer[5] &= 0xFC;
                              }
                              
                              if(Key_Reg_End_Button_Flag == SET)             // 등록 종료버튼 누를 시
                              {
                                   Tx_LENGTH = 7;
                                   U2_Tx_Buffer[5] |= 0x02;
                                    Key_Reg_End_Button_Flag = RESET;
                              }
                       }
              }  // END of if (등록 키값 안받았을때 )
               Key_Reg_Timeout_flag = SET;  // 등록 모드 타임아웃 세팅 
             
         }// END of case
         break;  

        /*************************** 0x33  스마트키 등록 모드 종료  ********************************/
        case REG_MODE_END_RQST:  // 0x33  스마트키 등록 모드 종료 
        {
              #ifdef Consol_LOG        
              printf ("\r\n[System                ] RF Status clear is Requested.");     
              #endif                    
               TX_CMD = REG_MODE_END_RSPN ;    
               Tx_LENGTH = 7;
               U2_Tx_Buffer[5] = 0x01;
                RF_DATA_RQST_Flag = RESET;     //  동록 모드 시작  전에 키인식 되면 키값이 들어오는것 방지 

                Key_Reg_End_Flag = SET;
                Key_Reg_RQST_Flag = RESET;
                Reg_Mode_Start_Flag = RESET;
              
                BuzzerRun(100, 1,80,10);

               GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);  //  LED OFF
               
                for(char i = 5 ; i < 14 ; i++)
                {
                        Temp_buffer[i] = 0;
                }  
               
               U1_Paket_Type = 0xA0;  // 등록 종료 RF 모듈에 알림
               USART1_TX();
         }
         break;  
         
         /*************************** 0x01 기기 정보 요청 ********************************/
        case EQUIP_INFOR_RQST:              // 0x01 기기 정보 요청
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
  


                  
/******************** 이전 키 정보 비교 함수 *******************/
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
                  
