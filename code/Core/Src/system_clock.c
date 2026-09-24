#include "system_clock.h"

#include "stm32c5xx_ll_flash.h"
#include "stm32c5xx_ll_rcc.h"
#include "stm32c5xx_ll_utils.h"

#define SYSTEM_CLOCK_HZ          (144000000U)
#define CLOCK_STARTUP_LOOP_LIMIT (5000000U)

static bool SystemClock_WaitUntil(uint32_t (*ready_function)(void))
{
  volatile uint32_t timeout = CLOCK_STARTUP_LOOP_LIMIT;

  while ((ready_function() == 0U) && (timeout != 0U))
  {
    --timeout;
  }

  return (timeout != 0U);
}

bool SystemClock_Config(void)
{
  LL_RCC_HSE_Enable();
  if (!SystemClock_WaitUntil(LL_RCC_HSE_IsReady))
  {
    return false;
  }

  LL_RCC_ConfigPSI(LL_RCC_PSIFREQ_144MHZ,
                   LL_RCC_PSIREF_24MHZ,
                   LL_RCC_PSISOURCE_HSE);
  LL_RCC_PSIS_Enable();
  if (!SystemClock_WaitUntil(LL_RCC_PSIS_IsReady))
  {
    return false;
  }

  LL_RCC_ConfigBusClock(LL_RCC_HCLK_PRESCALER_1 |
                        LL_RCC_APB1_PRESCALER_1 |
                        LL_RCC_APB2_PRESCALER_1 |
                        LL_RCC_APB3_PRESCALER_1);

  LL_FLASH_SetLatency(FLASH, LL_FLASH_LATENCY_4WS);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PSIS);

  {
    volatile uint32_t timeout = CLOCK_STARTUP_LOOP_LIMIT;

    while ((LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PSIS) &&
           (timeout != 0U))
    {
      --timeout;
    }

    if (timeout == 0U)
    {
      return false;
    }
  }

  LL_FLASH_SetProgrammingDelay(FLASH, LL_FLASH_PROGRAM_DELAY_2);
  LL_SetSystemCoreClock(SYSTEM_CLOCK_HZ);
  return true;
}
