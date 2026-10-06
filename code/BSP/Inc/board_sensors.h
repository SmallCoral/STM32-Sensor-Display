#ifndef BOARD_SENSORS_H
#define BOARD_SENSORS_H
#include <stdbool.h>
#include <stdint.h>
typedef struct
{
  uint32_t pulses;
  uint32_t rejected_pulses;
  uint32_t period_us;
  bool flowing;
} BoardFlowSample;
void Board_Flow_Init(void);
BoardFlowSample Board_Flow_Read(void);
bool Board_NTC_Init(void);
bool Board_NTC_Read(uint16_t *sample);
void Board_Watchdog_Init(void);
void Board_Watchdog_Feed(void);
#endif
