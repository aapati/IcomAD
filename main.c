/*  main.c   Device:  PIC16F15323 Driver Version    :  2.00
 * 2021-03-03 verzio folytatasa 2023 j�lius  majd 2024 jan 25-t�l
 * Hibak 2021-02-23 verzi�ban: 
 *  - COMRXTIME=100 nagyon r�vid volt 2024-01-25-ig, megn�velve 10 000-re  tobb intercom modul lekerdezesekor mindig soros kapcsolat hibabol indul, talan ez�rt nem megy a ' ' kiolvasas
 *  - egyszeres ' ' lek�rdez�s nem m�k�dik j�l 
 *  - nagyon lassan v�laszol lek�rdez�sre 100...250msec
 * 2024-01-26 a soros a kommunik�ci� felgyors�tva
 * Projekt �tmenevezve IcomAD2024-re 2024-03-06
 */

#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/adc.h"
#include <stdio.h>
#include <string.h>

#define VERZIO 1v1  //2024 janu�r 25-t�l

//#define  HANGBUSZMER   //CTRL modul hangsin mero adapter �ramk�rbe 

#define ARAMOFFSET 0x20   //�ramm�ro tipikus mert ert�k ha nincs aram
#define TULARAM    0x19   //offsethez kepest
#define COMRXTIME 10000   //(WORD) ennyi ciklust v�r a soros vonalon lek�rdez�sre, ha nincs lek�rdez�s akkor hiba eset�n statikus jelz�st ad (BREAK) a soros vonalra

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
     // bit7=1 soros vonalra az ad�s eng; 
     // bit6=
       // bit5=1 biztos�t�k hiba, 
       // bit4=1 tul�ram, 
       // bit3=1 24V t�pfesz hiba, 
       // bit2=1 +0V bizti hiba
     // bit1=1 wdog stop var resetre; 
     // bit0=1 folyamatos ad�s a soros vonalra;  

