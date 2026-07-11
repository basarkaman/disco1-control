#include "linear.h"
#include "control.h"
#include "sr_control.h"
#include "app_config.h"

#define LINEAR1_BIT  0x40u   /* bit 6: extend */
#define LINEAR2_BIT  0x80u   /* bit 7: retract */

static volatile uint8_t s_linear_cmd = LINEAR_IDLE;

static void Motor_Extend(void)  { SR_SetBits(SR_MASK_LINEAR, LINEAR1_BIT); }
static void Motor_Retract(void) { SR_SetBits(SR_MASK_LINEAR, LINEAR2_BIT); }
static void Motor_Stop(void)    { SR_SetBits(SR_MASK_LINEAR, 0); }

static void Linear_Update(ControlMode_t mode, float rc_pct)
{
    switch (mode) {
        case CTRL_MODE_PC:
            switch (s_linear_cmd) {
                case LINEAR_EXTEND:  Motor_Extend();  break;
                case LINEAR_RETRACT: Motor_Retract(); break;
                default:              Motor_Stop();   break;
            }
            break;
        case CTRL_MODE_RC:
            if (rc_pct < APP_LINEAR_THRESHOLD_PCT) Motor_Retract();
            else                                    Motor_Extend();
            break;
        case CTRL_MODE_FAILSAFE:
        default:
            Motor_Stop();
            break;
    }
}

static const ControlSubsystem_t s_linear_subsystem = {
    .name       = "linear",
    .rc_channel = RC_CH_LINEAR,
    .period_ms  = APP_LINEAR_TASK_PERIOD_MS,
    .update     = Linear_Update,
};

void Linear_Init(void)
{
    Motor_Stop();
    Control_Register(&s_linear_subsystem);
}

void Linear_SetCmd(uint8_t cmd)
{
    s_linear_cmd = cmd;
}
