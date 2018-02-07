/* Includes ------------------------------------------------------------------*/


#include "platform_config.h"

/* Private define ------------------------------------------------------------*/
/////////       DEVICE STATE    /////////






/* Private function prototypes -----------------------------------------------*/
void Packet_handler(void);
unsigned char Check_Checksum(void);
unsigned char Make_Checksum(void);
void Passing(void);
void Response(void);
unsigned char CMD_Check(unsigned char *CMD, unsigned char CNT);