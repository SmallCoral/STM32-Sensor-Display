#include "display_demo.h"

#include "ht16k33.h"

#include <stdint.h>

#define SEG_A  (1U << 0U)
#define SEG_B  (1U << 1U)
#define SEG_C  (1U << 2U)
#define SEG_D  (1U << 3U)
#define SEG_E  (1U << 4U)
#define SEG_F  (1U << 5U)
#define SEG_G  (1U << 6U)
#define SEG_H  (1U << 7U)
#define SEG_I  (1U << 8U)
#define SEG_J  (1U << 9U)

#define PROGRESS_SEGMENT_COUNT  (9U)
#define DEMO_MAX_VALUE          (100U)
#define DEMO_MAX_HOLD_STEPS     (10U)
#define HOT_WATER_TEMPERATURE_C (40U)
#define WARNING_TEMPERATURE_C   (80U)

#define PROGRESS_ALL_MASK       (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | \
                                 SEG_F | SEG_G | SEG_H | SEG_I)
#define TEXT_PAIR_MASK          (SEG_H | SEG_I)
#define WARNING_MASK            (SEG_J)
#define WORK_MASK               (SEG_A | SEG_B | SEG_C)
#define FAHRENHEIT_MASK         (SEG_D)
#define HUNDREDS_ONE_MASK       (SEG_E | SEG_F)
#define CELSIUS_MASK            (SEG_G)
#define FLOW_TEXT_MASK          (SEG_H | SEG_I)
#define FLOW_UNIT_MASK          (SEG_J)

#define GLYPH_H                 (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define GLYPH_I                 (SEG_B | SEG_C)

/* Screen COM1..COM6 are wired to HT16K33 COM0..COM5. */
enum
{
  COM_PROGRESS = 0U,
  COM_TOP_TENS,
  COM_TOP_ONES,
  COM_BOTTOM_TENS,
  COM_BOTTOM_ONES,
  COM_LABELS
};

static const uint16_t digit_segments[10] =
{
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
  SEG_B | SEG_C,
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
  SEG_B | SEG_C | SEG_F | SEG_G,
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  SEG_A | SEG_B | SEG_C,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G
};

static uint8_t demo_value;
static bool demo_is_rising;
static uint8_t demo_max_hold_steps;
static bool display_fahrenheit;

static uint16_t DisplayDemo_GetProgressMask(uint8_t value)
{
  const uint32_t lit_segments = (((uint32_t)value * PROGRESS_SEGMENT_COUNT) +
                                 (DEMO_MAX_VALUE / 2U)) / DEMO_MAX_VALUE;

  if (lit_segments == 0U)
  {
    return 0U;
  }

  return (uint16_t)(((1UL << lit_segments) - 1UL) & PROGRESS_ALL_MASK);
}

static uint16_t DisplayDemo_CelsiusToFahrenheit(uint8_t celsius)
{
  return (uint16_t)((((uint16_t)celsius * 9U) + 2U) / 5U) + 32U;
}

static bool DisplayDemo_Render(uint8_t temperature_c)
{
  const uint8_t flow_value = (uint8_t)(temperature_c % 100U);
  const uint16_t displayed_temperature = display_fahrenheit ?
                                         DisplayDemo_CelsiusToFahrenheit(temperature_c) :
                                         temperature_c;
  const bool temperature_out_of_range = (displayed_temperature > 199U);
  const bool has_flow = (temperature_c > 0U);
  const bool hot_water = (temperature_c >= HOT_WATER_TEMPERATURE_C);
  const bool temperature_warning = (temperature_c >= WARNING_TEMPERATURE_C);
  uint16_t top_tens = temperature_out_of_range ?
                      GLYPH_H : digit_segments[(displayed_temperature / 10U) % 10U];
  uint16_t top_ones = temperature_out_of_range ?
                      GLYPH_I : digit_segments[displayed_temperature % 10U];
  uint16_t bottom_tens = digit_segments[flow_value / 10U];
  uint16_t bottom_ones = digit_segments[flow_value % 10U];
  uint16_t bottom_ones_extras = 0U;
  uint16_t labels = display_fahrenheit ? FAHRENHEIT_MASK : CELSIUS_MASK;

  if (hot_water)
  {
    top_tens |= TEXT_PAIR_MASK;
    top_ones |= TEXT_PAIR_MASK;
    bottom_tens |= TEXT_PAIR_MASK;
  }

  if (temperature_warning)
  {
    top_tens |= WARNING_MASK;
    top_ones |= WARNING_MASK;
  }

  if (has_flow)
  {
    bottom_ones_extras |= FLOW_TEXT_MASK | FLOW_UNIT_MASK;
    labels |= WORK_MASK | FLOW_TEXT_MASK;
  }

  if (!temperature_out_of_range && (displayed_temperature >= 100U))
  {
    labels |= HUNDREDS_ONE_MASK;
  }

  HT16K33_ClearFrame();
  HT16K33_SetComRows(COM_PROGRESS, DisplayDemo_GetProgressMask(temperature_c));
  HT16K33_SetComRows(COM_TOP_TENS, top_tens);
  HT16K33_SetComRows(COM_TOP_ONES, top_ones);
  HT16K33_SetComRows(COM_BOTTOM_TENS, bottom_tens);
  HT16K33_SetComRows(COM_BOTTOM_ONES, bottom_ones | bottom_ones_extras);
  HT16K33_SetComRows(COM_LABELS, labels);
  return HT16K33_Update();
}

bool DisplayDemo_Init(void)
{
  demo_value = 0U;
  demo_is_rising = true;
  demo_max_hold_steps = 0U;
  display_fahrenheit = false;
  return HT16K33_Init();
}

void DisplayDemo_ToggleTemperatureUnit(void)
{
  display_fahrenheit = !display_fahrenheit;
}

bool DisplayDemo_Step(void)
{
  if (!DisplayDemo_Render(demo_value))
  {
    return false;
  }

  if (demo_is_rising)
  {
    if (demo_value >= DEMO_MAX_VALUE)
    {
      if (++demo_max_hold_steps >= DEMO_MAX_HOLD_STEPS)
      {
        demo_max_hold_steps = 0U;
        demo_is_rising = false;
        --demo_value;
      }
    }
    else
    {
      ++demo_value;
    }
  }
  else if (demo_value == 0U)
  {
    demo_is_rising = true;
    ++demo_value;
  }
  else
  {
    --demo_value;
  }

  return true;
}
