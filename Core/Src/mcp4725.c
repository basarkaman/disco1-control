#include "mcp4725.h"
#include "control.h"
#include "app_config.h"
#include "rc.h"

static I2C_HandleTypeDef *s_hi2c      = NULL;
static volatile uint16_t  s_raw12_pc  = 0;

/* ch9 anahtari acikken izin verilen azami raw DAC degeri. */
#define APP_DAC_AUX_LIMIT_RAW \
    ((uint16_t)((APP_DAC_AUX_LIMIT_V / APP_DAC_AUX_LIMIT_FULLSCALE_V) * MCP4725_MAX))

/* MCP4725 Fast Mode Write: 2 bayt, ust 4 bit komut (0000) + 12 bit veri */
static void DAC_Write(uint16_t raw)
{
    if (raw > MCP4725_MAX) raw = MCP4725_MAX;
    uint8_t buf[2] = {
        (uint8_t)((raw >> 8) & 0x0F),
        (uint8_t)(raw & 0xFF)
    };
    HAL_I2C_Master_Transmit(s_hi2c, MCP4725_ADDR, buf, 2, 10);
}

static void Throttle_Update(ControlMode_t mode, float rc_pct)
{
    uint16_t raw;

    switch (mode) {
        case CTRL_MODE_PC:
            raw = s_raw12_pc;
            break;
        case CTRL_MODE_RC:
            raw = (rc_pct >= APP_DAC_THRESHOLD_PCT)
                  ? (uint16_t)((rc_pct - APP_DAC_THRESHOLD_PCT) * APP_DAC_GAIN)
                  : 0;
            break;
        case CTRL_MODE_FAILSAFE:
        default:
            raw = 0;
            break;
    }

    uint16_t limit_raw;
    if (RCInput_GetPosPct(RC_CH_SR_AUX) > APP_AUX_THRESHOLD_PCT) {
        /* anahtar ON (kapali): sabit 1.5V/5V guvenlik tavani */
        limit_raw = APP_DAC_AUX_LIMIT_RAW;
    } else {
        /* anahtar OFF (acik): tavan CH6 slider yuzdesinden okunur */
        float limit_pct = RCInput_GetPosPct(RC_CH_THROTTLE_LIMIT);
        limit_raw = (uint16_t)((limit_pct / 100.0f) * MCP4725_MAX);
    }

    if (raw > limit_raw) raw = limit_raw;

    DAC_Write(raw);
}

static const ControlSubsystem_t s_throttle_subsystem = {
    .name       = "throttle",
    .rc_channel = RC_CH_THROTTLE,
    .period_ms  = APP_DAC_TASK_PERIOD_MS,
    .update     = Throttle_Update,
};

void MCP4725_Init(I2C_HandleTypeDef *hi2c)
{
    s_hi2c = hi2c;
    Control_Register(&s_throttle_subsystem);
}

void MCP4725_SetRaw12(uint16_t val)
{
    if (val > MCP4725_MAX) val = MCP4725_MAX;
    s_raw12_pc = val;
}
