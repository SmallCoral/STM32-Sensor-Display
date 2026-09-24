#include "main.h"

#include "board_i2c.h"
#include "board_user_button.h"
#include "display_demo.h"
#include "system_clock.h"
#include "stm32c5xx_ll_utils.h"

#define DISPLAY_STEP_DELAY_MS  (100U)
#define DISPLAY_RETRY_DELAY_MS (500U)
#define BUTTON_POLL_DELAY_MS   (10U)
#define BUTTON_POLLS_PER_STEP  (DISPLAY_STEP_DELAY_MS / BUTTON_POLL_DELAY_MS)

int main(void)
{
  if (!SystemClock_Config())
  {
    for (;;)
    {
      __NOP();
    }
  }

  LL_Init1msTick(SystemCoreClock);
  Board_I2C1_Init();
  Board_UserButton_Init();

  while (!DisplayDemo_Init())
  {
    LL_Delay_NoISR(DISPLAY_RETRY_DELAY_MS);
    Board_I2C1_Init();
  }

  for (;;)
  {
    if (!DisplayDemo_Step())
    {
      LL_Delay_NoISR(DISPLAY_RETRY_DELAY_MS);
      Board_I2C1_Init();

      while (!DisplayDemo_Init())
      {
        LL_Delay_NoISR(DISPLAY_RETRY_DELAY_MS);
        Board_I2C1_Init();
      }
    }

    for (uint32_t poll = 0U; poll < BUTTON_POLLS_PER_STEP; ++poll)
    {
      if (Board_UserButton_PollPress())
      {
        DisplayDemo_ToggleTemperatureUnit();
      }

      LL_Delay_NoISR(BUTTON_POLL_DELAY_MS);
    }
  }
}
