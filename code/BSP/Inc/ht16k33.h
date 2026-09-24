#ifndef HT16K33_H
#define HT16K33_H

#include <stdbool.h>
#include <stdint.h>

#define HT16K33_COM_COUNT  (8U)

bool HT16K33_Init(void);
void HT16K33_ClearFrame(void);
void HT16K33_SetComRows(uint8_t com_index, uint16_t row_mask);
bool HT16K33_Update(void);
bool HT16K33_SetBrightness(uint8_t level);

#endif /* HT16K33_H */
