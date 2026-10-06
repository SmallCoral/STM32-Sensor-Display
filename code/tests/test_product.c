#include "sensor_model.h"
#include "product_display.h"
#include "ht16k33.h"
#include <assert.h>
#include <stdio.h>

/* Transport is tested on the board. These stubs isolate the frame and sensor calculations. */
bool HT16K33_Init(void) { return true; }
void HT16K33_ClearFrame(void) { }
void HT16K33_SetComRows(uint8_t com, uint16_t rows) { (void)com; (void)rows; }
bool HT16K33_Update(void) { return true; }
bool HT16K33_SetBrightness(uint8_t level) { (void)level; return true; }

static uint16_t adc_for(uint32_t ohms)
{
  return (uint16_t)((49900ULL * 4095U + (49900U + ohms) / 2U) / (49900U + ohms));
}

static void settle(SensorModel *m, uint16_t adc)
{
  for (unsigned i = 0; i < 100U; ++i) { SensorModel_UpdateTemperature(m, true, adc); }
}

static void test_ntc(void)
{
  int32_t t = 0;
  /* Independent golden points from the manufacturer's nominal R-T column. */
  assert(Sensor_NtcFromResistance(893990U, &t) == NTC_OK && t == -300);
  assert(Sensor_NtcFromResistance(163510U, &t) == NTC_OK && t == 0);
  assert(Sensor_NtcFromResistance(50000U, &t) == NTC_OK && t == 250);
  assert(Sensor_NtcFromResistance(17942U, &t) == NTC_OK && t == 500);
  assert(Sensor_NtcFromResistance(2850U, &t) == NTC_OK && t == 1050);
  assert(Sensor_NtcFromResistance(1000000U, &t) == NTC_TOO_COLD);
  assert(Sensor_NtcFromResistance(1000U, &t) == NTC_TOO_HOT);
  assert(Sensor_NtcFromADC(0U, &t) == NTC_OPEN);
  assert(Sensor_NtcFromADC(8U, &t) == NTC_OPEN);
  assert(Sensor_NtcFromADC(4087U, &t) == NTC_SHORT);
  assert(Sensor_NtcFromADC(4095U, &t) == NTC_SHORT);
  assert(Sensor_NtcFromADC(adc_for(50000U), &t) == NTC_OK && t >= 249 && t <= 251);
  int32_t previous = -301;
  for (unsigned adc = 9U; adc < 4087U; ++adc)
  {
    if (Sensor_NtcFromADC((uint16_t)adc, &t) == NTC_OK)
    {
      assert(t >= previous && t >= -300 && t <= 1050);
      previous = t;
    }
  }
  assert(Sensor_DisplayTemperature(250, true) == 77);
  assert(Sensor_DisplayTemperature(1000, true) == 212);
  assert(Sensor_DisplayTemperature(-50, false) == -5);
  assert(Sensor_DisplayTemperature(-55, false) == -6);
}

static void test_faults_and_hysteresis(void)
{
  SensorModel m;
  SensorModel_Init(&m);
  SensorModel_UpdateTemperature(&m, true, adc_for(50000U));
  assert(m.readings.temperature_status == NTC_STARTING);
  settle(&m, adc_for(50000U));
  assert(m.readings.temperature_status == NTC_OK && !m.readings.warning);
  SensorModel_UpdateTemperature(&m, true, 0U);
  SensorModel_UpdateTemperature(&m, true, 0U);
  assert(m.readings.temperature_status == NTC_OK);
  SensorModel_UpdateTemperature(&m, true, 0U);
  assert(m.readings.temperature_status == NTC_OPEN && m.readings.warning);
  settle(&m, adc_for(50000U));
  assert(m.readings.temperature_status == NTC_OK && !m.readings.warning);
  settle(&m, adc_for(5990U)); /* 81 C, manufacturer Rnor */
  assert(m.readings.warning && m.readings.hot_water);
  settle(&m, adc_for(6400U)); /* 79 C: inside the warning hysteresis band */
  assert(m.readings.warning);
  settle(&m, adc_for(6838U)); /* 77 C */
  assert(!m.readings.warning && m.readings.hot_water);
  settle(&m, adc_for(30035U)); /* 37 C */
  assert(!m.readings.hot_water);
  for (unsigned i = 0; i < 3U; ++i) { SensorModel_UpdateTemperature(&m, false, 0U); }
  assert(m.readings.temperature_status == NTC_ADC_ERROR);
}

static void test_flow(void)
{
  assert(Sensor_FlowMlMin(0U) == 0U);
  assert(Sensor_FlowMlMin(90909U) == 1000U); /* 11 Hz = 1 L/min */
  assert(Sensor_FlowMlMin(9091U) == 10000U); /* 110 Hz = 10 L/min */
  assert(Sensor_FlowMlMin(3636U) >= 25000U && Sensor_FlowMlMin(3636U) <= 25003U);
  SensorModel m;
  SensorModel_Init(&m);
  SensorModel_UpdateFlow(&m, 660U, 9091U, true);
  assert(m.readings.total_ml == 1000U && m.readings.flow_ml_min == 10000U);
  SensorModel_UpdateFlow(&m, 660U, 9091U, false);
  assert(m.readings.total_ml == 1000U && m.readings.flow_ml_min == 0U && !m.readings.flowing);
  m.last_pulses = 0xFFFFFFFEU;
  const uint64_t old = m.total_pulses;
  SensorModel_UpdateFlow(&m, 1U, 0U, false);
  assert(m.total_pulses == old + 3U);
}

static void test_frames(void)
{
  ProductReadings r = {0};
  uint16_t f[8];
  r.temperature_status = NTC_OK;
  r.temperature_c10 = 250;
  r.flow_ml_min = 3000U;
  r.flowing = true;
  ProductDisplay_BuildFrame(&r, false, f);
  assert(f[1] == 0x5BU && f[2] == 0x6DU); /* 25 */
  assert((f[3] & 0x7FU) == 0x3FU && (f[4] & 0x7FU) == 0x4FU); /* 03 */
  assert((f[5] & 0x7U) == 0x7U && (f[5] & 0x40U) != 0U);
  assert(f[6] == 0U && f[7] == 0U);
  r.temperature_c10 = 1000;
  ProductDisplay_BuildFrame(&r, true, f);
  assert((f[1] & 0x7FU) == 0x76U && (f[2] & 0x7FU) == 0x06U); /* HI, not 12 */
  r.temperature_c10 = -50;
  ProductDisplay_BuildFrame(&r, false, f);
  assert(f[1] == 0x40U && f[2] == 0x6DU);
  r.temperature_status = NTC_OPEN;
  r.warning = true;
  ProductDisplay_BuildFrame(&r, false, f);
  assert((f[1] & 0x7FU) == 0x79U && (f[2] & 0x7FU) == 0x50U);
  assert((f[1] & 0x200U) != 0U && (f[2] & 0x200U) != 0U);
  r.flow_ml_min = 100000U;
  ProductDisplay_BuildFrame(&r, false, f);
  assert((f[3] & 0x7FU) == 0x76U && (f[4] & 0x7FU) == 0x06U);
}

int main(void)
{
  test_ntc();
  test_faults_and_hysteresis();
  test_flow();
  test_frames();
  puts("PASS: NTC reference points/monotonicity, fault debounce/recovery, hysteresis, flow/volume/wrap, display frames");
  return 0;
}
