/* bsw/ecua/iohwab/IoHwAb.c */
#include "IoHwAb.h"
#include "Adc.h"
#include "Dio.h"
#include "Pwm.h"
#include <stdio.h>

#define CH_TEMP     ((uint8)0u)
#define CH_VOLTAGE  ((uint8)1u)
#define CH_CURRENT  ((uint8)2u)
#define CH_TORQUE   ((uint8)3u)
#define CH_RPM      ((uint8)4u)

/* Quy đổi vật lý theo tài liệu:
   Temp: 0..200 C, Voltage: 0..60 V, Current: 0..50 A, Torque: 0..120 Nm */
#define ADC_MAX_F  (4095.0f)

Std_ReturnType IoHwAb_ReadTemp(uint16* temp_adc, float* temp_C){
    if(!temp_adc || !temp_C) return E_NOT_OK;
    *temp_adc = Adc_ReadChannel(CH_TEMP);
    *temp_C   = ((float)(*temp_adc)) * 200.0f / ADC_MAX_F;
    printf("[IoHwAb] Temp adc=%u => %.2f C\n", *temp_adc, *temp_C);
    return E_OK;

}

Std_ReturnType IoHwAb_ReadVoltage(uint16* voltage_adc, float* voltage_V){
    if(!voltage_adc || !voltage_V) return E_NOT_OK;
    *voltage_adc = Adc_ReadChannel(CH_VOLTAGE);
    *voltage_V   = ((float)(*voltage_adc)) * 60.0f / ADC_MAX_F;
    printf("[IoHwAb] Volt adc=%u => %.2f V\n", *voltage_adc, *voltage_V);
    return E_OK;
}

Std_ReturnType IoHwAb_ReadCurrent(uint16* current_adc, float* current_A){
    if(!current_adc || !current_A) return E_NOT_OK;
    *current_adc = Adc_ReadChannel(CH_CURRENT);
    *current_A   = ((float)(*current_adc)) * 50.0f / ADC_MAX_F;
    printf("[IoHwAb] Curr adc=%u => %.2f A\n", *current_adc, *current_A);
    return E_OK;
}

Std_ReturnType IoHwAb_ReadTorque(uint16* torque_adc, float* torque_Nm){
    if(!torque_adc || !torque_Nm) return E_NOT_OK;
    *torque_adc = Adc_ReadChannel(CH_TORQUE);
    *torque_Nm  = ((float)(*torque_adc)) * 120.0f / ADC_MAX_F;
    printf("[IoHwAb] Torq adc=%u => %.2f Nm\n", *torque_adc, *torque_Nm);
    return E_OK;
}

Std_ReturnType IoHwAb_ReadRpm(uint16* rpm){
    if(!rpm) return E_NOT_OK;
    *rpm = Adc_ReadChannel(CH_RPM);
    printf("[IoHwAb] RPM=%u\n", *rpm);
    return E_OK;
}

/* (tuỳ chọn) chấp hành để mô phỏng hướng & PWM */
Std_ReturnType IoHwAb_SetDirection(Direction_e dir){
    /* forward=1, reverse=0; neutral do PWM=0 xử lý */
    Dio_WriteChannel(DIO_CHANNEL_DIRECTION, (dir==DIR_FWD)?1u:-1u);
    printf("[IoHwAb] SetDirection=%d\n", (int)dir);

    return E_OK;
}
Std_ReturnType IoHwAb_SetDutyPercent(uint16 duty_percent){
    if(duty_percent>100u) duty_percent=100u;
    Pwm_SetDutyCycle(PWM_CHANNEL_MAIN, duty_percent);
    printf("[IoHwAb] SetDuty=%u%%\n", (unsigned)duty_percent);
    return E_OK;
}
