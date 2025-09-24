/* bsw/mcal/adc/Adc.h */
#ifndef ADC_H
#define ADC_H

#include "Std_Types.h"

/* Logical channels */
#define CH_TEMP     ((uint8)0u)
#define CH_VOLTAGE  ((uint8)1u)
#define CH_CURRENT  ((uint8)2u)
#define CH_TORQUE   ((uint8)3u)
#define CH_RPM      ((uint8)4u)

void   Adc_Init(void);
uint16 Adc_ReadChannel(uint8 channelId);

#endif /* ADC_H */
