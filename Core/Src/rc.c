#include "rc.h"
#include "main.h"        /* pin tanimlari (LDx_Pin, GPIO_PIN_x) + HAL */
#include "usart.h"       /* huart2 (CubeMX tarafindan uretilir) */
#include "cmsis_os.h"

/* Task periyodu - iBUS ~50Hz (her ~7ms frame) oldugundan 10ms yeterli */
#define RC_TASK_PERIOD_MS   10u

/* iBUS protokol sabitleri */
#define IBUS_FRAME_LEN   32u
#define IBUS_HEADER_LEN  0x20u
#define IBUS_HEADER_CMD  0x40u
#define IBUS_NUM_CH      14u

#define IBUS_VAL_MIN   800u
#define IBUS_VAL_MAX   2200u
#define IBUS_VAL_ZERO  1000.0f
#define IBUS_VAL_SCALE   10.0f

volatile uint16_t ibus_val[NUM_CH]  = {0};
volatile float    pos_pct[NUM_CH]   = {0};
volatile uint8_t  ready[NUM_CH]     = {0};

static uint8_t ibusBuf[IBUS_FRAME_LEN];

static const uint16_t led_pin[4] = { LD4_Pin, LD3_Pin, LD5_Pin, LD6_Pin };

static osThreadId_t rcTaskHandle;
static const osThreadAttr_t rcTask_attributes = {
  .name = "rcTask",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* ---------------------------------------------------------------------
 * iBUS checksum dogrulama: 0xFFFF - (ilk 30 byte toplami) == son 2 byte
 * ------------------------------------------------------------------- */
static int IBUS_ValidateChecksum(const uint8_t *buf)
{
    uint16_t sum = 0xFFFF;
    for (uint32_t i = 0; i < IBUS_FRAME_LEN - 2; i++) {
        sum -= buf[i];
    }
    uint16_t rx_chk = (uint16_t)buf[IBUS_FRAME_LEN - 2] |
                       ((uint16_t)buf[IBUS_FRAME_LEN - 1] << 8);
    return (sum == rx_chk);
}

/* HAL'in weak RxEvent callback'inin gercek implementasyonu.
   Proje genelinde sadece BURADA tanimli olmali (baska yerde tekrar
   tanimlarsan linker "multiple definition" hatasi verir). */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance != USART2) {
        return;
    }

    if (Size == IBUS_FRAME_LEN &&
        ibusBuf[0] == IBUS_HEADER_LEN &&
        ibusBuf[1] == IBUS_HEADER_CMD &&
        IBUS_ValidateChecksum(ibusBuf))
    {
        for (uint32_t ch = 0; ch < NUM_CH && ch < IBUS_NUM_CH; ch++) {
            ibus_val[ch] = (uint16_t)ibusBuf[2 + ch * 2] |
                           ((uint16_t)ibusBuf[3 + ch * 2] << 8);
            ready[ch] = 1;
        }
    }

    /* DMA'yi yeni frame icin yeniden tetikle */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, ibusBuf, IBUS_FRAME_LEN);
    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
}

/* ---------------------------------------------------------------------
 * RC_ProcessChannels
 * Callback'in doldurdugu ibus_val[] degerlerini yüzdeye çevirir.
 * ------------------------------------------------------------------- */
static void RC_ProcessChannels(void)
{
    for (int i = 0; i < NUM_CH; i++) {
        if (!ready[i]) continue;
        uint16_t val = ibus_val[i];
        ready[i] = 0;
        if (val >= IBUS_VAL_MIN && val <= IBUS_VAL_MAX) {   // gecersiz/glitch frame'leri ele
            float p = ((float)val - IBUS_VAL_ZERO) / IBUS_VAL_SCALE;
            if (p < 0)   p = 0;
            if (p > 100) p = 100;
            pos_pct[i] = p;
        }
    }
}

/* ---------------------------------------------------------------------
 * RC_UpdateLEDs
 * Ilk 4 kanalin pozisyonuna gore LED'leri günceller.
 * ------------------------------------------------------------------- */
static void RC_UpdateLEDs(void)
{
    for (int i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(GPIOD, led_pin[i],
            (pos_pct[i] > 50.0f) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

float RCInput_GetPosPct(uint8_t channel)
{
    if (channel >= NUM_CH) return 0.0f;
    return pos_pct[channel];
}

static void StartRCTask(void *argument)
{
  (void)argument;
  for(;;)
  {
    RC_ProcessChannels();
    RC_UpdateLEDs();
    osDelay(RC_TASK_PERIOD_MS);
  }
}

void RCInput_Init(void)
{
  /* USART2 DMA dinlemeyi baslat */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, ibusBuf, IBUS_FRAME_LEN);
  __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);

  rcTaskHandle = osThreadNew(StartRCTask, NULL, &rcTask_attributes);
}