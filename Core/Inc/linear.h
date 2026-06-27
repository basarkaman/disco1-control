#ifndef LINEAR_H
#define LINEAR_H

/* Lineer aktüatör görevini oluşturur.
   osKernelInitialize()'dan sonra, osKernelStart()'tan önce çağrılmalı. */
void Linear_Init(void);

/* Hedef pozisyonu doğrudan yüzde olarak ayarlar (0-100). */
void Linear_SetTarget(float pct);

#endif /* LINEAR_H */
