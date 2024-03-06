/*  @File Name   eusart1.c  Device:  PIC16F15323 */

#include "eusart1.h"

#define EUSART1_TX_BUFFER_SIZE 8
#define EUSART1_RX_BUFFER_SIZE 8

/*  Section: Global Variables */
//volatile uint8_t eusart1TxHead = 0;
//volatile uint8_t eusart1TxTail = 0;
//volatile uint8_t eusart1TxBuffer[EUSART1_TX_BUFFER_SIZE];
//volatile uint8_t eusart1TxBufferRemaining;

volatile uint8_t eusart1RxHead = 0;
volatile uint8_t eusart1RxTail = 0;
volatile uint8_t eusart1RxBuffer[EUSART1_RX_BUFFER_SIZE];
volatile eusart1_status_t eusart1RxStatusBuffer[EUSART1_RX_BUFFER_SIZE];
volatile uint8_t eusart1RxCount;
volatile eusart1_status_t eusart1RxLastError;

/*  Section: EUSART1 APIs */
void (*EUSART1_TxDefaultInterruptHandler)(void);
void (*EUSART1_RxDefaultInterruptHandler)(void);

void (*EUSART1_FramingErrorHandler)(void);
void (*EUSART1_OverrunErrorHandler)(void);
void (*EUSART1_ErrorHandler)(void);

void EUSART1_DefaultFramingErrorHandler(void);
void EUSART1_DefaultOverrunErrorHandler(void);
void EUSART1_DefaultErrorHandler(void);
// * * * * * * * * * * * * * * * 
void EUSART1_Initialize(void) //9600 Baud 8 bit
{   
    PIE3bits.RC1IE = 0;  // disable interrupts before changing states
    EUSART1_SetRxInterruptHandler(EUSART1_Receive_ISR);
    PIE3bits.TX1IE = 0;
    //EUSART1_SetTxInterruptHandler(EUSART1_Transmit_ISR);
    
 // --- 3 DB CONFIG REG:  Set the EUSART1 module to the options selected in the user interface.   
    BAUD1CON = 0x08;  // ABDOVF no_overflow; BIT4=0 SCKP (TX) Non-Inverted; BRG16 16bit_generator; WUE disabled; ABDEN disabled;    
    RC1STA = 0x90; //SPEN=1 enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled;    
    TX1STA = 0x24; //TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN=1 enabled; SYNC=0 asynchronous; BRGH hi_speed; CSRC slave; 
 // ---
    
    SP1BRGL = 0xA0; // SP1BRGL 160;      
    SP1BRGH = 0x01; // SP1BRGH 1;

    EUSART1_SetFramingErrorHandler(EUSART1_DefaultFramingErrorHandler);
    EUSART1_SetOverrunErrorHandler(EUSART1_DefaultOverrunErrorHandler);
    EUSART1_SetErrorHandler(EUSART1_DefaultErrorHandler);

    eusart1RxLastError.status = 0;

    // initializing the driver state
    // eusart1TxHead = 0;
    // eusart1TxTail = 0;
    //eusart1TxBufferRemaining = sizeof(eusart1TxBuffer);

    eusart1RxHead = 0;
    eusart1RxTail = 0;
    eusart1RxCount = 0;
   
    PIE3bits.RC1IE = 1;  // enable receive interrupt
}

//bool EUSART1_is_tx_ready(void)
//{
   // return (eusart1TxBufferRemaining ? true : false);
//}

/*
bool EUSART1_is_rx_ready(void)
{
    return (eusart1RxCount ? true : false);
}
 */ 
/*
bool EUSART1_is_tx_done(void)
{
    return TX1STAbits.TRMT;
} 

eusart1_status_t EUSART1_get_last_status(void){
    return eusart1RxLastError;
} */

uint8_t EUSART1_Read(void) //!! feltétel eusart1RxCount >0!
{
    uint8_t readValue  = 0;
    eusart1RxLastError = eusart1RxStatusBuffer[eusart1RxTail];

    readValue = eusart1RxBuffer[eusart1RxTail++];
    if(sizeof(eusart1RxBuffer) <= eusart1RxTail)
    {
        eusart1RxTail = 0;
    }
    PIE3bits.RC1IE = 0; // vétel IT dis
    eusart1RxCount--;
    PIE3bits.RC1IE = 1; // vétel IT enable

    return readValue;
}

