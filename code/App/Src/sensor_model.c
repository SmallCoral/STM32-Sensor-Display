#include "sensor_model.h"
#include <string.h>

/* Manufacturer 180长规格书(1).pdf, Rnor column, -30..105 C at 1 C intervals. */
static const uint32_t ntc_ohms[] = {
  893990U, 839801U, 789253U, 742079U, 698033U, 656888U, 618434U, 582480U,
  548847U, 517372U, 487902U, 460298U, 434431U, 410180U, 387436U, 366094U,
  346062U, 327250U, 309577U, 292967U, 277351U, 262662U, 248841U, 235831U,
  223580U, 212039U, 201163U, 190910U, 181241U, 172119U, 163510U, 155382U,
  147706U, 140454U, 133601U, 127121U, 120994U, 115197U, 109711U, 104518U,
  99600U, 94942U, 90528U, 86344U, 82377U, 78614U, 75045U, 71657U,
  68441U, 65387U, 62487U, 59731U, 57111U, 54621U, 52253U, 50000U,
  47857U, 45817U, 43875U, 42025U, 40264U, 38585U, 36986U, 35461U,
  34007U, 32620U, 31298U, 30035U, 28830U, 27680U, 26582U, 25532U,
  24530U, 23572U, 22656U, 21781U, 20944U, 20143U, 19377U, 18644U,
  17942U, 17270U, 16627U, 16011U, 15420U, 14855U, 14313U, 13793U,
  13295U, 12817U, 12358U, 11919U, 11497U, 11092U, 10703U, 10330U,
  9971U, 9627U, 9296U, 8978U, 8673U, 8379U, 8097U, 7825U,
  7564U, 7313U, 7071U, 6838U, 6614U, 6399U, 6191U, 5991U,
  5799U, 5614U, 5435U, 5263U, 5097U, 4937U, 4783U, 4634U,
  4490U, 4352U, 4219U, 4090U, 3966U, 3846U, 3730U, 3618U,
  3510U, 3406U, 3305U, 3208U, 3114U, 3023U, 2935U, 2850U,
};

NtcStatus Sensor_NtcFromResistance(uint32_t ohms, int32_t *celsius10)
{
  if (ohms > ntc_ohms[0]) { return NTC_TOO_COLD; }
  const unsigned last = (unsigned)(sizeof(ntc_ohms) / sizeof(ntc_ohms[0]) - 1U);
  if (ohms < ntc_ohms[last]) { return NTC_TOO_HOT; }
  if (ohms == ntc_ohms[last]) { *celsius10 = 1050; return NTC_OK; }
  for (unsigned i = 0U; i < last; ++i)
  {
    if (ohms >= ntc_ohms[i + 1U])
    {
      const uint32_t span = ntc_ohms[i] - ntc_ohms[i + 1U];
      *celsius10 = ((int32_t)i - 30) * 10 +
        (int32_t)(((ntc_ohms[i] - ohms) * 10U + span / 2U) / span);
      return NTC_OK;
    }
  }
  return NTC_ADC_ERROR;
}

NtcStatus Sensor_NtcFromADC(uint16_t adc, int32_t *celsius10)
{
  if (adc <= 8U) { return NTC_OPEN; }
  if (adc >= 4087U) { return NTC_SHORT; }
  /* NTC is above the 49.9 kohm resistor; excitation and ADC reference share +3V3. */
  const uint32_t ohms = (uint32_t)(((uint64_t)49900U * (4095U - adc) + adc / 2U) / adc);
  return Sensor_NtcFromResistance(ohms, celsius10);
}

uint32_t Sensor_FlowMlMin(uint32_t period_us)
{
  if (period_us == 0U) { return 0U; }
  /* F=11Q: 660 pulses/litre. Result retains ml/min resolution until display rounding. */
  const uint64_t denominator = (uint64_t)660U * period_us;
  return (uint32_t)((60000000000ULL + denominator / 2U) / denominator);
}

int32_t Sensor_DisplayTemperature(int32_t celsius10, bool fahrenheit)
{
  const int32_t tenths = fahrenheit ? (celsius10 * 9 / 5) + 320 : celsius10;
  return (tenths < 0) ? -((-tenths + 5) / 10) : (tenths + 5) / 10;
}

void SensorModel_Init(SensorModel *m)
{
  memset(m, 0, sizeof(*m));
  m->readings.temperature_status = NTC_STARTING;
  m->candidate_status = NTC_STARTING;
}

void SensorModel_UpdateTemperature(SensorModel *m, bool adc_ok, uint16_t adc)
{
  int32_t temperature = 0;
  const NtcStatus status = adc_ok ? Sensor_NtcFromADC(adc, &temperature) : NTC_ADC_ERROR;
  if (status != m->candidate_status) { m->candidate_status = status; m->candidate_count = 0U; }
  if (m->candidate_count < 3U) { ++m->candidate_count; }
  if (m->candidate_count < 3U) { return; }
  m->readings.temperature_status = status;
  if (status != NTC_OK)
  {
    m->filter_ready = false;
    m->readings.hot_water = false;
    m->readings.warning = (status != NTC_STARTING) && (status != NTC_TOO_COLD);
    return;
  }
  if (!m->filter_ready) { m->filtered_adc_q8 = (int32_t)adc * 256; m->filter_ready = true; }
  else { m->filtered_adc_q8 += ((int32_t)adc * 256 - m->filtered_adc_q8) / 8; }
  (void)Sensor_NtcFromADC((uint16_t)((m->filtered_adc_q8 + 128) / 256), &temperature);
  m->readings.temperature_c10 = temperature;
  /* 2 C hysteresis keeps labels steady at threshold boundaries. */
  m->readings.hot_water = m->readings.hot_water ? (temperature >= 380) : (temperature >= 400);
  m->readings.warning = m->readings.warning ? (temperature >= 780) : (temperature >= 800);
}

void SensorModel_UpdateFlow(SensorModel *m, uint32_t pulses, uint32_t period_us, bool flowing)
{
  m->total_pulses += (uint32_t)(pulses - m->last_pulses);
  m->last_pulses = pulses;
  m->readings.total_ml = (m->total_pulses * 1000U) / 660U;
  m->readings.flowing = flowing;
  m->readings.flow_ml_min = flowing ? Sensor_FlowMlMin(period_us) : 0U;
}
