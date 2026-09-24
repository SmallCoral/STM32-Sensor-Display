#include "board_user_button.h"

#include "stm32c5xx_ll_bus.h"
#include "stm32c5xx_ll_gpio.h"

#include <stdint.h>

#define USER_BUTTON_PIN              (LL_GPIO_PIN_13)
#define USER_BUTTON_DEBOUNCE_SAMPLES (3U)

static bool stable_pressed;
static bool candidate_pressed;
static uint8_t candidate_samples;

static bool Board_UserButton_IsPressed(void)
{
  return (LL_GPIO_IsInputPinSet(GPIOC, USER_BUTTON_PIN) == 0U);
}

void Board_UserButton_Init(void)
{
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_GPIO_SetPinPull(GPIOC, USER_BUTTON_PIN, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinMode(GPIOC, USER_BUTTON_PIN, LL_GPIO_MODE_INPUT);

  stable_pressed = Board_UserButton_IsPressed();
  candidate_pressed = stable_pressed;
  candidate_samples = 0U;
}

bool Board_UserButton_PollPress(void)
{
  const bool pressed = Board_UserButton_IsPressed();

  if (pressed != candidate_pressed)
  {
    candidate_pressed = pressed;
    candidate_samples = 0U;
    return false;
  }

  if (candidate_samples < USER_BUTTON_DEBOUNCE_SAMPLES)
  {
    ++candidate_samples;
  }

  if ((candidate_samples >= USER_BUTTON_DEBOUNCE_SAMPLES) &&
      (stable_pressed != candidate_pressed))
  {
    stable_pressed = candidate_pressed;
    return stable_pressed;
  }

  return false;
}
