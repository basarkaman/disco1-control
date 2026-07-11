#ifndef AUX_CONTROL_H
#define AUX_CONTROL_H

/* RC_CH_SR_AUX kanalini SR_MASK_AUX bitine aktaran kucuk subsystem.
   Eskiden rc.c StartRCTask icinde dogrudan g_sr_state'e yaziliyordu;
   artik sr_control'un mask-tabanli SR_SetBits() API'sini kullanir. */
void AuxControl_Init(void);

#endif /* AUX_CONTROL_H */