void EUSART1_Write(uint8_t txData) //!!
{
  // while(0 == eusart1TxBufferRemaining)
 //   {
  //      NOP();
 //   }
    TX1REG = txData; 
 return;
 /*
    if(0 == PIE3bits.TX1IE) //TX IT enable?
    { //no
        TX1REG = txData;
    }
    else
    {
        PIE3bits.TX1IE = 0;
        eusart1TxBuffer[eusart1TxHead++] = txData;
        if(sizeof(eusart1TxBuffer) <= eusart1TxHead)
        {
            eusart1TxHead = 0;
        }
        eusart1TxBufferRemaining--;
    }
    PIE3bits.TX1IE = 1;
  */ 
}

/*
void EUSART1_Transmit_ISR(void)
{
    if(sizeof(eusart1TxBuffer) > eusart1TxBufferRemaining)
    {
        TX1REG = eusart1TxBuffer[eusart1TxTail++];
        if(sizeof(eusart1TxBuffer) <= eusart1TxTail)
        {
            eusart1TxTail = 0;
        }
        eusart1TxBufferRemaining++;
    }
    else
    {
        PIE3bits.TX1IE = 0; //ures a buffer nem lesz több IT
    }
}
*/
void EUSART1_Receive_ISR(void)
{    
    eusart1RxStatusBuffer[eusart1RxHead].status = 0;

    if(RC1STAbits.FERR){
        eusart1RxStatusBuffer[eusart1RxHead].ferr = 1;
        EUSART1_FramingErrorHandler();
    }

    if(RC1STAbits.OERR){
        eusart1RxStatusBuffer[eusart1RxHead].oerr = 1;
        EUSART1_OverrunErrorHandler();
    }
    
    if(eusart1RxStatusBuffer[eusart1RxHead].status){
        EUSART1_ErrorHandler();
    } else {
        EUSART1_RxDataHandler(); 
    }    
    // or set custom function using EUSART1_SetRxInterruptHandler()
}

void EUSART1_RxDataHandler(void){ // use this default receive interrupt handler code
    eusart1RxBuffer[eusart1RxHead++] = RC1REG;
    if(sizeof(eusart1RxBuffer) <= eusart1RxHead)
    {
        eusart1RxHead = 0;
    }
    eusart1RxCount++;
}

void EUSART1_DefaultFramingErrorHandler(void){}

void EUSART1_DefaultOverrunErrorHandler(void){
    // EUSART1 error - restart
    RC1STAbits.CREN = 0;
    RC1STAbits.CREN = 1;
}

void EUSART1_DefaultErrorHandler(void){
    EUSART1_RxDataHandler();
}

void EUSART1_SetFramingErrorHandler(void (* interruptHandler)(void)){
    EUSART1_FramingErrorHandler = interruptHandler;
}

void EUSART1_SetOverrunErrorHandler(void (* interruptHandler)(void)){
    EUSART1_OverrunErrorHandler = interruptHandler;
}

void EUSART1_SetErrorHandler(void (* interruptHandler)(void)){
    EUSART1_ErrorHandler = interruptHandler;
}

//void EUSART1_SetTxInterruptHandler(void (* interruptHandler)(void)){
   //EUSART1_TxDefaultInterruptHandler = interruptHandler;
//}

void EUSART1_SetRxInterruptHandler(void (* interruptHandler)(void)){
    EUSART1_RxDefaultInterruptHandler = interruptHandler;
}

void EUSART1_Init_txLOW(void) //!! HIBAJEL ircs-nek (ha nincs soros vonali kapcsolat)
{   
    PIE3bits.RC1IE = 0;  // disable interrupts before changing states      
    BAUD1CON |= 0x10;    // BIT4=1 SCKP (TX) Inverted;      
    PIE3bits.RC1IE = 1;  // enable receive interrupt
}
//
//void EUSART1_Init_txH(void)
//{   
   // PIE3bits.RC1IE = 0;  // disable interrupts before changing states
      
  //  BAUD1CON &= 0xef;    //BIT4=0 SCKP (TX)non-Inverted; alapállapot nincs HIBAJEL az ircs-nek (ha nincs soros vonali kapcsolat)   
   
  //  PIE3bits.RC1IE = 1;  // enable receive interrupt
//}
/* End of File */
