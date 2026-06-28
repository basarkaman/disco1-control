#include "step_control.h"
#include "rc.h"
#include "cmsis_os2.h"
#include "main.h"
#include "cmsis_os.h"
#include <math.h>

#define STEP_TASK_PERIOD_MS  5u

extern volatile float pos_pct[];

static volatile float   s_step_target = 50.0f;
static volatile uint8_t s_step_dir    = 0;

uint32_t remaining_delay;
const uint32_t POLL_MS = 5;

static osThreadId_t stepTaskHandle;
static const osThreadAttr_t stepTask_attributes = {
  .name = "stepTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

extern void set_arr(int arr);
extern TIM_HandleTypeDef htim3;

#define STEP_TIM_CHANNEL   TIM_CHANNEL_3   // kendi timer kanalına göre değiştir
#define STEP_ARR_VALUE     999             // hareket halindeki periyot (frekansı burada ayarla)
#define STEP_DUTY_PERCENT  50              // step pulse duty cycle (%)

#define MIN_FREQ_HZ    100      // %0 hız -> en düşük frekans
#define MAX_FREQ_HZ    1000    // %100 hız -> en yüksek frekans
#define COUNTER_CLOCK  1000000UL  // PSC ile elde ettiğin sayıcı clock'u (örn. 1 MHz)

uint32_t SpeedPercentToARR(float speed_percent)
{
    if(speed_percent < 0.0f)   speed_percent = 0.0f;
    if(speed_percent > 100.0f) speed_percent = 100.0f;

    /* 1. Önce % değerini frekansa çevir (doğrusal interpolasyon) */
    float freq = MIN_FREQ_HZ + (MAX_FREQ_HZ - MIN_FREQ_HZ) * (speed_percent / 100.0f);

    /* 2. Frekansı ARR'a çevir */
    uint32_t arr = (uint32_t)(COUNTER_CLOCK / freq) - 1;

    return arr;
}

static void StartStepTask(void *argument)
{
  (void)argument;

  set_arr(999);
  uint8_t pwm_active = 1;

  for(;;)
  {
    int stop;

    if (g_pc_mode) {
        float speed = s_step_target;
        if (speed < 1.0f) {
            stop = 1;
        } else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1,
                s_step_dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
            set_arr(SpeedPercentToARR(speed));
            stop = 0;
        }
    } else {
        float pos = pos_pct[3];
        if (pos < 45.0f) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
            set_arr(SpeedPercentToARR(fabs(50.0f - pos) * 2.0f));
            stop = 0;
        } else if (pos > 55.0f) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
            set_arr(SpeedPercentToARR(fabs(50.0f - pos) * 2.0f));
            stop = 0;
        } else {
            stop = 1;
        }
    }

    if (stop && pwm_active) {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
        pwm_active = 0;
    } else if (!stop && !pwm_active) {
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
        pwm_active = 1;
    }

    osDelay(1);
  }
}

void StepControl_Init(void)
{
  stepTaskHandle = osThreadNew(StartStepTask, NULL, &stepTask_attributes);
}

void StepControl_SetTarget(float pct)
{
    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    s_step_target = pct;
}

void StepControl_SetDirAngle(uint8_t dir, uint16_t angle)
{
    s_step_dir    = dir;
    s_step_target = (angle / 8191.0f) * 100.0f;
}
