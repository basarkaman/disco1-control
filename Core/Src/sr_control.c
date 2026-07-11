#include "sr_control.h"
#include "main.h"
#include "cmsis_os.h"

static volatile uint8_t   s_sr_state = 0;
static SPI_HandleTypeDef *s_hspi     = NULL;
static osMutexId_t        s_mutex;
static const osMutexAttr_t s_mutex_attr = {
    .name = "srMutex",
};

#define LATCH_LOW()  HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_RESET)
#define LATCH_HIGH() HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_SET)

static void SR_Flush(void)
{
    uint8_t data = s_sr_state;
    LATCH_LOW();
    HAL_SPI_Transmit(s_hspi, &data, 1, HAL_MAX_DELAY);
    LATCH_HIGH();
}

void SR_Init(SPI_HandleTypeDef *hspi)
{
    s_hspi = hspi;
    s_mutex = osMutexNew(&s_mutex_attr);
    LATCH_HIGH();
}

void SR_SetBits(uint8_t mask, uint8_t value)
{
    osMutexAcquire(s_mutex, osWaitForever);
    s_sr_state = (s_sr_state & ~mask) | (value & mask);
    SR_Flush();
    osMutexRelease(s_mutex);
}

void SR_WriteN(const uint8_t *data, uint8_t n)
{
    osMutexAcquire(s_mutex, osWaitForever);
    LATCH_LOW();
    HAL_SPI_Transmit(s_hspi, (uint8_t *)data, n, HAL_MAX_DELAY);
    LATCH_HIGH();
    osMutexRelease(s_mutex);
}
