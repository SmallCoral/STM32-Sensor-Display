#ifndef BOARD_I2C_H
#define BOARD_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void Board_I2C1_Init(void);
bool Board_I2C1_Write(uint8_t address_7bit, const uint8_t *data, size_t length);

#endif /* BOARD_I2C_H */
