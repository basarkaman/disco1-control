#include "sr_control.h"
#include "main.h"

volatile uint8_t g_sr_state = 0;

static SPI_HandleTypeDef *s_hspi = NULL;

#define LATCH_LOW()  HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_RESET)
#define LATCH_HIGH() HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_SET)

void SR_Init(SPI_HandleTypeDef *hspi)
{
    s_hspi = hspi;
    LATCH_HIGH();
}

void SR_Write(uint8_t data)
{
    g_sr_state = data;
    LATCH_LOW();
    HAL_SPI_Transmit(s_hspi, &data, 1, HAL_MAX_DELAY);
    LATCH_HIGH();
}

void SR_Flush(void)
{
    LATCH_LOW();
    HAL_SPI_Transmit(s_hspi, (uint8_t *)&g_sr_state, 1, HAL_MAX_DELAY);
    LATCH_HIGH();
}

void SR_WriteN(const uint8_t *data, uint8_t n)
{
    LATCH_LOW();
    HAL_SPI_Transmit(s_hspi, (uint8_t *)data, n, HAL_MAX_DELAY);
    LATCH_HIGH();
}
