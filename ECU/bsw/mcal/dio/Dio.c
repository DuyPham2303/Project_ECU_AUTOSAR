/* bsw/mcal/dio/Dio.c */
#include "Dio.h"
#include "csv_io.h"
#include <stdio.h>

void Dio_Init(void){
    printf("[DIO] Init OK\n");
}

void Dio_WriteChannel(uint8 channelId, uint8 level){
    (void)channelId;
    (void)csv_setInt("direction", level ? 1 : 0);
    printf("[DIO] Write channel=%u level=%u\n", (unsigned)channelId, (unsigned)level);
}
