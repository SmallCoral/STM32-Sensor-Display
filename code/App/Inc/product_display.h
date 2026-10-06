#ifndef PRODUCT_DISPLAY_H
#define PRODUCT_DISPLAY_H
#include "sensor_model.h"
void ProductDisplay_BuildFrame(const ProductReadings *readings, bool fahrenheit, uint16_t frame[8]);
bool ProductDisplay_Init(void);
bool ProductDisplay_Update(const ProductReadings *readings, bool fahrenheit);
#endif
