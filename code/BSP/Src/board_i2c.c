#include "board_i2c.h"

#include "stm32c5xx_ll_bus.h"
#include "stm32c5xx_ll_gpio.h"
#include "stm32c5xx_ll_i2c.h"
#include "stm32c5xx_ll_rcc.h"

#define I2C1_TIMING_144MHZ_100KHZ (0x80B25455U)
#define I2C_TIMEOUT_MS            (25U)
#define I2C_ERROR_FLAGS           (I2C_ISR_NACKF | I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR)

static void Board_I2C1_ConfigurePeripheral(void)
{
  LL_I2C_Disable(I2C1);
  LL_I2C_EnableAnalogFilter(I2C1);
  LL_I2C_SetDigitalFilter(I2C1, 0U);
  LL_I2C_SetMasterAddressingMode(I2C1, LL_I2C_ADDRESSING_MODE_7BIT);
  LL_I2C_SetTiming(I2C1, I2C1_TIMING_144MHZ_100KHZ);
  LL_I2C_Enable(I2C1);
}

static void Board_I2C1_ClearFlags(void)
{
  if (LL_I2C_IsActiveFlag_STOP(I2C1) != 0U)
  {
    LL_I2C_ClearFlag_STOP(I2C1);
  }
  if (LL_I2C_IsActiveFlag_NACK(I2C1) != 0U)
  {
    LL_I2C_ClearFlag_NACK(I2C1);
  }
  if (LL_I2C_IsActiveFlag_BERR(I2C1) != 0U)
  {
    LL_I2C_ClearFlag_BERR(I2C1);
  }
  if (LL_I2C_IsActiveFlag_ARLO(I2C1) != 0U)
  {
    LL_I2C_ClearFlag_ARLO(I2C1);
  }
  if (LL_I2C_IsActiveFlag_OVR(I2C1) != 0U)
  {
    LL_I2C_ClearFlag_OVR(I2C1);
  }
}

static bool Board_I2C1_WaitForFlag(uint32_t flag, bool active)
{
  uint32_t timeout_ms = I2C_TIMEOUT_MS;

  (void)SysTick->CTRL;

  for (;;)
  {
    const uint32_t status = LL_I2C_READ_REG(I2C1, ISR);
    const bool flag_is_active = ((status & flag) != 0U);

    if (flag_is_active == active)
    {
      return true;
    }

    if ((status & I2C_ERROR_FLAGS) != 0U)
    {
      return false;
    }

    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U)
    {
      if (timeout_ms == 0U)
      {
        return false;
      }
      --timeout_ms;
    }
  }
}

static void Board_I2C1_Recover(void)
{
  LL_I2C_Disable(I2C1);
  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);
  Board_I2C1_ConfigurePeripheral();
  Board_I2C1_ClearFlags();
}

void Board_I2C1_Init(void)
{
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);

  LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_6 | LL_GPIO_PIN_7, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_6, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_7, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_LOW);
  LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_LOW);
  LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_4);
  LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_7, LL_GPIO_AF_4);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);

  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);
  Board_I2C1_ConfigurePeripheral();
  Board_I2C1_ClearFlags();
}

bool Board_I2C1_Write(uint8_t address_7bit, const uint8_t *data, size_t length)
{
  if ((data == NULL) || (length == 0U) || (length > 255U) || (address_7bit > 0x7FU))
  {
    return false;
  }

  Board_I2C1_ClearFlags();
  if (!Board_I2C1_WaitForFlag(I2C_ISR_BUSY, false))
  {
    Board_I2C1_Recover();
    return false;
  }

  LL_I2C_HandleTransfer(I2C1,
                        (uint32_t)address_7bit << 1U,
                        LL_I2C_ADDRSLAVE_7BIT,
                        (uint32_t)length,
                        LL_I2C_MODE_AUTOEND,
                        LL_I2C_GENERATE_START_WRITE);

  for (size_t index = 0U; index < length; ++index)
  {
    if (!Board_I2C1_WaitForFlag(I2C_ISR_TXIS, true))
    {
      Board_I2C1_Recover();
      return false;
    }

    LL_I2C_TransmitData8(I2C1, data[index]);
  }

  if (!Board_I2C1_WaitForFlag(I2C_ISR_STOPF, true))
  {
    Board_I2C1_Recover();
    return false;
  }

  LL_I2C_ClearFlag_STOP(I2C1);
  return true;
}
