#ifndef STEP_CONTROL_H
#define STEP_CONTROL_H

#include <stdint.h>

void StepControl_Init(void);

/* RC modu: hedef hiz yüzdesi (0-100), yön implicit. */
void StepControl_SetTarget(float pct);

/* PC modu: dir=0/1 yön, angle=0-8191 hız (0=dur). */
void StepControl_SetDirAngle(uint8_t dir, uint16_t angle);

#endif /* STEP_CONTROL_H */
