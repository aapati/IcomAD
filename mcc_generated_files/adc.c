/*  ADC Generated Driver File  File Name adc.c  Device            :  PIC16F15323
*/

#include <xc.h>
#include "adc.h"
#include "device_config.h"

void ADC_Initialize(void)
{
    // ADCCON1/BIT7= ADFM  10 bites mért érték 16 bitben elhelyezése kétféleképpen
    //         bit1,0=00 ADPREF VDD; bit6-4 = 110 ADCS FOSC/64; 
#ifdef ATLAGMERES   
    ADCON1 = 0xe0;  //bit7 ADFM =1   mért érték MSB= bit10, LSB= bit0
#else    
    ADCON1 = 0x60;   //bit7 ADFM =0  mért érték MSB= bit15, LSB= bit6 
#endif    
    ADRESL = 0x00;   
    ADRESH = 0x00;   
    ADCON0 = 0x01;   // GOnDONE stop; ADON enabled; CHS ANA0;     
  
    //PIE1bits.ADIE = 1;   // Enabling ADC interrupt.	
	// Set Default Interrupt Handler
    //ADC_SetInterruptHandler(ADC_DefaultInterruptHandler);
}
//void ADC_StartConversion(void)
//{  
   // ADCON0bits.GOnDONE = 1;    // Start the conversion
//}

//bool ADC_IsConversionDone(void)
//{   
   //return ((bool)(!ADCON0bits.GOnDONE));   // Start the conversion
//}

//adc_result_t ADC_GetConversionResult(void)
//{   
   // return ((adc_result_t)((ADRESH << 8) + ADRESL));  // Conversion finished, return the result
//}

adc_result_t ADC_GetConversion(uint8_t channel) //!!
{//                      RC2  RA4 RC0 RC1  RC3  RA5 RA1 RA2
static uint8_t ch_tbl[] ={18,   4, 16, 17,  19,   5,  1,  2}; //T1,T2,T3,T4, +0V,+0VB, NTC, áram  
 
    ADCON0bits.CHS = 0x3B; // a pontosabb méréshez elöször Vss-re kapcsol egy ideig
    ADCON0bits.ADON = 1;  // Turn on the ADC module  
    __delay_us(ACQ_US_DELAY_VSS);  // Acquisition time delay 
    ADCON0bits.ADON = 0;  // Turn off the ADC module  
      
    ADCON0bits.CHS = ch_tbl[channel];   // select the A/D channel     
    ADCON0bits.ADON = 1;  // Turn on the ADC module       
    __delay_us(ACQ_US_DELAY);  // Acquisition time delay (10))
    
    ADCON0bits.GOnDONE = 1;  // Start the conversion 
    
    while (ADCON0bits.GOnDONE)  // Wait for the conversion to finish
    {
       NOP();
    }   
    ADCON0bits.CHS = 0x3B; // a pontosabb méréshez Vss-re kapcsol
    return ((adc_result_t)((ADRESH << 8) + ADRESL)); // Conversion finished, return the result
}

/*
void ADC_ISR(void) // IT rutin hívja ha ADC okozta az IT-t
{    
    PIR1bits.ADIF = 0; // Clear the ADC interrupt flag
	
	if(ADC_InterruptHandler)
        ADC_InterruptHandler();
}

void ADC_SetInterruptHandler(void (* InterruptHandler)(void)){
    ADC_InterruptHandler = InterruptHandler;
}

void ADC_DefaultInterruptHandler(void){
    // add your ADC interrupt custom code
    // or set custom function using ADC_SetInterruptHandler()
}
 * */
/*  End of File */