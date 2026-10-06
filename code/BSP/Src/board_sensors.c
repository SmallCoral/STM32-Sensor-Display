#include "board_sensors.h"
#include "board_time.h"
#include "stm32c5xx_ll_adc.h"
#include "stm32c5xx_ll_bus.h"
#include "stm32c5xx_ll_exti.h"
#include "stm32c5xx_ll_gpio.h"
#include "stm32c5xx_ll_iwdg.h"
#include "stm32c5xx_ll_rcc.h"
#include "stm32c5xx_ll_tim.h"

#define FLOW_STOP_MS 2000U
#define FLOW_MIN_PERIOD_US 1500U
#define FLOW_PERIOD_COUNT 8U
#define ADC_TIMEOUT_MS 10U

static volatile uint32_t flow_pulses;
static volatile uint32_t rejected_pulses;
static volatile uint32_t last_edge_us;
static volatile uint32_t last_edge_ms;
static volatile uint32_t period_sum;
static volatile uint32_t periods[FLOW_PERIOD_COUNT];
static volatile uint8_t period_count;
static volatile uint8_t period_index;
static volatile bool edge_seen;

void Board_Flow_Init(void)
{
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_1, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_1, LL_GPIO_MODE_INPUT);
  LL_TIM_SetPrescaler(TIM2, (SystemCoreClock / 1000000U) - 1U);
  LL_TIM_SetAutoReload(TIM2, 0xFFFFFFFFU);
  LL_TIM_GenerateEvent_UPDATE(TIM2);
  LL_TIM_SetCounter(TIM2, 0U);
  LL_TIM_EnableCounter(TIM2);
  LL_EXTI_SetEXTISource(LL_EXTI_GPIO_PORTA, LL_EXTI_GPIO_LINE1);
  LL_EXTI_DisableFallingTrig_0_31(LL_EXTI_LINE_1);
  LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_1);
  LL_EXTI_ClearRisingFlag_0_31(LL_EXTI_LINE_1);
  LL_EXTI_ClearFallingFlag_0_31(LL_EXTI_LINE_1);
  LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_1);
  NVIC_SetPriority(EXTI1_IRQn, 1U);
  NVIC_EnableIRQ(EXTI1_IRQn);
}

void EXTI1_IRQHandler(void)
{
  if (LL_EXTI_IsActiveRisingFlag_0_31(LL_EXTI_LINE_1) != 0U)
  {
    LL_EXTI_ClearRisingFlag_0_31(LL_EXTI_LINE_1);
    const uint32_t now_us = LL_TIM_GetCounter(TIM2);
    const uint32_t now_ms = Board_Time_Millis();
    const uint32_t period = now_us - last_edge_us;
    if (edge_seen && ((uint32_t)(now_ms - last_edge_ms) < FLOW_STOP_MS))
    {
      if (period < FLOW_MIN_PERIOD_US)
      {
        ++rejected_pulses;
        return;
      }
      if (period_count == FLOW_PERIOD_COUNT) { period_sum -= periods[period_index]; }
      else { ++period_count; }
      periods[period_index] = period;
      period_sum += period;
      period_index = (uint8_t)((period_index + 1U) % FLOW_PERIOD_COUNT);
    }
    else
    {
      period_count = 0U;
      period_index = 0U;
      period_sum = 0U;
    }
    ++flow_pulses;
    last_edge_us = now_us;
    last_edge_ms = now_ms;
    edge_seen = true;
  }
}

BoardFlowSample Board_Flow_Read(void)
{
  BoardFlowSample sample;
  const uint32_t primask = __get_PRIMASK();
  __disable_irq();
  sample.pulses = flow_pulses;
  sample.rejected_pulses = rejected_pulses;
  sample.flowing = edge_seen && ((uint32_t)(Board_Time_Millis() - last_edge_ms) < FLOW_STOP_MS);
  sample.period_us = (sample.flowing && (period_count != 0U)) ? period_sum / period_count : 0U;
  __set_PRIMASK(primask);
  return sample;
}

static bool ADC_Wait(uint32_t (*flag)(const ADC_TypeDef *), bool expected)
{
  const uint32_t start = Board_Time_Millis();
  while ((flag(ADC1) != 0U) != expected)
  {
    if ((uint32_t)(Board_Time_Millis() - start) >= ADC_TIMEOUT_MS) { return false; }
  }
  return true;
}

bool Board_NTC_Init(void)
{
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA | LL_AHB2_GRP1_PERIPH_ADC12);
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);
  LL_AHB2_GRP1_ForceReset(LL_AHB2_GRP1_PERIPH_ADC12);
  LL_AHB2_GRP1_ReleaseReset(LL_AHB2_GRP1_PERIPH_ADC12);
  /* 144 MHz / 16 = 9 MHz; 289 cycles suits the high impedance divider. */
  LL_RCC_ConfigADCDAC(LL_RCC_ADCDAC_CLKSOURCE_HCLK, LL_RCC_ADCDAC_PRESCALER_16);
  LL_ADC_DisableDeepPowerDown(ADC1);
  LL_ADC_EnableInternalRegulator(ADC1);
  Board_Time_DelayUs(LL_ADC_DELAY_INTERNAL_REGUL_STAB_US + 10U);
  LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);
  LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);
  LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
  LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_DISABLE);
  LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_OVERWRITTEN);
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_289CYCLES);
  LL_ADC_SetChannelSingleDiff(ADC1, LL_ADC_CHANNEL_0, LL_ADC_IN_SINGLE_ENDED);
  LL_ADC_SetChannelPreselection(ADC1, LL_ADC_CHANNEL_0);
  LL_ADC_StartCalibration(ADC1, LL_ADC_IN_SINGLE_ENDED);
  if (!ADC_Wait(LL_ADC_IsCalibrationOnGoing, false)) { return false; }
  Board_Time_DelayUs(2U);
  LL_ADC_ClearFlag_ADRDY(ADC1);
  LL_ADC_Enable(ADC1);
  return ADC_Wait(LL_ADC_IsActiveFlag_ADRDY, true);
}

bool Board_NTC_Read(uint16_t *sample)
{
  uint32_t sum = 0U;
  for (uint32_t i = 0U; i < 16U; ++i)
  {
    LL_ADC_ClearFlag_EOC(ADC1);
    LL_ADC_REG_StartConversion(ADC1);
    if (!ADC_Wait(LL_ADC_IsActiveFlag_EOC, true)) { return false; }
    sum += LL_ADC_REG_ReadConversionData12(ADC1);
  }
  *sample = (uint16_t)((sum + 8U) / 16U);
  return true;
}

void Board_Watchdog_Init(void)
{
  DBGMCU->APB1LFZR |= DBGMCU_APB1LFZR_DBG_IWDG_STOP;
  LL_IWDG_Enable(IWDG);
  LL_IWDG_EnableWriteAccess(IWDG);
  LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_128);
  LL_IWDG_SetReloadCounter(IWDG, 2000U);
  LL_IWDG_SetWindow(IWDG, 0xFFFU);
  const uint32_t start = Board_Time_Millis();
  while ((LL_IWDG_IsReady(IWDG) == 0U) && ((uint32_t)(Board_Time_Millis() - start) < 100U)) { }
  LL_IWDG_ReloadCounter(IWDG);
}

void Board_Watchdog_Feed(void)
{
  LL_IWDG_ReloadCounter(IWDG);
}
