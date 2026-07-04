#ifndef SR_CONTROL_H
#define SR_CONTROL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

/* Shift register'in güncel çıkış değeri. Doğrudan değiştirip SR_Flush()
   ile veya SR_Write() ile SR'a gönderilebilir. */
extern volatile uint8_t g_sr_state;

void SR_Init(SPI_HandleTypeDef *hspi);

/* data'yı yazar ve g_sr_state'i günceller. */
void SR_Write(uint8_t data);

/* g_sr_state'i olduğu gibi SR'a gönderir. */
void SR_Flush(void);

void SR_WriteN(const uint8_t *data, uint8_t n);

#endif /* SR_CONTROL_H */
