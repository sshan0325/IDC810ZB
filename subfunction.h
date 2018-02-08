void BuzzerRun(unsigned char Freq, unsigned char BuzzerCount, unsigned char Ontime, unsigned char Offtime);
void Key_Polling(void);
void Check_Key_State(void);

/* Private define ------------------------------------------------------------*/

/* KEY STATE -----------------------------------------------------------------*/
#define KEY_PUSHED                      0
#define KEY_RELEASED                    1
#define KEY_POLLING_CHECKCOUNT          2

/* KEY ACTIVE ----------------------------------------------------------------*/
#define KEY_ACTIVE                      1
#define KEY_INACTIVE                    0