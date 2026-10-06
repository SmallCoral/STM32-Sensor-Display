#ifndef BOARD_TIME_H
#define BOARD_TIME_H
#include <stdint.h>
void Board_Time_Init(void);
uint32_t Board_Time_Millis(void);
void Board_Time_DelayUs(uint32_t microseconds);
#endif
