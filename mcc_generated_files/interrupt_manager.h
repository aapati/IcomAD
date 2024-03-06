/* File Name:    interrupt_manager.h  PIC16F15323 */

#ifndef INTERRUPT_MANAGER_H
#define INTERRUPT_MANAGER_H

#define INTERRUPT_GlobalInterruptEnable() (INTCONbits.GIE = 1)
#define INTERRUPT_GlobalInterruptDisable() (INTCONbits.GIE = 0)
#define INTERRUPT_PeripheralInterruptEnable() (INTCONbits.PEIE = 1)
#define INTERRUPT_PeripheralInterruptDisable() (INTCONbits.PEIE = 0)

#endif  // INTERRUPT_MANAGER_H
/* End of File */
