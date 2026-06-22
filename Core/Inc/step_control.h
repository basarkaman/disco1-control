#ifndef STEP_CONTROL_H
#define STEP_CONTROL_H

/* Step (STEP_PUL/STEP_DIR) sinyalini ureten stepTask'i yaratir.
   osKernelInitialize() sonrasi, osKernelStart() oncesi cagrilmali. */
void StepControl_Init(void);

#endif /* STEP_CONTROL_H */
