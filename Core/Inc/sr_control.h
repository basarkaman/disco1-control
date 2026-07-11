#ifndef SR_CONTROL_H
#define SR_CONTROL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

void SR_Init(SPI_HandleTypeDef *hspi);

/* Shadow byte'in verilen mask'e denk gelen bitlerini value ile degistirir,
   geri kalan bitlere dokunmaz, sonucu SR'a yazar. Mutex ile korunur -
   birden fazla task kendi mask'ine guvenle yazabilir. */
void SR_SetBits(uint8_t mask, uint8_t value);

/* Cok baytlik ham veriyi dogrudan SR zincirine yazar (shadow byte'i etkilemez). */
void SR_WriteN(const uint8_t *data, uint8_t n);

#endif /* SR_CONTROL_H */
