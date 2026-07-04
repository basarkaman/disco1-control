#include "pc_control.h"
#include "main.h"
#include "usart.h"
#include "cmsis_os.h"
#include "linear.h"
#include "step_control.h"
#include "mcp4725.h"
#include "sr_control.h"
#include "rc.h"
#include <string.h>

#define QUEUE_DEPTH 8

typedef struct { uint8_t b0; uint8_t b1; } PCFrame_t;
typedef enum   { RX_FIRST, RX_SECOND    } RxState_t;

static uint8_t            s_rx_byte;
static RxState_t          s_state = RX_FIRST;
static uint8_t            s_b0;
static osMessageQueueId_t s_queue;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_9);
    if (huart->Instance != USART1) return;

    if (s_state == RX_FIRST) {
        uint8_t op = s_rx_byte >> 6;
        if (op == OP_LINEAR) {
            PCFrame_t f = { s_rx_byte, 0 };
            osMessageQueuePut(s_queue, &f, 0, 0);
        } else {
            s_b0    = s_rx_byte;
            s_state = RX_SECOND;
        }
    } else {
        PCFrame_t f = { s_b0, s_rx_byte };
        osMessageQueuePut(s_queue, &f, 0, 0);
        s_state = RX_FIRST;
    }

    HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
}

static const uint16_t s_led_pin[4] = { LD4_Pin, LD3_Pin, LD5_Pin, LD6_Pin };

static void execute(const PCFrame_t *f)
{
    uint8_t op = f->b0 >> 6;
    HAL_GPIO_WritePin(GPIOD, s_led_pin[op & 3u], GPIO_PIN_SET);
    RC_FlashLED(op & 3u);
    switch (op) {
        case OP_LINEAR:
            Linear_SetCmd((f->b0 >> 4) & 0x03u);
            break;
        case OP_THROTTLE: {
            uint16_t raw = ((uint16_t)(f->b0 & 0x3Fu) << 6) | (f->b1 >> 2);
            MCP4725_SetRaw12(raw);
            break;
        }
        case OP_STEP: {
            uint8_t  dir   = (f->b0 >> 5) & 0x01u;
            uint16_t angle = ((uint16_t)(f->b0 & 0x1Fu) << 8) | f->b1;
            StepControl_SetDirAngle(dir, angle);
            break;
        }
        case OP_SR:
            SR_Write(f->b1);
            break;
        default:
            break;
    }
}

static osThreadId_t pcTaskHandle;
static const osThreadAttr_t pcTask_attributes = {
    .name       = "pcTask",
    .stack_size = 256 * 4,
    .priority   = (osPriority_t)osPriorityNormal,
};

static void StartPCTask(void *argument)
{
    (void)argument;
    PCFrame_t frame;
    for (;;) {
        if (osMessageQueueGet(s_queue, &frame, NULL, osWaitForever) == osOK)
            execute(&frame);
    }
}

void PCControl_Init(void)
{
    s_state = RX_FIRST;
    s_queue = osMessageQueueNew(QUEUE_DEPTH, sizeof(PCFrame_t), NULL);
    HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
    pcTaskHandle = osThreadNew(StartPCTask, NULL, &pcTask_attributes);
}
