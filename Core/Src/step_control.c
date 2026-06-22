#include "step_control.h"
#include "main.h"
#include "cmsis_os.h"
#include <math.h>

#define STEP_TASK_PERIOD_MS  5u

#define NUM_CH 6


extern volatile uint32_t t_rise[NUM_CH];
extern volatile uint32_t pulse_cyc[NUM_CH];
extern volatile uint8_t  ready[NUM_CH];
extern volatile uint32_t pulse_us[NUM_CH];
extern volatile float    pos_pct[NUM_CH];

uint32_t remaining_delay;
const uint32_t POLL_MS = 5;

static osThreadId_t stepTaskHandle;
static const osThreadAttr_t stepTask_attributes = {
  .name = "stepTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Yuzdeye gore pulse suresi (ms) - ileride step hizini ayarlamak icin.
   Su an cagrilmiyor; referans olarak burada tutuluyor. */
#define PULSE_K       200.0f
#define PULSE_MS_MAX  5000.0f   // x=0'a yakinken pratik tavan (5 saniye gibi)

float PulseMsFromPercent(float pct)
{
    if (pct <= 0.1f) {
        return PULSE_MS_MAX;   // ya da: pulse uretme, motoru tamamen durdur
    }
    float ms = PULSE_K / pct;
    if (ms > PULSE_MS_MAX) ms = PULSE_MS_MAX;
    return ms;
}

static void StepTask_Wait()
{
    const uint32_t POLL_MS = 1;

    while (remaining_delay > 0) {
        uint32_t chunk = (remaining_delay < POLL_MS) ? remaining_delay : POLL_MS;
        osDelay(chunk);
        remaining_delay -= chunk;
    }
}

static void StepControl(int b)
{
    if(b) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);   
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
    } else if(!b) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
    }
}



static void StartStepTask(void *argument)
{
  (void)argument;
  for(;;)
  {
    int b;
    if(pos_pct[0] < 45.f) {
        b = 1;
    } else if(pos_pct[0] > 55.f) {
        b = 0;
    }
    StepControl(b);
    float pct = fabs(pos_pct[0] - 50);
    remaining_delay = PulseMsFromPercent(pct);
    StepTask_Wait();
  }
}

void StepControl_Init(void)
{
  stepTaskHandle = osThreadNew(StartStepTask, NULL, &stepTask_attributes);
}
