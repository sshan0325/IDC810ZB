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
extern unsigned char U2_Rx_Buffer[128];  
extern unsigned char U2_Tx_Buffer[128];  

unsigned char RF_Key_Data[128] = {0};


//Need to Check
unsigned char RF_Packet_1_4 = 0x00;
unsigned char RF_Packet_5 = 0x00;
unsigned char RF_Data_Check = RESET;
extern unsigned char Reg_key_Value_Receive_Flag ;
extern unsigned char Usual_RF_Detec_Flag ;
extern unsigned char RF_Detec_Timeout_Flag ;
unsigned char value_1 = 6;
unsigned char Reg_Fail_Flag = RESET;
unsigned char RF_Communi_Fail = RESET;
extern unsigned char Key_Reg_RQST_Flag;
//Flag
unsigned char RF_Key_Detec_CNT_Flag = RESET;
//Timer
extern unsigned char Time_Out_Flag ;
extern unsigned char Time_Out_Flag_CNT;



/* Private functions ---------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////////
/******************** RF 모듈 패킷 수신 처리 함수   *******************/
///////////////////////////////////////////////////////////////////////////////////
void RF_Key_Packet_handler(void)
{
    int tmp=0;
    
    if(U1_Rx_Count >= RF_KEY_PACKET_SIZE)
    {
#if 1
        #ifdef Consol_LOG 
        printf ("\r\n");
        printf ("[RF -> CAM]  : ") ;            
        //printf ("Total Input Data Length : %d \r\n ",U1_Rx_Count) ;      
        for (tmp=U1_Rx_DataPosition ; tmp<U1_Rx_DataPosition+17 ; tmp++)
        {
          printf ("%x, ",U1_Rx_Buffer[tmp]) ;
        }
        printf ("\r\n");
        #endif
#endif              
        switch (U1_Rx_Buffer[U1_Rx_DataPosition])
        {
        case RF_KEY_CHECK:
                RF_Key_CNT = U1_Rx_Buffer[U1_Rx_DataPosition+2];
            
                U2_Tx_Buffer[6] =  RF_Key_CNT; //U1_DA_Buffer[2];  // 키 갯수 전달 
                RF_Key_Detec_CNT_Flag = SET;
                
                Time_Out_Flag = RESET;     // RF 모듈 타임 아웃 초기화
                Time_Out_Flag_CNT = 0;
                
                #ifdef Consol_LOG        
                if (RF_Key_CNT >0)
                {
                    printf ("\r\n[RF Key Comm           ] RF_KEY Recognition No. : %d ( %x, %x, %x, %x)\r\n", RF_Key_CNT,U1_Rx_Buffer[U1_Rx_DataPosition],U1_Rx_Buffer[U1_Rx_DataPosition+1],U1_Rx_Buffer[U1_Rx_DataPosition+2],U1_Rx_Buffer[U1_Rx_DataPosition+3]); 
                }
                #endif      
                
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
                    
                    if(U1_Rx_Buffer[U1_Rx_DataPosition+6] == 0x01) // 이미 등록된 키일 경우(부저음 4회 발생)
                    {
                          U2_Tx_Buffer[5] |= 0x04;   // 등록된 키라는 플래그 셋팅 및 데이터 저장
                          Reg_Fail_Flag = SET;
                            
                          BuzzerRun(100, 4, 70, 15);
                    }
                    else if(U1_Rx_Buffer[U1_Rx_DataPosition+6] == 0x02)  // 통신 실패 났을 경우
                    {
                          U2_Tx_Buffer[5] |= 0x08;     // 통신 실패라는 플래그 셋팅 및 데이터 저장
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
              
        case RF_KEY_RECOGNITION:   
                if ((RF_Key_CNT * RF_KEY_PACKET_SIZE) <= U1_Rx_Count)
                {
                    #ifdef Consol_LOG                      
                    printf ("\r\n[RF Key Comm           ] RF_KEY Recognition Done\r\n"); 
                    printf ("\r\n[RF Key Comm           ] RF_KEY Count : %d\r\n",RF_Key_CNT); 
                    #endif                      
                    
                    RF_Key_Detec_CNT_Flag = SET;

                    if(RF_Key_CNT > 5)                 // 인식 키 개수가 5개 이상일 경우
                    {
                          U1_Rx_Count = 0;                // 배열 카운트 초기화 안하면 시스템 다운
                          RF_Key_CNT = 5;  
                    }
                    Usual_RF_Detec_Flag = SET;
                    RF_Data_Save(RF_Key_CNT,&U1_Rx_Buffer[U1_Rx_DataPosition]);  // 키 데이터 저장 함수 
                    RF_Detec_Timeout_Flag = SET;                  // 평상시 키인식 타임 아웃 플래그 셋팅      

                    U1_Rx_Count -= RF_KEY_PACKET_SIZE;
                    U1_Rx_DataPosition = (U1_Rx_DataPosition+RF_KEY_PACKET_SIZE);                    
                }
                else
                {
                  
                }
                break;
        default:
            U1_Rx_Count --;
            U1_Rx_DataPosition ++;              
            
            #ifdef Consol_LOG        
            printf ("\r\n[RF Key Comm           ] RF_KEY Normal mode Packet Error\r\n"); 
            #endif                  
            
            break;
        }
             
    }
} 
  
    
/////////////////////////////////////////////////////////////////////////////////////////
/******************** 인식된 스마트키 데이터 확인 함수   *******************/
/////////////////////////////////////////////////////////////////////////////////////////
    
void RF_Data_Confirm(unsigned char CNT)  // 인식된 키 데이터 확인 함수 
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
    
        U2_Tx_Buffer[6] = RF_Packet_1_4;           // 검사 결과 전송데이터에 저장
        U2_Tx_Buffer[7] = RF_Packet_5;
        RF_Data_Check = 0;
  
  

} // end of RF_Data_Confirm()
 


////////////////////////////////////////////////////////////////////////////////////////
/******************** 인식된 스마트키 데이터 저장 함수  *******************/
///////////////////////////////////////////////////////////////////////////////////////
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
                  
                RF_Key_Data[i] = U1_Rx[j];
        }
}


 /////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************** 전송 데이터 초기화 함수 ( 타임아웃 발생시 사용 )  *******************/
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void Clear_Tx_Buffer(void)  // 전송 데이터 초기화 함수
{
        unsigned char  i= 0;

        for( i = 0 ; i < 92 ; i++)  
        {
                U2_Tx_Buffer[i] = 0x00;
        }
} 
