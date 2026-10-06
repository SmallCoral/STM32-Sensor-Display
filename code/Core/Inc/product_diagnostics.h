#ifndef PRODUCT_DIAGNOSTICS_H
#define PRODUCT_DIAGNOSTICS_H
#include <stdint.h>
/* Read-only runtime telemetry through J-Link; no UART wiring is required. */
typedef struct
{
  uint32_t magic, uptime_ms, adc_raw, ntc_status;
  int32_t temperature_c10;
  uint32_t flow_ml_min, pulses, rejected_pulses;
  uint32_t total_ml_low, total_ml_high, display_errors, adc_errors;
  uint32_t display_online, fahrenheit, display_updates, adc_online, flow_period_us;
} ProductDiagnostics;
extern volatile ProductDiagnostics g_product_diagnostics;
#endif
