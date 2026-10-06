#ifndef SENSOR_MODEL_H
#define SENSOR_MODEL_H
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  NTC_OK = 0, NTC_STARTING, NTC_OPEN, NTC_SHORT, NTC_TOO_COLD, NTC_TOO_HOT, NTC_ADC_ERROR
} NtcStatus;
typedef struct
{
  int32_t temperature_c10;
  uint32_t flow_ml_min;
  uint64_t total_ml;
  NtcStatus temperature_status;
  bool flowing;
  bool hot_water;
  bool warning;
} ProductReadings;
typedef struct
{
  ProductReadings readings;
  int32_t filtered_adc_q8;
  NtcStatus candidate_status;
  uint8_t candidate_count;
  bool filter_ready;
  uint32_t last_pulses;
  uint64_t total_pulses;
} SensorModel;

NtcStatus Sensor_NtcFromResistance(uint32_t ohms, int32_t *celsius10);
NtcStatus Sensor_NtcFromADC(uint16_t adc, int32_t *celsius10);
uint32_t Sensor_FlowMlMin(uint32_t period_us);
int32_t Sensor_DisplayTemperature(int32_t celsius10, bool fahrenheit);
void SensorModel_Init(SensorModel *model);
void SensorModel_UpdateTemperature(SensorModel *model, bool adc_ok, uint16_t adc);
void SensorModel_UpdateFlow(SensorModel *model, uint32_t pulses, uint32_t period_us, bool flowing);
#endif
