#include "linear.h"
#include "main.h"
#include "cmsis_os.h"

#define LINEAR_TASK_PERIOD_MS  20u
#define DEADBAND_PCT           3.0f   /* ±3% içinde dur */
#define RC_CHANNEL             0     /* pos_pct[2] = CH3 */

extern ADC_HandleTypeDef hadc1;
extern volatile float pos_pct[];

static volatile float s_target_pct = 50.0f;

static osThreadId_t linearTaskHandle;
static const osThreadAttr_t linearTask_attributes = {
  .name       = "linearTask",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t) osPriorityNormal,
};

static float ADC_ReadPct(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 5);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    return (raw / 4095.0f) * 100.0f;
}

static void Motor_Extend(void)   /* LINEAR2=1, LINEAR1=0  (PC8=1, PC9=0) */
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
}

static void Motor_Retract(void)  /* LINEAR2=0, LINEAR1=1  (PC8=0, PC9=1) */
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
}

static void Motor_Stop(void)     /* PC8=0, PC9=0 */
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
}

static void StartLinearTask(void *argument)
{
    (void)argument;
    for (;;)
    {
        s_target_pct = pos_pct[1];
        float actual = ADC_ReadPct();
        float error  = s_target_pct - actual;

        if (s_target_pct < 45.f)
            Motor_Retract();
        else
            Motor_Extend();

        osDelay(LINEAR_TASK_PERIOD_MS);
    }
}

void Linear_Init(void)
{
    Motor_Stop();
    linearTaskHandle = osThreadNew(StartLinearTask, NULL, &linearTask_attributes);
}

void Linear_SetTarget(float pct)
{
    if (pct < 0.0f)   pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    s_target_pct = pct;
}
