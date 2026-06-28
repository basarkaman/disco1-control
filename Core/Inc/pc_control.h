#ifndef PC_CONTROL_H
#define PC_CONTROL_H

#include <stdint.h>

/* Binary frame: ilk byte'in [7:6] bitleri opcode seçici.
 *
 * LINEAR   0b00  1 byte:  [5:4]=cmd  (00=idle 01=extend 10=retract)
 * THROTTLE 0b01  2 byte:  [5:0]=dac[11:6]  |  byte1[7:2]=dac[5:0]   (12-bit raw)
 * STEP     0b10  2 byte:  [5]=dir [4:0]=angle[12:8]  |  byte1=angle[7:0]  (13-bit)
 * SR       0b11  2 byte:  byte1=data  (8-bit)
 */
#define OP_LINEAR    0b00u
#define OP_THROTTLE  0b01u
#define OP_STEP      0b10u
#define OP_SR        0b11u

void PCControl_Init(void);

#endif /* PC_CONTROL_H */
