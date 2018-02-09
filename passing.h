/* Includes ------------------------------------------------------------------*/


#include "platform_config.h"

/* Private define ------------------------------------------------------------*/
#define VALID   2
/////////       DEVICE STATE    /////////






/* Private function prototypes -----------------------------------------------*/
void Packet_handler(void);
unsigned char PacketValidation(void);
void Response(void);
unsigned char CMD_Check(unsigned char *CMD, unsigned char CNT);