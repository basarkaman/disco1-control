#include "aux_control.h"
#include "control.h"
#include "sr_control.h"
#include "app_config.h"

static void Aux_Update(ControlMode_t mode, float rc_pct)
{
    if (mode == CTRL_MODE_FAILSAFE) {
        SR_SetBits(SR_MASK_AUX, 0);
        return;
    }
    SR_SetBits(SR_MASK_AUX, (rc_pct > APP_AUX_THRESHOLD_PCT) ? SR_MASK_AUX : 0);
}

static const ControlSubsystem_t s_aux_subsystem = {
    .name       = "aux",
    .rc_channel = RC_CH_SR_AUX,
    .period_ms  = APP_AUX_TASK_PERIOD_MS,
    .update     = Aux_Update,
};

void AuxControl_Init(void)
{
    Control_Register(&s_aux_subsystem);
}
