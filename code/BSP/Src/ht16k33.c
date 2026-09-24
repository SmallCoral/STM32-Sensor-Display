#include "ht16k33.h"

#include "board_i2c.h"

#define HT16K33_ADDRESS_7BIT       (0x70U)
#define HT16K33_CMD_OSCILLATOR_ON  (0x21U)
#define HT16K33_CMD_ROW_OUTPUT     (0xA0U)
#define HT16K33_CMD_DISPLAY_OFF    (0x80U)
#define HT16K33_CMD_DISPLAY_ON     (0x81U)
#define HT16K33_CMD_BRIGHTNESS     (0xE0U)
#define HT16K33_RAM_START          (0x00U)
#define HT16K33_DEFAULT_BRIGHTNESS (15U)

static uint16_t display_frame[HT16K33_COM_COUNT];

static bool HT16K33_WriteCommand(uint8_t command)
{
  return Board_I2C1_Write(HT16K33_ADDRESS_7BIT, &command, 1U);
}

bool HT16K33_Init(void)
{
  if (!HT16K33_WriteCommand(HT16K33_CMD_OSCILLATOR_ON))
  {
    return false;
  }
  if (!HT16K33_WriteCommand(HT16K33_CMD_ROW_OUTPUT))
  {
    return false;
  }
  if (!HT16K33_WriteCommand(HT16K33_CMD_DISPLAY_OFF))
  {
    return false;
  }
  if (!HT16K33_SetBrightness(HT16K33_DEFAULT_BRIGHTNESS))
  {
    return false;
  }

  HT16K33_ClearFrame();
  if (!HT16K33_Update())
  {
    return false;
  }

  return HT16K33_WriteCommand(HT16K33_CMD_DISPLAY_ON);
}

void HT16K33_ClearFrame(void)
{
  for (uint32_t index = 0U; index < HT16K33_COM_COUNT; ++index)
  {
    display_frame[index] = 0U;
  }
}

void HT16K33_SetComRows(uint8_t com_index, uint16_t row_mask)
{
  if (com_index < HT16K33_COM_COUNT)
  {
    display_frame[com_index] = row_mask;
  }
}

bool HT16K33_Update(void)
{
  uint8_t transfer[1U + (HT16K33_COM_COUNT * 2U)];

  transfer[0] = HT16K33_RAM_START;
  for (uint32_t index = 0U; index < HT16K33_COM_COUNT; ++index)
  {
    transfer[1U + (index * 2U)] = (uint8_t)(display_frame[index] & 0x00FFU);
    transfer[2U + (index * 2U)] = (uint8_t)(display_frame[index] >> 8U);
  }

  return Board_I2C1_Write(HT16K33_ADDRESS_7BIT, transfer, sizeof(transfer));
}

bool HT16K33_SetBrightness(uint8_t level)
{
  if (level > 15U)
  {
    level = 15U;
  }

  return HT16K33_WriteCommand((uint8_t)(HT16K33_CMD_BRIGHTNESS | level));
}
