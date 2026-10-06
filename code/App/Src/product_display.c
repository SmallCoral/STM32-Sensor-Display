#include "product_display.h"
#include "ht16k33.h"

#define A (1U << 0)
#define B (1U << 1)
#define C (1U << 2)
#define D (1U << 3)
#define E (1U << 4)
#define F (1U << 5)
#define G (1U << 6)
#define H (1U << 7)
#define I (1U << 8)
#define J (1U << 9)
static const uint16_t digits[10] = {
  A|B|C|D|E|F, B|C, A|B|D|E|G, A|B|C|D|G, B|C|F|G,
  A|C|D|F|G, A|C|D|E|F|G, A|B|C, A|B|C|D|E|F|G, A|B|C|D|F|G
};

void ProductDisplay_BuildFrame(const ProductReadings *r, bool fahrenheit, uint16_t frame[8])
{
  for (unsigned i = 0U; i < 8U; ++i) { frame[i] = 0U; }
  frame[5] = fahrenheit ? D : G;
  if (r->temperature_status == NTC_STARTING)
  {
    frame[1] = G; frame[2] = G;
  }
  else if (r->temperature_status == NTC_TOO_COLD)
  {
    frame[1] = D|E|F; frame[2] = C|D|E|G; /* Lo */
  }
  else if (r->temperature_status == NTC_TOO_HOT)
  {
    frame[1] = B|C|E|F|G; frame[2] = B|C; /* HI */
  }
  else if (r->temperature_status != NTC_OK)
  {
    frame[1] = A|D|E|F|G; frame[2] = E|G; /* Er */
  }
  else
  {
    const int32_t value = Sensor_DisplayTemperature(r->temperature_c10, fahrenheit);
    if (value > 199) { frame[1] = B|C|E|F|G; frame[2] = B|C; }
    else if (value < -9) { frame[1] = D|E|F; frame[2] = C|D|E|G; }
    else if (value < 0) { frame[1] = G; frame[2] = digits[-value]; }
    else
    {
      frame[1] = (value >= 10) ? digits[(value / 10) % 10] : 0U;
      frame[2] = digits[value % 10];
      if (value >= 100) { frame[5] |= E|F; }
    }
    int32_t progress = (r->temperature_c10 * 9 + 500) / 1000;
    if (progress < 0) { progress = 0; }
    if (progress > 9) { progress = 9; }
    frame[0] = (uint16_t)((1U << progress) - 1U);
  }

  if (r->hot_water) { frame[1] |= H|I; frame[2] |= H|I; frame[3] |= H|I; }
  if (r->warning) { frame[1] |= J; frame[2] |= J; }
  const uint32_t flow = (r->flow_ml_min + 500U) / 1000U;
  if (flow > 99U) { frame[3] |= B|C|E|F|G; frame[4] = B|C; }
  else { frame[3] |= digits[flow / 10U]; frame[4] = digits[flow % 10U]; }
  /* Keep WATER FLOW and L/min labels visible even at zero flow. */
  frame[4] |= H|I|J;
  frame[5] |= H|I;
  if (r->flowing) { frame[5] |= A|B|C; }
}

bool ProductDisplay_Init(void) { return HT16K33_Init(); }
bool ProductDisplay_Update(const ProductReadings *readings, bool fahrenheit)
{
  uint16_t frame[8];
  ProductDisplay_BuildFrame(readings, fahrenheit, frame);
  HT16K33_ClearFrame();
  for (uint8_t i = 0U; i < 8U; ++i) { HT16K33_SetComRows(i, frame[i]); }
  return HT16K33_Update();
}
