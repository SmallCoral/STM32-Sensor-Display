#include "board_time.h"
#include "stm32c5xx.h"

static volatile uint32_t milliseconds;

void Board_Time_Init(void)
{
  milliseconds = 0U;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  (void)SysTick_Config(SystemCoreClock / 1000U);
  NVIC_SetPriority(SysTick_IRQn, 3U);
}

void SysTick_Handler(void)
{
  ++milliseconds;
}

uint32_t Board_Time_Millis(void)
{
  return milliseconds;
}

void Board_Time_DelayUs(uint32_t microseconds)
{
  const uint32_t start = DWT->CYCCNT;
  const uint32_t cycles = microseconds * (SystemCoreClock / 1000000U);
  while ((uint32_t)(DWT->CYCCNT - start) < cycles) { __NOP(); }
}
