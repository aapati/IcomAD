/*File Name adc.h */

#ifndef ADC_H
#define ADC_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#define ATLAGMERES  // több mérés átlagát küldi el  
#define ACQ_US_DELAY 10  // 5 volt eredetileg
#define ACQ_US_DELAY_VSS 5  
//#define ACQ_US_DELAY_Temperature  200

typedef uint16_t adc_result_t;
typedef struct
{
    adc_result_t adcResult1;
    adc_result_t adcResult2;
} adc_sync_double_result_t;

/*
typedef enum
{
    AD_NTC_ICD_CLK = 0x1,
    AD_DC_RA2 =  0x2,
    AD_T2_RA4 =  0x4,
    AD_FUSE_RA5 = 0x5,
    AD_T3_RC0 =  0x10,
    AD_T4_RC1 =  0x11,
    AD_T1_RC2 =  0x12,
    AD_0V_RC3 =  0x13,
    channel_AVSS =  0x3B,
    channel_Temp =  0x3C,
    channel_DAC1 =  0x3D,
    channel_FVR_BUF1 =  0x3E,
    channel_FVR_BUF2 =  0x3F
} adc_channel_t;
 * */

void ADC_Initialize(void);
//void ADC_StartConversion(void);
//bool ADC_IsConversionDone(void);
//adc_result_t ADC_GetConversionResult(void);
adc_result_t ADC_GetConversion(uint8_t channel);

//void ADC_ISR(void);
//void ADC_SetInterruptHandler(void (* InterruptHandler)(void));
//extern void (*ADC_InterruptHandler)(void);
//void ADC_DefaultInterruptHandler(void);

#endif	//ADC_H
/** End of File*/

