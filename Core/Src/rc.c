#include "rc.h"
#include "main.h"        /* pin tanimlari (LDx_Pin, GPIO_PIN_x) + HAL */
#include "cmsis_os.h"

/* Task periyodu - RC sinyalleri ~50Hz (20ms) oldugundan 10ms fazlasiyla yeter */
#define RC_TASK_PERIOD_MS   10u

#define PULSE_US_MIN   800u
#define PULSE_US_MAX   2200u
#define PULSE_US_ZERO 1000.0f
#define PULSE_US_SCALE  10.0f

volatile uint32_t t_rise[NUM_CH]    = {0};
volatile uint32_t pulse_cyc[NUM_CH] = {0};
volatile uint8_t  ready[NUM_CH]     = {0};
volatile uint32_t pulse_us[NUM_CH]  = {0};
volatile float    pos_pct[NUM_CH]   = {0};

static const uint16_t led_pin[4] = { LD4_Pin, LD3_Pin, LD5_Pin, LD6_Pin };

static osThreadId_t rcTaskHandle;
static const osThreadAttr_t rcTask_attributes = {
  .name = "rcTask",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

static inline int ch_index(uint16_t pin)
{
  switch (pin) {
    case GPIO_PIN_4: return 0;
    case GPIO_PIN_5: return 1;
    case GPIO_PIN_6: return 2;
    case GPIO_PIN_7: return 3;
    case GPIO_PIN_8: return 4;
    case GPIO_PIN_9: return 5;
    default:         return -1;
  }
}

extern void user_pwm_setvalue(uint16_t value);

/* HAL'in weak EXTI callback'inin gercek implementasyonu.
   Proje genelinde sadece BURADA tanimli olmali (baska yerde tekrar
   tanimlarsan linker "multiple definition" hatasi verir). */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  uint32_t now = DWT->CYCCNT;          // EN BAŞTA oku (jitter'ı azaltır)
  int i = ch_index(GPIO_Pin);
  if (i < 0) return;

  if (HAL_GPIO_ReadPin(GPIOE, GPIO_Pin) == GPIO_PIN_SET) {
    t_rise[i] = now;                   // yükselen
  } else {
    pulse_cyc[i] = now - t_rise[i];    // düşen — math yok, sadece fark
    ready[i] = 1;
  }
}

/* ---------------------------------------------------------------------
 * RC_ProcessChannels
 * EXTI callback'in doldurduğu pulse_cyc[] degerlerini yüzdeye çevirir.
 * ------------------------------------------------------------------- */
static void RC_ProcessChannels(void)
{
    for (int i = 0; i < NUM_CH; i++) {
        if (!ready[i]) continue;
        ready[i] = 0;

        uint32_t us = pulse_cyc[i] / 168u;   // 168 MHz -> µs

        if (us >= PULSE_US_MIN && us <= PULSE_US_MAX) {   // servo araligi disini ele (glitch)
            pulse_us[i] = us;
            float p = ((float)us - PULSE_US_ZERO) / PULSE_US_SCALE; // 1000us=%0, 2000us=%100
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
  rcTaskHandle = osThreadNew(StartRCTask, NULL, &rcTask_attributes);
}
