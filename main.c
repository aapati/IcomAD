/*  main.c   Device:  PIC16F15323 Driver Version    :  2.00
 * 2021-03-03 verzio folytatasa 2023 július  majd 2024 jan 25-töl
 * Hibak 2021-02-23 verzióban: 
 *  - COMRXTIME=100 nagyon rövid volt 2024-01-25-ig, megnövelve 10 000-re  tobb intercom modul lekerdezesekor mindig soros kapcsolat hibabol indul, talan ezért nem megy a ' ' kiolvasas
 *  - egyszeres ' ' lekérdezés nem müködik jól 
 *  - nagyon lassan válaszol lekérdezésre 100...250msec
 * 2024-01-26 a soros a kommunikáció felgyorsítva
 * Projekt átmenevezve IcomAD2024-re 2024-03-06
 */

#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/adc.h"
#include <stdio.h>
#include <string.h>

#define VERZIO 1v1  //2024 január 25-töl

//#define  HANGBUSZMER   //CTRL modul hangsin mero adapter áramkörbe 

#define ARAMOFFSET 0x20   //áramméro tipikus mert erték ha nincs aram
#define TULARAM    0x19   //offsethez kepest
#define COMRXTIME 10000   //(WORD) ennyi ciklust vár a soros vonalon lekérdezésre, ha nincs lekérdezés akkor hiba esetén statikus jelzést ad (BREAK) a soros vonalra

void vetel(void);      //vetel a soros vonalon
void adas_init(void);  //soros vonalon adas init ha kell
void adas();

extern volatile uint8_t eusart1RxCount;
volatile uint8_t rxData;  
adc_result_t adc_mertadat[8],tmpw; //16 bit
#ifdef HANGBUSZMER  
  adc_result_t adc_mertadat1[8], adc_mertadat2[8];
#endif
uint8_t  adc_mertadatB[8]; //8 bit
uint8_t i,j, soroskapcsolat=0, aramoffset = ARAMOFFSET;
char buff[5];
uint8_t tmp;
uint8_t uzemmod; //vegyes uzemmod byte
     // bit7=1 soros vonalra az adás eng; 
     // bit6=
       // bit5=1 biztosíték hiba, 
       // bit4=1 tuláram, 
       // bit3=1 24V tápfesz hiba, 
       // bit2=1 +0V bizti hiba
     // bit1=1 wdog stop var resetre; 
     // bit0=1 folyamatos adás a soros vonalra;  

