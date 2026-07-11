#include "control.h"
#include "app_config.h"
#include "rc.h"
#include "main.h"
#include "cmsis_os.h"

#define MAX_SUBSYSTEMS  8u

static const ControlSubsystem_t *s_subsystems[MAX_SUBSYSTEMS];
static uint16_t                  s_elapsed_ms[MAX_SUBSYSTEMS];
static uint8_t                   s_count = 0;

static const uint16_t s_led_pin[4] = APP_LED_PINS;
static volatile uint32_t s_led_flash_ms[4] = {0};

static osThreadId_t controlTaskHandle;
static const osThreadAttr_t controlTask_attributes = {
    .name       = "controlTask",
    .stack_size = 256 * 4,
    .priority   = (osPriority_t) osPriorityAboveNormal,
};

void Control_Register(const ControlSubsystem_t *sub)
{
    if (s_count >= MAX_SUBSYSTEMS) return;
    s_subsystems[s_count] = sub;
    s_elapsed_ms[s_count] = 0;
    s_count++;
}

ControlMode_t Control_GetMode(void)
{
    if (!RCInput_LinkOk()) return CTRL_MODE_FAILSAFE;
    return (RCInput_GetPosPct(RC_CH_MODE) > APP_MODE_THRESHOLD_PCT)
           ? CTRL_MODE_PC : CTRL_MODE_RC;
}

void Control_FlashLED(uint8_t led_idx)
{
    if (led_idx < 4) s_led_flash_ms[led_idx] = APP_LED_FLASH_MS;
}

static void Control_UpdateLEDs(ControlMode_t mode)
{
    for (int i = 0; i < 4; i++) {
        GPIO_PinState state;
        if (s_led_flash_ms[i] > 0u) {
            s_led_flash_ms[i] -= (s_led_flash_ms[i] > APP_CONTROL_TICK_MS)
                                  ? APP_CONTROL_TICK_MS : s_led_flash_ms[i];
            state = GPIO_PIN_SET;
        } else if (mode == CTRL_MODE_RC) {
            state = (RCInput_GetPosPct((uint8_t)i) > APP_MODE_THRESHOLD_PCT)
                    ? GPIO_PIN_SET : GPIO_PIN_RESET;
        } else {
            state = GPIO_PIN_RESET;
        }
        HAL_GPIO_WritePin(GPIOD, s_led_pin[i], state);
    }
}

static void StartControlTask(void *argument)
{
    (void)argument;
    for (;;)
    {
        ControlMode_t mode = Control_GetMode();

        for (uint8_t i = 0; i < s_count; i++) {
            s_elapsed_ms[i] += APP_CONTROL_TICK_MS;
            if (s_elapsed_ms[i] >= s_subsystems[i]->period_ms) {
                s_elapsed_ms[i] = 0;
                float rc_pct = RCInput_GetPosPct(s_subsystems[i]->rc_channel);
                s_subsystems[i]->update(mode, rc_pct);
            }
        }

        Control_UpdateLEDs(mode);

        osDelay(APP_CONTROL_TICK_MS);
    }
}

void Control_Init(void)
{
    controlTaskHandle = osThreadNew(StartControlTask, NULL, &controlTask_attributes);
}
