/*  @File Name:    mcc.h     Device :  PIC16F15323 */

#ifndef MCC_H
#define	MCC_H
#include <xc.h>
#include "device_config.h"
#include "pin_manager.h"
#include <stdint.h>
#include <stdbool.h>
#include <conio.h>
#include "interrupt_manager.h"
//#include "tmr1.h"
#include "adc.h"
//#include "tmr0.h"
#include "eusart1.h"

#define WDTCWS  7
#define WDTCPS  10
#define WDTCCS  1

#define ATLAGMERES  // több mérés átlagát küldi el  

 
void SYSTEM_Initialize(void);
void OSCILLATOR_Initialize(void);
void PMD_Initialize(void);
//void WWDT_Initialize(void); 
void WWDT_SoftEnable(void); /*  Enable the WWDT by setting the SEN bit. */
//void WWDT_SoftDisable(void); /*    Disable the WWDT by clearing the SEN bit.  */
//void WWDT_TimerClear(void);
//bool WWDT_TimeOutStatusGet(void);
//bool WWDT_WindowViolationStatusGet(void);      

#endif	/* MCC_H */
/** End of File */