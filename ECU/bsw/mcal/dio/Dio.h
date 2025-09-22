/* bsw/mcal/dio/Dio.h */
#ifndef DIO_H
#define DIO_H

#include "Std_Types.h"

#define DIO_CHANNEL_DIRECTION (0u)

void Dio_Init(void);
void Dio_WriteChannel(uint8 channelId, uint8 level); /* 0/1 */

#endif /* DIO_H */
