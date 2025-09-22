/* bsw/mcal/adc/Adc.h */
#ifndef ADC_H
#define ADC_H

#include "Std_Types.h"

/* Logical channels */
#define CH_TEMP     (0u)
#define CH_VOLTAGE  (1u)
#define CH_CURRENT  (2u)
#define CH_TORQUE   (3u)
#define CH_RPM      (4u)

void   Adc_Init(void);
uint16 Adc_ReadChannel(uint8 channelId);

#endif /* ADC_H */
