#include "pc_control.h"
#include "control.h"
#include "app_config.h"
#include "main.h"
#include "usart.h"
#include "cmsis_os.h"
#include "linear.h"
#include "step_control.h"
#include "mcp4725.h"
#include "sr_control.h"

#define QUEUE_DEPTH 8

typedef struct { uint8_t b0; uint8_t b1; } PCFrame_t;
typedef enum   { RX_FIRST, RX_SECOND    } RxState_t;
typedef struct { uint8_t nbytes; void (*handle)(const PCFrame_t *f); } PCCommand_t;

static uint8_t            s_rx_byte;
static RxState_t          s_state = RX_FIRST;
static uint8_t            s_b0;
static osMessageQueueId_t s_queue;

static void Handle_Linear(const PCFrame_t *f)
{
    Linear_SetCmd((f->b0 >> 4) & 0x03u);
}

static void Handle_Throttle(const PCFrame_t *f)
{
    uint16_t raw = ((uint16_t)(f->b0 & 0x3Fu) << 6) | (f->b1 >> 2);
    MCP4725_SetRaw12(raw);
}

static void Handle_Step(const PCFrame_t *f)
{
    uint8_t  dir   = (f->b0 >> 5) & 0x01u;
    uint16_t angle = ((uint16_t)(f->b0 & 0x1Fu) << 8) | f->b1;
    StepControl_SetDirAngle(dir, angle);
}

static void Handle_SR(const PCFrame_t *f)
{
    SR_SetBits(SR_MASK_PC_SPARE, f->b1);
}

/* Indeks = opcode (OP_LINEAR..OP_SR, s_rx_byte'in ust 2 biti - her zaman 0-3). */
static const PCCommand_t s_commands[4] = {
    [OP_LINEAR]   = { 1, Handle_Linear   },
    [OP_THROTTLE] = { 2, Handle_Throttle },
    [OP_STEP]     = { 2, Handle_Step     },
    [OP_SR]       = { 2, Handle_SR       },
};

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_9);  /* skop probu: UART1 aktivite gostergesi */
    if (huart->Instance != USART1) return;

    if (s_state == RX_FIRST) {
        uint8_t op = s_rx_byte >> 6;
        if (s_commands[op].nbytes == 1) {
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

static void execute(const PCFrame_t *f)
{
    uint8_t op = f->b0 >> 6;
    Control_FlashLED(op & 3u);
    s_commands[op].handle(f);
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
