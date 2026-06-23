#include "mcp4725.h"
#include "cmsis_os.h"
#include <math.h>

#define DAC_TASK_PERIOD_MS  10u

static I2C_HandleTypeDef *s_hi2c = NULL;
static volatile uint16_t  s_raw  = 0;

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

static void StartDACTask(void *argument)
{
    (void)argument;
    for (;;)
    {
        DAC_Write(s_raw);
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
