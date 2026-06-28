#ifndef SR_CONTROL_H
#define SR_CONTROL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

void SR_Init(SPI_HandleTypeDef *hspi);
void SR_Write(uint8_t data);
void SR_WriteN(const uint8_t *data, uint8_t n);

#endif /* SR_CONTROL_H */
