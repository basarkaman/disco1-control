#ifndef LINEAR_H
#define LINEAR_H

#include <stdint.h>

#define LINEAR_IDLE    0u
#define LINEAR_EXTEND  1u
#define LINEAR_RETRACT 2u

void Linear_Init(void);

/* PC modu: direkt motor komutu (LINEAR_IDLE / EXTEND / RETRACT). */
void Linear_SetCmd(uint8_t cmd);

#endif /* LINEAR_H */
