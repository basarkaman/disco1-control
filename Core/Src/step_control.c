#include "step_control.h"
#include "control.h"
#include "app_config.h"
#include "main.h"
#include "tim.h"
#include <math.h>

#define STEP_TIM_CHANNEL   TIM_CHANNEL_3   /* stepper pulse timer kanali */

static volatile float   s_step_target = 0.0f;
static volatile uint8_t s_step_dir    = 0;
static uint8_t          s_pwm_active  = 1;

/* PWM kanal baslangic konfigurasyonu (duty/Pulse degeri sabit kalir,
   hiz set_arr() ile ARR degistirilerek ayarlanir). */
static void ConfigurePWMChannel(uint16_t pulse)
{
    TIM_OC_InitTypeDef sConfigOC;

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, STEP_TIM_CHANNEL);
    HAL_TIM_PWM_Start(&htim3, STEP_TIM_CHANNEL);
}

static void SetARR(uint32_t arr)
{
    HAL_TIM_PWM_Stop(&htim3, STEP_TIM_CHANNEL);
    __HAL_TIM_SET_PRESCALER(&htim3, APP_STEP_TIM_PSC);
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    HAL_TIM_PWM_Start(&htim3, STEP_TIM_CHANNEL);
}

static uint32_t SpeedPercentToARR(float speed_percent)
{
    if (speed_percent < 0.0f)   speed_percent = 0.0f;
    if (speed_percent > 100.0f) speed_percent = 100.0f;

    float freq = APP_STEP_MIN_FREQ_HZ +
                 (APP_STEP_MAX_FREQ_HZ - APP_STEP_MIN_FREQ_HZ) * (speed_percent / 100.0f);

    return (uint32_t)(APP_STEP_COUNTER_CLOCK_HZ / freq) - 1;
}

static void Step_Update(ControlMode_t mode, float rc_pct)
{
    int stop;

    switch (mode) {
        case CTRL_MODE_PC: {
            float speed = s_step_target;
            if (speed < 1.0f) {
                stop = 1;
            } else {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1,
                    s_step_dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
                SetARR(SpeedPercentToARR(speed));
                stop = 0;
            }
            break;
        }
        case CTRL_MODE_RC:
            if (rc_pct < APP_STEP_DEADBAND_LOW_PCT) {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
                SetARR(SpeedPercentToARR(fabsf(APP_STEP_MIDPOINT_PCT - rc_pct) * APP_STEP_SPEED_GAIN));
                stop = 0;
            } else if (rc_pct > APP_STEP_DEADBAND_HIGH_PCT) {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
                SetARR(SpeedPercentToARR(fabsf(APP_STEP_MIDPOINT_PCT - rc_pct) * APP_STEP_SPEED_GAIN));
                stop = 0;
            } else {
                stop = 1;
            }
            break;
        case CTRL_MODE_FAILSAFE:
        default:
            stop = 1;
            break;
    }

    if (stop && s_pwm_active) {
        HAL_TIM_PWM_Stop(&htim3, STEP_TIM_CHANNEL);
        s_pwm_active = 0;
    } else if (!stop && !s_pwm_active) {
        HAL_TIM_PWM_Start(&htim3, STEP_TIM_CHANNEL);
        s_pwm_active = 1;
    }
}

static const ControlSubsystem_t s_step_subsystem = {
    .name       = "step",
    .rc_channel = RC_CH_STEP,
    .period_ms  = APP_STEP_TASK_PERIOD_MS,
    .update     = Step_Update,
};

void StepControl_Init(void)
{
    ConfigurePWMChannel(APP_STEP_INITIAL_PULSE);
    SetARR(APP_STEP_IDLE_ARR);
    Control_Register(&s_step_subsystem);
}

void StepControl_SetDirAngle(uint8_t dir, uint16_t angle)
{
    s_step_dir    = dir;
    s_step_target = (angle / APP_STEP_ANGLE_MAX) * 100.0f;
}
