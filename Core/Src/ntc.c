#include "ntc.h"
#include <math.h>

static float NTC_Vref = 5.08f;           // 默认供电电压（建议使用实际测量值）
static float NTC_R_fixed = 50000.0f;    // 默认固定电阻（Ω）
static float NTC_R0 = 50000.0f;         // 默认热敏电阻在25℃的阻值（Ω）
static float NTC_B = 3950.0f;           // 默认B值
static const float NTC_T0 = 298.15f;    // 25℃ in Kelvin

void NTC_Init(float vref, float r_fixed, float r0, float b) {
    NTC_Vref = vref;
    NTC_R_fixed = r_fixed;
    NTC_R0 = r0;
    NTC_B = b;
}

float NTC_ConvertToCelsius(uint32_t adc_value) {
    if (adc_value == 0 || adc_value >= 4095) {
        return -1000.0f; // 表示异常情况
    }

    float Vadc = (adc_value / 4095.0f) * NTC_Vref;
    float R_ntc = NTC_R_fixed * ((NTC_Vref / Vadc) - 1.0f);

    float tempK = 1.0f / ((1.0f / NTC_T0) + (1.0f / NTC_B) * log(R_ntc / NTC_R0));
    float tempC = tempK - 273.15f;

    return tempC;
}
