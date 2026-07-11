#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

typedef enum {
    CTRL_MODE_RC,
    CTRL_MODE_PC,
    CTRL_MODE_FAILSAFE,
} ControlMode_t;

typedef struct {
    const char *name;
    uint8_t     rc_channel;   /* RCInput_GetPosPct kanali */
    uint16_t    period_ms;    /* update() cagri periyodu */
    void      (*update)(ControlMode_t mode, float rc_pct);
} ControlSubsystem_t;

/* Bir subsystem'i scheduler'a kaydeder. sub, kaydeden modulun static
   storage'inda yasamali (Control_Init'ten sonra da gecerli kalmali). */
void Control_Register(const ControlSubsystem_t *sub);

/* Scheduler task'ini olusturur. Tum subsystem *_Init() cagrilarindan
   (ve dolayisiyla Control_Register'lardan) SONRA cagrilmali. */
void Control_Init(void);

/* Guncel kontrol modunu dondurur (RC link kaybi -> FAILSAFE). */
ControlMode_t Control_GetMode(void);

/* PC modunda komut alindiginda ilgili LED'i APP_LED_FLASH_MS boyunca yakar. */
void Control_FlashLED(uint8_t led_idx);

#endif /* CONTROL_H */
