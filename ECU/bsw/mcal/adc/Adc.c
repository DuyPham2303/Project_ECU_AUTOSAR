/* bsw/mcal/adc/Adc.c */
#include "Adc.h"
#include "csv_io.h"
#include <stdio.h>

static const char* key_of(uint8 ch){
    switch(ch){
        case CH_TEMP:    return "temp";
        case CH_VOLTAGE: return "voltage";
        case CH_CURRENT: return "current";
        case CH_TORQUE:  return "torque";
        case CH_RPM:     return "rpm";
        default:         return NULL;
    }
}

void Adc_Init(void){
    printf("[ADC] Init OK\n");
}

uint16 Adc_ReadChannel(uint8 channelId){
    const char* k = key_of(channelId);
    int v = 0;
    if(!k || csv_getInt(k, &v) != 0){
        printf("[ADC] Read ch=%u FAILED\n", (unsigned)channelId);
        return 0u;
    }
    if(v < 0) v = 0;
    if(v > 65535) v = 65535;
    printf("[ADC] Read ch=%u key=%s value=%d\n", (unsigned)channelId, k, v);
    return (uint16)v;
}