uint16_t timerrx; // v�r v�telre
uint8_t  uart_adbuf[32];
static uint8_t  bitkonv_tbl[] = {4,0,6,2,1,7,5,3};  // {4,0,6,?,1,?,3,5}; k�rd�jeles �rt�kek lehet hogy rosszak (felcser�lend?k)! (HW hib�k matt)
uint8_t h_atlag_cnt[8];
// - - - - - - - - -
void main(void)
{  
    SYSTEM_Initialize();  // initialize the device
    uzemmod = 0;             
      //uzemmod = 0x81; //soros adas eng, folyamatosan 
      //uzemmod=0x40;  //aszinkron soros vonal helyett statikus hibajelz�s ad�sa Tx-en, Intercom rd3/7 bit-en beolvassa
    timerrx = COMRXTIME;
       
    INTERRUPT_GlobalInterruptEnable(); // b�rmelyik IT-hez kell hogy eng.
    INTERRUPT_PeripheralInterruptEnable(); //(itt a soros vonal) IT-hez kell   
    WWDT_SoftEnable();
   
// - - - - - - - - - - - - - -      
    while (1) {  //*****   f�ciklus *******
        if( !(uzemmod & 2)) { //bit1 wdog
            CLRWDT(); 
        }  //else  NOP(); //debug,nincs wdog frissites        
        if(timerrx)  timerrx--;  

    // A/D m�r�s 8 csatorn�n       
        for(i=0;i<8; i++){
            adc_mertadat[i]=0; 
#ifdef HANGBUSZMER            
            adc_mertadat1[i]=0;
            adc_mertadat2[i]=0;
#endif            
        }
#ifdef HANGBUSZMER
        for(i=0; i<8; i++)  h_atlag_cnt[i]=0;  //clr minta cnt
        
        for(j=0; j<64; j++){ //m�r�s �tlag �s 10 bit->16 bit atalakitasa. a ciklus kb.1.2msec
            for(i=0; i<8; i++){ 
                tmpw = ADC_GetConversion(i); //10 bit!
                if(tmpw > 511){   // hang 0 pont      
                    tmpw -= 511;
                }else { 
                    tmpw = 511 - tmpw;
                } 
            /*    
                if(h_atlag_cnt[i]) { //0 oszt�s elker�l�se
                    if(tmpw > (adc_mertadat[i]/(h_atlag_cnt[i]*4) ) ) {
                        adc_mertadat[i] += tmpw;
                        h_atlag_cnt[i]++;
                    }    
                }else{ //els� m�rt adat   
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
        
        for(i=0;i<8; i++){  // 16->8 bit konverzio, �s mintasz�m szerinti korrekci�
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
#else   //Intercom  t�pm�r�s    
        for(j=0; j<64; j++){ // t�bb m�r�s �tlaga 10 bit->16 bit. a ciklus kb.1.2msec
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
 
    
    //a mert adatok kiert�kel�se, a hiba flag-ek be�llit�sa 
        uzemmod &= 0xc3;  //bit5-4-3-2 hiba allapotok clr
        for(i=0;i<4; i++){
           if(adc_mertadatB[i] > 0x30) 
              uzemmod |= 0x20;  //bit5=1 biztos�t�k hiba a 4 kimenet egyik�n vagy t�bben
        }
        if(adc_mertadatB[7] > (aramoffset + TULARAM) )
           uzemmod |= 0x10;  //bit4 set tul�ram
        if( (adc_mertadatB[4] < 0x50) || (adc_mertadatB[4] > 0x90) )// -24V fesz m�r�se m�rt:  0x86 = 28V, 0x7e = 26,7V  0x50 = 20V
           uzemmod |= 0x08;  //bit3 set t�pfesz hiba
         
        if( ( (adc_mertadatB[4] - adc_mertadatB[5]) > 20) || ( (adc_mertadatB[5] - adc_mertadatB[4]) > 20) ) //szakadt a +0V bizti
           uzemmod |= 0x04;  //bit2 set 0V bizti hiba        
 #endif         
        //k�sz egy m�r�s, soros ad�s buffer el?k�sz�t�se 
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
    } //f�ciklus v�ge  while()  
} //main
// - - - - - - - 
void vetel(void) //vetel a soros vonalon
{
    if(eusart1RxCount) {  //vett adat ??
        rxData = EUSART1_Read();  //hiv�si felt�tel eusart1RxCount >0            
        timerrx = COMRXTIME;                       
        if(rxData == ' ') { //egyszeres lek�rdez�s clr wdog tilt bit
            uzemmod |= 0x80;
            uzemmod &= 0xfe; //bit0 clr
        }else if(rxData == 'a') { 
            uzemmod |= 0x81; //folyamatos ad�s a soros vonalra clr wdog tilt bitet                
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
        EUSART1_Write(' ');   // 'space' mert az elso karakter k�ld�se hib�s lehet break-bol visszatereskor
    }    
}
// - - - - - - - 
void adas(void)
{
    if(uzemmod & 0x80) { //ad�s a soros vonalra 
        if(!(uzemmod & 1)) {// ha bit0 =0 egyszer ad, =1 folyamatos ad�s, allapot string k�ld
             uzemmod &= 0x7f; //bit7 clr egyszeres adas t�bbet nem ad
        } 
        adas_init();  //ha kell (BAUD1CON & 0x10)   
        for(i=0;i<30; i++){   //3+(3*9))
            __delay_us(1100);
            EUSART1_Write(uart_adbuf[i]);   
        }        
    }else if( !timerrx ) { //nincs ad�s, a timer lejart, statikus hibajelz�s ad�sa  ha kell
        if(uzemmod & 0x3c) {  // hiba bit5...2
            if( !(BAUD1CON & 0x10) )  
                EUSART1_Init_txLOW();   //TX folyamatos L = hiba                 
        }else //hibajelz�s megsz�ntet�se a TX-en
                adas_init();             
    }          
}
// - - - - - 
//  End of main.c