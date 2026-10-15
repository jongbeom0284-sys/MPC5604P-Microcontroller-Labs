#include "MPC5604P_M26V.h"
#include "freemaster.h"
#include "init_base.h"

volatile int i = 0;
int test1 = 0, test2 = 0;
char LED1 = 1, LED2 = 1, LED3 = 1, LED4 = 1;
char cnt = 0;
char SW[4] = {0,0,0,0};
char SWold[4] = {0,0,0,0};
int R_adc = 0;
float R_adc_0 = 0;

//unsigned int Blank = 1000;

/*void Pit2ISR(void)
{
    Pit2Cotr++;
    Counter[2]++;

    if(Counter[2]>=Blank)
    {
        SIU.GPDO[55].B.PDO =~ SIU.GPDO[55].B.PDO;
        Counter[2] =0;
    }
    PIT.CH[2].TFLG.B.TIF =1;
}*/

void init_ADC1(void);
void ADCRead_1(void);
void init_ADC0(void);
void ADCRead_0(void);


void init_GPIO(void)
{
    SIU.PCR[52].R = 0x0200;  //LED1
    SIU.PCR[53].R = 0x0200;	 //LED2
    SIU.PCR[54].R = 0x0200;  //LED3
    SIU.PCR[55].R = 0x0200;  //LED4

    SIU.PCR[48].R = 0x0100;  //Switch1
    SIU.PCR[49].R = 0x0100;	 //SW2
    SIU.PCR[50].R = 0x0100;  //SW3
    SIU.PCR[51].R = 0x0100;  //SW4

    SIU.PCR[64].R = 0x2000;  //ADC_1 AN[5]
    SIU.PCR[64].R = 0x2000;  //ADC_1 AN[5]

}


void LED_Ctr(void)
{
    if (R_adc <= 250) {
        LED1 = 0; LED2 = 1; LED3 = 1; LED4 = 1;
    } else if (R_adc > 250 && R_adc <= 500) {
        LED1 = 1; LED2 = 0; LED3 = 1; LED4 = 1;
    } else if (R_adc > 500 && R_adc <= 750) {
        LED1 = 1; LED2 = 1; LED3 = 0; LED4 = 1;
    } else if (R_adc > 750 && R_adc <= 1050) {
        LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 0;
    } else {
        LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 1;
    }

    SIU.GPDO[52].B.PDO = LED1;
    SIU.GPDO[53].B.PDO = LED2;
    SIU.GPDO[54].B.PDO = LED3;
    SIU.GPDO[55].B.PDO = LED4;
}


void Switch(void)
{
    SWold[0] = SW[0];
    SWold[1] = SW[1];
    SWold[2] = SW[2];
    SWold[3] = SW[3];

    SW[0] = SIU.GPDI[48].B.PDI;
    SW[1] = SIU.GPDI[49].B.PDI;
    SW[2] = SIU.GPDI[50].B.PDI;
    SW[3] = SIU.GPDI[51].B.PDI;

    if(!SWold[0] && SW[0]) cnt+= 1;
    if(!SWold[1] && SW[1]) cnt+= 1;
    if(!SWold[2] && SW[2]) cnt+= 1;
    if(!SWold[3] && SW[3]) cnt+= 1;
}

int main(void)
{
    initModesAndClock();
    disableWatchdog();
    enableIrq();
    initOutputClock();
    FMSTR_Init();
    init_INTC();
    init_Linflex0();
    init_GPIO();
    init_ADC1 ();
    ADCRead_0();
    init_ADC0();
    /* Loop forever */
    for (;;)
    {
        FMSTR_Recorder();
        FMSTR_Poll();

        ADCRead_1();

        i++;
        test1 = i;
        test1 = test2;
        /*if(i> 100000)
        {
            SIU.GPDO[55].B.PDO =~GPDO[55].B.PDO;
            i = 0;
        }*/

        LED_Ctr();
        Switch();

    }
}


void init_ADC1(void)
{
    ADC_1.MCR.B.ABORT = 1;  // Abort ADC_1
    ADC_1.MCR.B.OWREN = 0;	// disable overwritting
    ADC_1.MCR.B.WLSIDE = 0;	// conversion data is written right_aligned
    ADC_1.MCR.B.MODE = 0;	// One Shot mode
    ADC_1.MCR.B.CTUEN = 0;	// disable CTU triggered
    ADC_1.MCR.B.ADCLKSEL = 0; 	// Set ADClock 32MHz
//	ADC_1.MCR.B.ADCLKSEL = 1;	// Set ADClock 64MHz
    ADC_1.MCR.B.ACK0 = 0;	// dsable auto clock off
    ADC_1.MCR.B.PWDN = 0;	// disable power down mode

    ADC_1.CTR[0].R = 0x00008208;

    ADC_1.NCMR[0].R = 0x00000020;	//Select ANSS inputs for conversion

    ADC_1.CDR[5].R = 0x00000000;	//Channel [5] Set default

    ADC_1.MCR.B.ABORT = 0;			// Exit Abort state

}


void ADCRead_1(void)
{
    ADC_1.MCR.B.NSTART = 1; // Module 1 Conversion Start
    asm("nop");

    while(ADC_1.MCR.B.NSTART) asm("nop");
    R_adc = ADC_1.CDR[5].B.CDATA;
}


void init_ADC0(void)
{
    ADC_0.MCR.B.ABORT = 1;  // Abort ADC_1
    ADC_0.MCR.B.OWREN = 0;	// disable overwritting
    ADC_0.MCR.B.WLSIDE = 0;	// conversion data is written right_aligned
    ADC_0.MCR.B.MODE = 0;	// One Shot mode
    ADC_0.MCR.B.CTUEN = 0;	// disable CTU triggered
    ADC_0.MCR.B.ADCLKSEL = 0; 	// Set ADClock 32MHz
//	ADC_0.MCR.B.ADCLKSEL = 1;	// Set ADClock 64MHz
    ADC_0.MCR.B.ACK0 = 0;	// dsable auto clock off
    ADC_0.MCR.B.PWDN = 0;	// disable power down mode

    ADC_0.CTR[0].R = 0x00008208;

    ADC_0.NCMR[0].R = 0x00000002;	//Select ANSS inputs for conversion

    ADC_0.CDR[1].R = 0x00000000;	//Channel [1] Set default

    ADC_0.MCR.B.ABORT = 0;			// Exit Abort state

}

void ADCRead_0(void)
{
    ADC_0.MCR.B.NSTART = 1; // Module 1 Conversion Start
    asm("nop");

    while(ADC_0.MCR.B.NSTART) asm("nop");
    R_adc_0 = (float)(0.004888*ADC_0.CDR[1].B.CDATA);
}
