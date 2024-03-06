/*File Name: pin_manager.c */
#include "pin_manager.h"

void PIN_MANAGER_Initialize(void)
{
    ANSELC = 0x0F; //RC0,1,23 4xADC
    ANSELA = 0x36; //RA5,4,2,1 4xADC    
    LATA = 0x37;
    LATC = 0x3F;
    TRISA = 0x36;
    TRISC = 0x2F; //bit..bit5 kivezetve, 0...3 AD, bit4 TX,bit5 RX
          
    WPUA = 0x00;  //0=pull up disable bitenként
    WPUC = 0x00;
    ODCONA = 0x00; //1= open drain kimenet bitenként
    ODCONC = 0x00;
    SLRCONA = 0x37; //1= slew rate limited
    SLRCONC = 0x3F;
    INLVLA = 0x3F; //0=TTL, 1= CMOS Smith Trigger bemenet bitenként
    INLVLC = 0x3F;    	
    RC4PPS = 0x0F;   //RC4->EUSART1:TX1;    
    RX1DTPPS = 0x15; //RC5->EUSART1:RX1;    
}
/* End of File*/