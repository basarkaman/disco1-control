#ifndef RC_H
#define RC_H

#include <stdint.h>

#define NUM_CH 6

/* RC giris isleme + LED guncelleme task'ini yaratir.
   osKernelInitialize()'dan sonra, osKernelStart()'tan once cagrilmali. */
void RCInput_Init(void);

/* Son islenmis kanal pozisyon yuzdesi (0-100). channel: 0=CH1 ... 5=CH6.
   Diger modullerin pos_pct[] dizisine direkt erismemesi icin getter. */
float RCInput_GetPosPct(uint8_t channel);

#endif /* RC_H */
