#include "main.h"
#include "board_i2c.h"
#include "board_sensors.h"
#include "board_time.h"
#include "board_user_button.h"
#include "product_display.h"
#include "product_diagnostics.h"
#include "system_clock.h"

volatile ProductDiagnostics g_product_diagnostics;
static SensorModel sensors;

int main(void)
{
  if (!SystemClock_Config()) { for (;;) { __NOP(); } }
  Board_Time_Init();
  Board_Watchdog_Init();
  Board_Flow_Init();
  Board_UserButton_Init();
  SensorModel_Init(&sensors);
  bool adc_online = Board_NTC_Init();
  Board_I2C1_Init();
  bool display_online = ProductDisplay_Init();
  bool fahrenheit = false;
  uint32_t last_button = Board_Time_Millis();
  uint32_t last_adc = last_button;
  uint32_t last_display = last_button;
  uint32_t last_adc_retry = last_button;
  uint32_t last_display_retry = last_button;
  g_product_diagnostics.magic = 0x53454E53U;

  for (;;)
  {
    const uint32_t now = Board_Time_Millis();
    if ((uint32_t)(now - last_button) >= 10U)
    {
      last_button = now;
      if (Board_UserButton_PollPress()) { fahrenheit = !fahrenheit; }
    }
    if (!adc_online && ((uint32_t)(now - last_adc_retry) >= 1000U))
    {
      last_adc_retry = now;
      adc_online = Board_NTC_Init();
    }
    if ((uint32_t)(now - last_adc) >= 50U)
    {
      last_adc = now;
      uint16_t raw = 0U;
      const bool ok = adc_online && Board_NTC_Read(&raw);
      if (!ok) { adc_online = false; ++g_product_diagnostics.adc_errors; }
      g_product_diagnostics.adc_raw = raw;
      SensorModel_UpdateTemperature(&sensors, ok, raw);
    }
    if (!display_online && ((uint32_t)(now - last_display_retry) >= 500U))
    {
      last_display_retry = now;
      Board_I2C1_Init();
      display_online = ProductDisplay_Init();
    }
    if ((uint32_t)(now - last_display) >= 200U)
    {
      last_display = now;
      const BoardFlowSample flow = Board_Flow_Read();
      SensorModel_UpdateFlow(&sensors, flow.pulses, flow.period_us, flow.flowing);
      if (display_online)
      {
        if (!ProductDisplay_Update(&sensors.readings, fahrenheit))
        {
          display_online = false;
          ++g_product_diagnostics.display_errors;
        }
        else { ++g_product_diagnostics.display_updates; }
      }
      g_product_diagnostics.pulses = flow.pulses;
      g_product_diagnostics.rejected_pulses = flow.rejected_pulses;
      g_product_diagnostics.flow_period_us = flow.period_us;
      g_product_diagnostics.total_ml_low = (uint32_t)sensors.readings.total_ml;
      g_product_diagnostics.total_ml_high = (uint32_t)(sensors.readings.total_ml >> 32U);
    }
    g_product_diagnostics.uptime_ms = now;
    g_product_diagnostics.ntc_status = sensors.readings.temperature_status;
    g_product_diagnostics.temperature_c10 = sensors.readings.temperature_c10;
    g_product_diagnostics.flow_ml_min = sensors.readings.flow_ml_min;
    g_product_diagnostics.display_online = display_online;
    g_product_diagnostics.adc_online = adc_online;
    g_product_diagnostics.fahrenheit = fahrenheit;
    Board_Watchdog_Feed();
    __WFI();
  }
}
