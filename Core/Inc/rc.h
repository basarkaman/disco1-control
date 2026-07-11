#ifndef RC_H
#define RC_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_CH 10

void RCInput_Init(void);
float RCInput_GetPosPct(uint8_t channel);

/* RC_FAILSAFE_TIMEOUT_MS icinde en az bir gecerli iBUS frame alindiysa true. */
bool RCInput_LinkOk(void);

#endif /* RC_H */
