#include "subfunction.h"
#include "stm32f0xx_tim.h"
#include "platform_config.h"

void BuzzerRun(unsigned char Freq, unsigned char BuzzerCount, unsigned char Ontime, unsigned char Offtime)
{
   unsigned char CurrntBuzzerCount;
   
   for (CurrntBuzzerCount = 0 ; CurrntBuzzerCount<BuzzerCount ; CurrntBuzzerCount++)
   {
        TIM_SetCompare1(TIM3,Freq);
        GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_RESET);
        Delay(Ontime);
        TIM_SetCompare1(TIM3,0);
        GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction) Bit_SET);
        Delay(Offtime);     
   }
}