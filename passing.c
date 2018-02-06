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


// 10 바이트 전송시간 1.04 ms /

///////////////////////////////////////////////////////////////////////////////
/******************** 월패드  패킷 처리  함수 *******************/
//////////////////////////////////////////////////////////////////////////////
void Packet_handler(void)       
{
          if(Rx_Compli_Flag == SET)
          {
                Rx_LENGTH = Rx_Buffer[2];
                Passing();
                if(Watch_Dog_init_Flag)                 //  초기 통신 시작시 1회 왓치독 셋팅 
                {
                    Watch_Dog_init_Flag = RESET;
                    WatchDog_Init(); 
                }

                if(Data_Check == 2) 
                {
                   Data_Check = 0;
                   CMD();
                   if((RF_Key_Detec_CNT_Flag))  // 평상시 키 인식시 
                    {
                          RF_Key_Detec_CNT_Flag = RESET;
                          Delay(12);                              //  idle time Delay 
                          Response();
                    }
                    
                    if((Status_Value_Clear_Flag)||(Reg_Mode_Start_Flag)||(Key_Reg_RQST_Flag)||(Key_Reg_End_Flag)||
                      (RF_DATA_RQST_Flag)||(RF_Data_Confirm_Flag)|| (Device_Info_Flag) ) // 그 외에 명령들
                    {  
                          Device_Info_Flag = RESET;  
                          Delay(12);                              //  idle time Delay 
                        Response();
                    }
                }
                
          }
          
}


////////////////////////////////////////////////////////////////////////////////
/******************** 월패드  패킷 검사 함수  *******************/
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


///////////////////////////////////////////////////////////////////////////
/******************** 체크섬 검사 함수 *******************/
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
/******************** 체크섬 만드는 함수 *******************/
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
/******************** 월패드  패킷에 대한 응답 처리 함수  *******************/
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
          
          
    /************* 평상시 RF 데이터 인식 시 응답패킷 저장 루틴 **************/
          if((RF_DATA_RQST_Flag == SET))
          {
            
                Tx_LENGTH = ( 16 * RF_Key_CNT ) + 7 ;
                
       
                  for(unsigned char i = 6 ; i < ( Tx_LENGTH - 1 ) ; i ++ ) 
                  {                                                                 // 패킷 길이 23 체크섭 전까지 버퍼  [23-1] = [22] ( 체크섬 자리) (<) 이므로 체크섬 자리바로 앞!
                      Tx_Buffer[i] = U1_71_Buffer[i-6] ;
                  }
                  
                  Tx_Buffer[2] = Tx_LENGTH;                            // Tx_LENGTH 변수에만 넣고 버퍼에 넣지 않으면 값이 않들어 감
                  
                  Tx_Buffer[Tx_LENGTH-1] = Make_Checksum();
             

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
                    Tx_Buffer[i] = U1_Rx_Buffer[U1_Rx_DataPosition-RF_KEY_PACKET_SIZE+i-5] ;                            //16까지 저장,  UART 1에서 들어오는 데이터 필드 [0] 은 버리고 전송
                }

                Tx_Buffer[2] = Tx_LENGTH;                                         // Tx_LENGTH 변수에만 넣고 버퍼에 넣지 않으면 값이 않들어 감
                
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
          
          
          
      /*************  응답 패킷 송신 후  예외 처리  루틴  **************/     
          if(Reg_Fail_Flag == SET)                 // 등록모드에서  이미 등록된 키일 경우 시 플래그 데이터 비트 재설정
          {
                Reg_Fail_Flag = RESET;
            
                Tx_Buffer[5] &= 0xFB;
           }
    
           if( RF_Communi_Fail == SET)          // 등록 모드에서  통신 실패 시 플래그 데이터 비트 재설정
           {
                RF_Communi_Fail = RESET;
              
                 Tx_Buffer[5] &= 0xF7;
           }
          
          
          
          
          U1_Tx_Flag= RESET;
          Rx_Compli_Flag = RESET;
          CNT = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/******************** 월패드 패킷에 대한 응답 패킷 송신  함수  *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////
void USART2_TX(void)            //현관 카메라 -> 월패드 전송 함수 
{
      for(unsigned char i = 0 ; i < Tx_LENGTH ; i++)
      {
           USART_SendData(USART2,Tx_Buffer[i]);  
           while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); // wait for trans
      }

        g_WatchdogEvent = SET;   // 통신시에 계속 이벤트 셋팅 
        Watch_Dog_Flag_CNT = 0; // 
        
        GPIO_WriteBit(GPIOB,  GPIO_Pin_0 , (BitAction) Bit_RESET);  // Receive Pin Enable
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);     
 
}


////////////////////////////////////////////////////////////////////////////////////
/******************** 각 명령어별 동작  함수  *******************/
///////////////////////////////////////////////////////////////////////////////////

