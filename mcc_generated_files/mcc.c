/*  File Name:  mcc.c  Device :  PIC16F15323 */

#include "mcc.h"

void SYSTEM_Initialize(void)
{
    PMD_Initialize();           //konfig regs.
    PIN_MANAGER_Initialize();   //LAT,TRIS,OD, stb. reg
    OSCILLATOR_Initialize();
    //WWDT_Initialize();
    ADC_Initialize();
    //TMR1_Initialize();
    //TMR0_Initialize();
    EUSART1_Initialize();
    
    FVRCON = 0x00; //belsö ref fesz. és hömérö beállítása
}

void OSCILLATOR_Initialize(void)
{
   
    OSCCON1 = 0x10;  // NOSC HFINTOSC with 2x PLL; NDIV 1;    
    OSCCON3 = 0x00;  // CSWHOLD may proceed;   
    OSCEN = 0x00;   // MFOEN disabled; LFOEN disabled; ADOEN disabled; EXTOEN disabled; HFOEN disabled; 
    OSCFRQ = 0x03;     // HFFRQ 8_MHz;    
    OSCSTAT = 0x00;  // MFOR not ready;    
    OSCTUNE = 0x00;  // HFTUN 0; 
   
    while(PLLR == 0)  // Wait for PLL to stabilize
    {
    }
}

void PMD_Initialize(void)
{
    // CLKRMD CLKR enabled; SYSCMD SYSCLK enabled; FVRMD FVR enabled; IOCMD IOC enabled; NVMMD NVM enabled; 
    PMD0 = 0x00;
    // TMR0MD TMR0 enabled; TMR1MD TMR1 enabled; TMR2MD TMR2 enabled; NCOMD DDS(NCO) enabled; 
    PMD1 = 0x00;
    // ZCDMD ZCD enabled; CMP1MD CMP1 enabled; ADCMD ADC enabled; CMP2MD CMP2 enabled; DAC1MD DAC1 enabled; 
    PMD2 = 0x00;
    // CCP2MD CCP2 enabled; CCP1MD CCP1 enabled; PWM4MD PWM4 enabled; PWM3MD PWM3 enabled; PWM6MD PWM6 enabled; PWM5MD PWM5 enabled; 
    PMD3 = 0x00;
    // CWG1MD CWG1 enabled; MSSP1MD MSSP1 enabled; UART1MD EUSART enabled; 
    PMD4 = 0x00;
    // CLC3MD CLC3 enabled; CLC4MD CLC4 enabled; CLC1MD CLC1 enabled; CLC2MD CLC2 enabled; 
    PMD5 = 0x00;
}
/*
void WWDT_Initialize(void)
{
    // Initializes the WWDT to the default states configured in the MCC GUI
    WDTCON0 = WDTCPS;
    WDTCON1 = WDTCWS|WDTCCS;    
}
*/

void WWDT_SoftEnable(void)
{   
    WDTCON0bits.SEN=1;  // WWDT software enable. 
}
/*
void WWDT_SoftDisable(void)
{    
    WDTCON0bits.SEN=0; // WWDT software disable.
}

bool WWDT_TimeOutStatusGet(void)
{
    // Return the status of WWDT time out reset.
    return (PCON0bits.nRWDT);
}

bool WWDT_WindowViolationStatusGet(void)
{
   // Return the status of WWDT window violation reset.
    return (PCON0bits.nWDTWV); 
}  

void WWDT_TimerClear(void)
{
    // Disable the interrupt,read back the WDTCON0 reg for arming, 
    // clearing the WWDT and enable the interrupt.
    uint8_t readBack=0;
    
    bool state = GIE;
    GIE = 0;
    readBack = WDTCON0;
    CLRWDT();
    GIE = state;
}
 */
/*End of File*/
