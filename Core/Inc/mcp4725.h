#ifndef MCP4725_H
#define MCP4725_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define MCP4725_ADDR  (0x60 << 1)  /* A0=GND varsayilan; A0=VCC ise 0x61 */
#define MCP4725_VREF  3.3f
#define MCP4725_MAX   4095u

/* I2C handle'ini baglar ve gorevi olusturur.
   osKernelInitialize()'dan sonra, osKernelStart()'tan once cagrilmali. */
void MCP4725_Init(I2C_HandleTypeDef *hi2c);

/* 0-4095 arasi ham DAC degeri yazar (bloke etmez, goreve kuyruklar). */
void MCP4725_SetRaw(uint16_t value);

/* 0.0f-3.3f arasi voltaj yazar. */
void MCP4725_SetVoltage(float volts);

#endif /* MCP4725_H */
