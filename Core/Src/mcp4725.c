#include "mcp4725.h"
#include "rc.h"
#include "cmsis_os.h"
#include <math.h>

#define DAC_TASK_PERIOD_MS  10u

static I2C_HandleTypeDef *s_hi2c = NULL;
static volatile uint16_t  s_raw       = 0;
static volatile uint16_t  s_raw12_pc  = 0;

#define NUM_CH 6

extern volatile uint32_t t_rise[NUM_CH];
extern volatile uint32_t pulse_cyc[NUM_CH];
extern volatile uint8_t  ready[NUM_CH];
extern volatile uint32_t pulse_us[NUM_CH];
extern volatile float    pos_pct[NUM_CH];

static osThreadId_t dacTaskHandle;
static const osThreadAttr_t dacTask_attributes = {
  .name       = "dacTask",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t) osPriorityLow,
};

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

float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static void StartDACTask(void *argument)
{
    (void)argument;
    for (;;)
    {
        if (g_pc_mode) {
            DAC_Write(s_raw12_pc);
        } else {
            float pct = pos_pct[1];
            if (pct >= 52.5f)
                DAC_Write((uint16_t)((pct - 52.5f) * 43.11f));
            else
                DAC_Write(0);
        }
        osDelay(DAC_TASK_PERIOD_MS);
    }
}

void MCP4725_Init(I2C_HandleTypeDef *hi2c)
{
    s_hi2c = hi2c;
    dacTaskHandle = osThreadNew(StartDACTask, NULL, &dacTask_attributes);
}

void MCP4725_SetRaw(uint16_t value)
{
    if (value > MCP4725_MAX) value = MCP4725_MAX;
    s_raw = value;
}

void MCP4725_SetVoltage(float volts)
{
    if (volts < 0.0f)          volts = 0.0f;
    if (volts > MCP4725_VREF)  volts = MCP4725_VREF;
    s_raw = (uint16_t)(volts / MCP4725_VREF * MCP4725_MAX);
}

void MCP4725_SetRaw12(uint16_t val)
{
    if (val > MCP4725_MAX) val = MCP4725_MAX;
    s_raw12_pc = val;
}
