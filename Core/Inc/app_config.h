#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* ---------------------------------------------------------------------
 * Tum RC kanal atamalari, shift register bit sahiplikleri ve
 * kalibrasyon sabitleri tek yerde. Yeni bir subsystem eklerken
 * sadece burasi ve ilgili subsystem dosyasi degisir.
 * ------------------------------------------------------------------- */

/* --- RC kanal atamalari (0-tabanli, rc.h NUM_CH ile sinirli) --- */
#define RC_CH_LINEAR      1u
#define RC_CH_THROTTLE    1u
#define RC_CH_STEP        3u
#define RC_CH_MODE        6u
#define RC_CH_SR_AUX      9u

/* --- RC link-loss failsafe --- */
#define RC_FAILSAFE_TIMEOUT_MS   150u

/* --- Shift register bit sahiplikleri (8 bit, ayrik/non-overlapping) ---
 * bit7 bit6 : linear motor (extend/retract)
 * bit5..1   : PC OP_SR (serbest, PC tarafindan istenildigi gibi kullanilir)
 * bit0      : rc aux (RC_CH_SR_AUX kanalindan)
 */
#define SR_MASK_LINEAR     0xC0u
#define SR_MASK_PC_SPARE   0x3Eu
#define SR_MASK_AUX        0x01u

/* --- Linear subsystem --- */
#define APP_LINEAR_TASK_PERIOD_MS   20u
#define APP_LINEAR_THRESHOLD_PCT    45.0f   /* < esik: retract, aksi: extend */

/* --- Throttle (MCP4725 DAC) subsystem --- */
#define APP_DAC_TASK_PERIOD_MS      10u
#define APP_DAC_THRESHOLD_PCT       52.5f
#define APP_DAC_GAIN                43.11f

/* --- Step (stepper) subsystem --- */
#define APP_STEP_TASK_PERIOD_MS     1u
#define APP_STEP_DEADBAND_LOW_PCT   45.0f
#define APP_STEP_DEADBAND_HIGH_PCT  55.0f
#define APP_STEP_MIDPOINT_PCT       50.0f
#define APP_STEP_SPEED_GAIN         2.0f
#define APP_STEP_ANGLE_MAX          8191.0f  /* PC protokolundeki 13-bit acinin ust siniri */
#define APP_STEP_MIN_FREQ_HZ        100u
#define APP_STEP_MAX_FREQ_HZ        1000u
#define APP_STEP_COUNTER_CLOCK_HZ   1000000UL /* TIM3 PSC ile elde edilen sayici clock'u */
#define APP_STEP_IDLE_ARR           999u
#define APP_STEP_TIM_PSC            84u
#define APP_STEP_INITIAL_PULSE      270u  /* baslangic PWM duty (Pulse) degeri */

/* --- Aux subsystem (RC_CH_SR_AUX -> SR_MASK_AUX) --- */
#define APP_AUX_TASK_PERIOD_MS      10u
#define APP_AUX_THRESHOLD_PCT       50.0f

/* --- LED pinleri (GPIOD), 4 adet --- */
#define APP_LED_PINS  { LD4_Pin, LD3_Pin, LD5_Pin, LD6_Pin }
#define APP_LED_FLASH_MS  200u

/* --- Control task (scheduler) --- */
#define APP_CONTROL_TICK_MS   1u
#define APP_MODE_THRESHOLD_PCT 50.0f

#endif /* APP_CONFIG_H */
