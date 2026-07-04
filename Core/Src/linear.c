#include "linear.h"
#include "sr_control.h"
#include "rc.h"
#include "main.h"
#include "cmsis_os.h"

#define LINEAR_TASK_PERIOD_MS  20u

extern ADC_HandleTypeDef hadc1;
extern volatile float pos_pct[];

static volatile float   s_target_pct  = 50.0f;
static volatile uint8_t s_linear_cmd  = LINEAR_IDLE;

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

#define LINEAR_BITS_MASK  0xC0u
#define LINEAR1_BIT       0x40u   /* bit 6 */
#define LINEAR2_BIT       0x80u   /* bit 7 */

static void Motor_Extend(void)
{
    g_sr_state = (g_sr_state & ~LINEAR_BITS_MASK) | LINEAR1_BIT;
    SR_Flush();
}

static void Motor_Retract(void)
{
    g_sr_state = (g_sr_state & ~LINEAR_BITS_MASK) | LINEAR2_BIT;
    SR_Flush();
}

static void Motor_Stop(void)
{
    g_sr_state = g_sr_state & ~LINEAR_BITS_MASK;
    SR_Flush();
}

static void StartLinearTask(void *argument)
{
    (void)argument;
    for (;;)
    {
        if (g_pc_mode) {
            switch (s_linear_cmd) {
                case LINEAR_EXTEND:  Motor_Extend();  break;
                case LINEAR_RETRACT: Motor_Retract(); break;
                default:             Motor_Stop();    break;
            }
        } else {
            s_target_pct = pos_pct[1];
            float actual = ADC_ReadPct();
            if (s_target_pct < 45.0f)
                Motor_Retract();
            else
                Motor_Extend();
        }
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

void Linear_SetCmd(uint8_t cmd)
{
    s_linear_cmd = cmd;
}
