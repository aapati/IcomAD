/*File Name:    interrupt_manager.c  Device            :  PIC16F15323    
*/

#include "interrupt_manager.h"
#include "mcc.h"

void __interrupt() INTERRUPT_InterruptManager (void)  // interrupt handler
{ 
    //if(PIE0bits.TMR0IE == 1 && PIR0bits.TMR0IF == 1) {
        //TMR0_ISR();
   // }
   // else if(INTCONbits.PEIE == 1) {
   if(INTCONbits.PEIE == 1) {
        //if(PIE1bits.ADIE == 1 && PIR1bits.ADIF == 1)  {
          //  ADC_ISR();
       // } 
       //else if(PIE3bits.TX1IE == 1 && PIR3bits.TX1IF == 1)  {
        //if(PIE3bits.TX1IE == 1 && PIR3bits.TX1IF == 1)  {
           // EUSART1_TxDefaultInterruptHandler();
       // } 
        //else if(PIE3bits.RC1IE == 1 && PIR3bits.RC1IF == 1)  { //RC1IF  vétel IT létrejött
        if(PIE3bits.RC1IE == 1 && PIR3bits.RC1IF == 1)  { // RC1IF  vétel IT létrejött
            EUSART1_RxDefaultInterruptHandler();
        } 
        //else if(PIE4bits.TMR1IE == 1 && PIR4bits.TMR1IF == 1)  {
            //TMR1_ISR();
        //} 
       // else if(PIE5bits.TMR1GIE == 1 && PIR5bits.TMR1GIF == 1) {
         //   TMR1_GATE_ISR();
      //  }      
    }         
}
/* End of File */
