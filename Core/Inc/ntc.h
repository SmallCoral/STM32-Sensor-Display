#ifndef NTC_H
#define NTC_H

#include <stdint.h>

// 初始化 NTC 模块（可选）
void NTC_Init(float vref, float r_fixed, float r0, float b);

// 根据 ADC 读数转换为摄氏度
float NTC_ConvertToCelsius(uint32_t adc_value);

#endif // NTC_H