void CMD(void)
{

        switch(Rx_Buffer[3])
        {
          
          /***************** 0x11 상태 값 요청 ****************/
                case RF_STATUS_RQST:  // 0x11 상태 값 요청 
                { 
                  
                      Tx_LENGTH = 8 ; 
                      TX_CMD = RF_STAUS_RSPN ;
         
                    
                               
                                 
                      if((RF_Data_Confirm_Flag == SET) || (RF_DATA_RQST_Flag == SET)|| (Status_Value_Clear_Flag == SET) 
                        || (Reg_Mode_Start_Flag == SET) || (Key_Reg_RQST_Flag == SET) ||  (Key_Reg_End_Flag == SET) ) 
                      {                                                         //  각 기능 구현 중 다시 평상시 패킷 들어올때 평상시 패킷 전송 
   
                             Clear_Tx_Buffer();
                            
                             if(Call_Button_Flag == SET)                // RF 모듈 뽑았을 때 버퍼 클리어로 인해 호출 안되는 현상 방지
                             {
                                    Call_Button_Flag = RESET;
                                    Tx_Buffer[5] |= 0x01;
                             }
                            
                             if(Key_Reg_RQST_Flag)                      // 등록 모드 중 평상 시 폴링시 RF 모듈를 등록 모드 종료시킴
                             {                                                     // 중간에 월패트가  꺼지고 나서 다시 켰을때 
                                    GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);  //  LED OFF
                                    U1_Paket_Type = 0xA0;                // 등록 종료 RF 모듈에 알림
                                    USART1_TX();
                             }
                             
                             
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
                
                        
                        
                       if(U1_Tx_Flag == RESET)                             // RF 모듈 폴링시 메인문 돌동안1번만 보내게 하는 루틴
                       {
                        
                                U1_Tx_Flag = SET;
                         
                                 U1_Paket_Type = 0xD0;  
                                 U1_Tx_Buffer[1] = 0xD0;
                                 U1_Tx_Buffer[2] = Rx_Buffer[5];
                                 U1_Tx_Buffer[3] = Rx_Buffer[6];
            
                                 USART1_TX();
                     
                                Time_Out_Flag = SET; //  타임 아웃
                     
                                
                       }
                       
             

                }
                break;
                
                
          /***************** 0x12  상태값 해제 요청 ****************/      
                case RF_STATUS_CLR_RQST:  // 0x12  상태값 해제 요청
                {
                  
                      Tx_LENGTH = 7 ; 
          
                      Status_Value_Clear_Flag = SET;
                      
             
                      
                      TX_CMD = RF_STAUS_CLR_RSPN;
          
    
                      
                      if((Rx_Buffer[5] & 0x80 ) == 0x80)  // 상태 값 해제 패킷에 따라 비트 클리어 
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
                
                
                
                /***************** 0x21 RF 데이터 요청  ****************/      
                case RF_DATA_RQST:      // 0x21 RF 데이터 요청 
                {
                  
                      RF_Key_CNT = Rx_Buffer[5];  // 요청한 데이터 패킷 갯수만 보내기 위함 
                      
                      TX_CMD = RF_DATA_RSPN; //  평상시 스마트 키 인식시 
                      
                      
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
                
                
                /***************** 0x22  스마트키 데이터 확인  ****************/      
                case RF_DATA_CONFIRM_RQST:  // 0x22  스마트키 데이터 확인
                {
                  
                        U1_Rx_Count = 0;
                        KEY_Number_to_Confirm = Rx_Buffer[5];                   // 요청 갯수 저장
                        
                    
                          
                        RF_Data_Confirm(KEY_Number_to_Confirm);                 // 전송 데이터 확인 함수
                          
                        TX_CMD = RF_DATA_CONFIRM_RSPN ;                       
                     
                    
                        Tx_Buffer[5] = KEY_Number_to_Confirm;
                        Tx_LENGTH = 9;
                          
                         RF_DATA_RQST_Flag = RESET;  
                         
                         RF_Data_Confirm_Flag = SET;
                         
                         Usual_RF_Detec_Flag = RESET;
                         
                   
                 }
                 break;
                 
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                   
                      
                /***************** 0x31  스마트키 등록 모드 시작  ****************/  
                case REG_MODE_START_RQST:  // 0x31  스마트키 등록 모드 시작
                {
                  
                        TX_CMD = REG_MODE_START_RSPN ;    
                       
                       
                        Key_Reg_RQST_Flag = SET;
                        Tx_LENGTH = 7;
                        
                        if(Call_Button_Flag == SET)                     // 중간에 호출 버튼 누름 방지
                        {
                          
                            Tx_Buffer[5] = 0x00;
                            Call_Button_Flag = RESET;
                          
                        }
                         
                        RF_DATA_RQST_Flag = RESET;     //  동록 모드 시작  전, 사이에 키인식 되면 키값이 들어오는것 방지
                        Reg_Mode_Start_Flag = SET;
                        
                        U1_Rx_Count = 0;                        // 등록 모드 시작 전, 키 인식 되면  U1_Rx_Count 증가되서 CA나 BA를 못받게 됨

                      
                 }
                 break;  
                 
                 
            /************************** 0x32  스마트키 등록 요청 ********************************/          
                case REG_KEY_DATA_RQST:  // 0x32  스마트키 등록 요청
                {
                    
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
                                       Temp_buffer[i] = Rx_Buffer[i];
                               }
                               
                               Key_Save_Flag = SET;
                               
                            
                       }
                     
                     
               
                       if(Key_Reg_U1_Send_Flag == RESET) 
                       {
                         
                             Key_Reg_U1_Send_Flag = SET;
                             
                             U1_Paket_Type = 0xC0;                    // 등록 요청 RF 모듈에 알림 
                             
                             USART1_TX();
                        }
                    
                       U1_Rx_Count = 0 ;  // 등록 키값 전달 전에 평상시 키값 들어오는 것 방지 

                     
                        
                           
                      if(Reg_key_Value_Receive_Flag == SET)  // 등록 키값 받았을때
                      { 
                    
                            if(Key_Reg_End_Button_Flag == RESET)             // 등록 종료버튼  안눌렀을 시
                            {
                              
                     
                              
                              
                                    RF_Key_CNT = 1;
                                    Tx_LENGTH = 23;
                                     
                                    Tx_Buffer[5] = 0x01;
                            
                                  
                                    
                                    Reg_Compli_Flag = SET; 
                                           
                             }
                            
                            
                            if(Key_Reg_End_Button_Flag == SET)             // 등록 종료버튼 누를 시 받은 패킷 지워야!!
                            {
                                  Reg_key_Value_Receive_Flag = RESET;
                    
                         
                                  Tx_LENGTH = 7;
                                  Tx_Buffer[5] |= 0x02;
                                 
                                  Key_Reg_End_Button_Flag = RESET;  // -> 클리어 시점 다시 정하기 
                            }
                            
                          
                      } // END of  if (등록 키값 받았을때 )
                      
                      
                      
                      
                      
                      
                      if(Reg_key_Value_Receive_Flag == RESET)  // 등록 키값  안받았을때 
                      {
                          
                             if((!(Key_Info_Compare())) && (Reg_Compli_Flag == SET))  // 패킷 못가져 갔을때  이전 키값과 같을때 
                             {
                                    RF_Key_CNT = 1;
                                    Tx_LENGTH = 23;
                                     
                                    Tx_Buffer[5] = 0x01;
                                    
                                    Reg_key_Value_Receive_Flag = SET;
                             }
                          

                             if(Reg_Compli_Flag == RESET)
                             {
                          
                                      if(Key_Reg_End_Button_Flag == RESET)              // 등록 종료버튼  안눌렀을 시
                                      {
                                        
                              
                                               
                                            Tx_LENGTH = 7;
                                            Tx_Buffer[5] &= 0xFC;
                                    
                                    
                                      }
                                      
                                             
                                      if(Key_Reg_End_Button_Flag == SET)             // 등록 종료버튼 누를 시
                                      {
                         
                                           Tx_LENGTH = 7;
                                           Tx_Buffer[5] |= 0x02;
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
                  
                       TX_CMD = REG_MODE_END_RSPN ;    
                        
                       Tx_LENGTH = 7;
                       
                       Tx_Buffer[5] = 0x01;

                        RF_DATA_RQST_Flag = RESET;     //  동록 모드 시작  전에 키인식 되면 키값이 들어오는것 방지 

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
                       
                       U1_Paket_Type = 0xA0;  // 등록 종료 RF 모듈에 알림
                       USART1_TX();
                      
                   
                     
                 }
                 break;  
                 
     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////            
                 
                 
                 /*************************** 0x01 기기 정보 요청 ********************************/
                case EQUIP_INFOR_RQST:              // 0x01 기기 정보 요청
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
/******************** RF 모듈로 패킷 송신 하는  함수 *******************/
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
       
                  
/******************** 이전 키 정보 비교 함수 *******************/
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
                  