uint16_t timerrx; // vár vételre
uint8_t  uart_adbuf[32];
static uint8_t  bitkonv_tbl[] = {4,0,6,2,1,7,5,3};  // {4,0,6,?,1,?,3,5}; kérdöjeles értékek lehet hogy rosszak (felcserélend?k)! (HW hibák matt)
uint8_t h_atlag_cnt[8];
// - - - - - - - - -
void main(void)
{  
    SYSTEM_Initialize();  // initialize the device
    uzemmod = 0;             
      //uzemmod = 0x81; //soros adas eng, folyamatosan 
      //uzemmod=0x40;  //aszinkron soros vonal helyett statikus hibajelzés adása Tx-en, Intercom rd3/7 bit-en beolvassa
    timerrx = COMRXTIME;
       
    INTERRUPT_GlobalInterruptEnable(); // bármelyik IT-hez kell hogy eng.
    INTERRUPT_PeripheralInterruptEnable(); //(itt a soros vonal) IT-hez kell   
    WWDT_SoftEnable();
   
// - - - - - - - - - - - - - -      
    while (1) {  //*****   föciklus *******
        if( !(uzemmod & 2)) { //bit1 wdog
            CLRWDT(); 
        }  //else  NOP(); //debug,nincs wdog frissites        
        if(timerrx)  timerrx--;  

    // A/D mérés 8 csatornán       
        for(i=0;i<8; i++){
            adc_mertadat[i]=0; 
#ifdef HANGBUSZMER            
            adc_mertadat1[i]=0;
            adc_mertadat2[i]=0;
#endif            
        }
#ifdef HANGBUSZMER
        for(i=0; i<8; i++)  h_atlag_cnt[i]=0;  //clr minta cnt
        
        for(j=0; j<64; j++){ //mérés átlag és 10 bit->16 bit atalakitasa. a ciklus kb.1.2msec
            for(i=0; i<8; i++){ 
                tmpw = ADC_GetConversion(i); //10 bit!
                if(tmpw > 511){   // hang 0 pont      
                    tmpw -= 511;
                }else { 
                    tmpw = 511 - tmpw;
                } 
            /*    
                if(h_atlag_cnt[i]) { //0 osztás elkerülése
                    if(tmpw > (adc_mertadat[i]/(h_atlag_cnt[i]*4) ) ) {
                        adc_mertadat[i] += tmpw;
                        h_atlag_cnt[i]++;
                    }    
                }else{ //elsö mért adat   
                    adc_mertadat[i] = tmpw;
                    h_atlag_cnt[i]++;
                }
             */
                if(tmpw > adc_mertadat[i]){
                    adc_mertadat[i] = tmpw;
                }else if(tmpw > adc_mertadat1[i]) {  
                    adc_mertadat1[i] = tmpw;
                }else if(tmpw > adc_mertadat2[i]) {  
                    adc_mertadat2[i] = tmpw;
                }     
            }
            vetel();
            adas(); //ad ha kell
        }  //for j        
        
        for(i=0;i<8; i++){  // 16->8 bit konverzio, és mintaszám szerinti korrekció
            /*
            tmpw = adc_mertadat[i]/h_atlag_cnt[i];
            if(h_atlag_cnt[i] > 15) {
               tmpw >>6;
            }else if(h_atlag_cnt[i] > 10) {          
               tmpw  >>7;  
            }else if(h_atlag_cnt[i] > 5) {          
               tmpw  >>8;    
            }else {
                tmpw >>9; 
            }    
            */
            tmpw = (adc_mertadat[i] + adc_mertadat1[i] +adc_mertadat2[i])/3;
            
            if( tmpw >= adc_mertadatB[bitkonv_tbl[i]] )
                adc_mertadatB[bitkonv_tbl[i]] = (uint8_t)tmpw;
            else
                adc_mertadatB[bitkonv_tbl[i]] = (uint8_t)(tmpw + adc_mertadatB[bitkonv_tbl[i]])/2;                           
        }
#else   //Intercom  tápmérés    
        for(j=0; j<64; j++){ // több mérés átlaga 10 bit->16 bit. a ciklus kb.1.2msec
            for(i=0;i<8; i++){ 
                adc_mertadat[i]+= ADC_GetConversion(i);   
                 __delay_us(2); //kell????
            }
            vetel();
            adas(); //ad ha kell  
        }  //for j        
        for(i=0;i<8; i++){  // 16->8 bit konverzio
           adc_mertadatB[i] = (uint8_t)((adc_mertadat[i]>>8) & 0x00ff);             
        }                
 
    
    //a mert adatok kiertékelése, a hiba flag-ek beállitása 
        uzemmod &= 0xc3;  //bit5-4-3-2 hiba allapotok clr
        for(i=0;i<4; i++){
           if(adc_mertadatB[i] > 0x30) 
              uzemmod |= 0x20;  //bit5=1 biztosíték hiba a 4 kimenet egyikén vagy többen
        }
        if(adc_mertadatB[7] > (aramoffset + TULARAM) )
           uzemmod |= 0x10;  //bit4 set tuláram
        if( (adc_mertadatB[4] < 0x50) || (adc_mertadatB[4] > 0x90) )// -24V fesz mérése mért:  0x86 = 28V, 0x7e = 26,7V  0x50 = 20V
           uzemmod |= 0x08;  //bit3 set tápfesz hiba
         
        if( ( (adc_mertadatB[4] - adc_mertadatB[5]) > 20) || ( (adc_mertadatB[5] - adc_mertadatB[4]) > 20) ) //szakadt a +0V bizti
           uzemmod |= 0x04;  //bit2 set 0V bizti hiba        
 #endif         
        //kész egy mérés, soros adás buffer el?készítése 
        uart_adbuf[0]= ' '; //a frame hiba miatt kellhet
        uart_adbuf[1]= 13;
        uart_adbuf[2]= 10;
        j=3;
        for(i=0; i<9; i++){
            if(i < 8) {                               
               tmp = adc_mertadatB[i];              
            }else
               tmp =  uzemmod;             
            sprintf(buff,"%2.2x",tmp);
            uart_adbuf[j++] = buff[0];
            uart_adbuf[j++] = buff[1];
            uart_adbuf[j++] = ' ';
        }
    } //föciklus vége  while()  
} //main
// - - - - - - - 
void vetel(void) //vetel a soros vonalon
{
    if(eusart1RxCount) {  //vett adat ??
        rxData = EUSART1_Read();  //hivási feltétel eusart1RxCount >0            
        timerrx = COMRXTIME;                       
        if(rxData == ' ') { //egyszeres lekérdezés clr wdog tilt bit
            uzemmod |= 0x80;
            uzemmod &= 0xfe; //bit0 clr
        }else if(rxData == 'a') { 
            uzemmod |= 0x81; //folyamatos adás a soros vonalra clr wdog tilt bitet                
        }else if(rxData == 'W') { //bit1 wdog frissites kikapcs. reset teszt           
            uzemmod |= 0x02;
        }
    }   
}
// - - - -
void adas_init(void) //init ha kell
{
    if(BAUD1CON & 0x10) {
        EUSART1_Initialize();      
        EUSART1_Write(' ');   // 'space' mert az elso karakter küldése hibás lehet break-bol visszatereskor
    }    
}
// - - - - - - - 
void adas(void)
{
    if(uzemmod & 0x80) { //adás a soros vonalra 
        if(!(uzemmod & 1)) {// ha bit0 =0 egyszer ad, =1 folyamatos adás, allapot string küld
             uzemmod &= 0x7f; //bit7 clr egyszeres adas többet nem ad
        } 
        adas_init();  //ha kell (BAUD1CON & 0x10)   
        for(i=0;i<30; i++){   //3+(3*9))
            __delay_us(1100);
            EUSART1_Write(uart_adbuf[i]);   
        }        
    }else if( !timerrx ) { //nincs adás, a timer lejart, statikus hibajelzés adása  ha kell
        if(uzemmod & 0x3c) {  // hiba bit5...2
            if( !(BAUD1CON & 0x10) )  
                EUSART1_Init_txLOW();   //TX folyamatos L = hiba                 
        }else //hibajelzés megszüntetése a TX-en
                adas_init();             
    }          
}
// - - - - - 
//  End of main.c