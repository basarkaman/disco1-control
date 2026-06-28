#ifndef RC_H
#define RC_H

#include <stdint.h>

#define NUM_CH 10

/* 0 = RC kumanda modu, 1 = PC kontrol modu */
extern volatile uint8_t g_pc_mode;

void RCInput_Init(void);
float RCInput_GetPosPct(uint8_t channel);

/* PC modunda komut alındığında ilgili LED'i 200ms yakar. led_idx: 0-3 */
void RC_FlashLED(uint8_t led_idx);

#endif /* RC_H */
