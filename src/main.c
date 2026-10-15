#include "MPC5604P_M26V.h"
#include "freemaster.h"
#include "init_base.h"

#define bound(in,lim) ((in>(lim))?(lim):((in < -(lim))?-(lim):in)) //input값을 +- lim값 범위로 지정
#define LPF(out,in,fct) out = out-in>0 ? fct*(out-in)+in:-fct*(in - out)+in
#define FILFCT(wc,fct,ts) fct = ts/(wc+ts)
#define rad2rpm 9.54929
#define TWO_PI 6.28316

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

vuint32_t Pit1cnt = 0;

void init_ADC1(void);
void ADCRead_1(void);
void init_ADC0(void);
void ADCRead_0(void);
void init_PIT(void);
void PIT0ISR(void);
void FLEXPWM0_Init(void);
void PWM_out(void);
void init_GPIO(void);
void LED_Ctr(void);
void Switch(void);
void SlaveDSPI1(void);
void MasterDSPI1(void);
void init_DSPI_1(void);

void init_eTimer(void);
void eTimer_0(void);
void init_PIT1(void);
void PIT1ISR(void);
void Errdetect(void);
void H_BridgeRun(void);
void init_Variable(void);
void PWMISR(void);
void EncoderFdb(void);
void SpeedCon(void);

#define Mastermode 1
#define Slavemode 0

#define SPImode Slavemode

uint8_t ModeVari = SPImode;


volatile int i = 0, mainisr =0;
int test1 = 0, test2 = 0;
char LED1 = 1, LED2 = 1, LED3 = 1, LED4 = 1;
char cnt = 0;
char SW[4] = {0,0,0,0};
char SWold[4] = {0,0,0,0};
int R_adc = 0;
float R_adc_0 = 0;
vuint32_t Pit0cnt = 0, temp =0;
unsigned short FSW_CLOCK_PERIOD = 3200;
unsigned short PWM1_X = 0, PWM1_Y = 0, PWM1_Z = 0;
uint32_t MasterCnt = 0, SlaveCnt = 0;
uint16_t Send_Data = 0, Read_Data =0;
unsigned int echo = 0;
float distance = 0.0f;

// 모터제어
uint8_t Err1 =0, Err2 = 0, ErrCode = 0, DSPRUN = 0;
uint32_t PWM_B = 0;
float VdcRef = 0.0f, FadcScale = 0.0f;

float VdcPre = 0.0f, VdcScale = 0.0f, PWM_Scale = 0.0f;
uint32_t PWM_Peak = 1600;

float IdcPre = 0.0f, iScale =0.0f, IdcFdb = 0.0f, IdcOffset = 0.0f, FiHwScale = 0.0f;

float IdcErr = 0.0f, IdcRef = 0.0f, VdcPterm = 0.0f, VdcIterm = 0.0f, VdcPiterm = 0.0f, vRefUlim = 12.0f;
float Kpc = 3.0f, Kic = 0.3f;


long int delta_m = 0, mold = 0, mnew = 0;
float rpm = 0.0f, EncoderScale = TWO_PI/4.0f, SamplingTime = 0.005f, rpmFil = 0.0f, rpmFilLpfFct = 0.0f;

float rpmErr =0.0f, rpmRef = 0.0f, spdPterm = 0.0f, kpSpd = 0.0f, spdIterm = 0.0f, kiSpd = 0.0f, spdPiterm= 0.0f;
uint8_t SpdCon = 0;

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
//	init_ADC1 ();
    init_ADC0();
    init_PIT();
    FLEXPWM0_Init();
    init_DSPI_1();
    SlaveDSPI1();
    MasterDSPI1();
    init_eTimer();
    init_PIT1();
    init_Variable();
    EncoderFdb();
    SpeedCon();
//	PWMISR();

//	(INTCInterruptFn handlerFn, unsigned short vectorNum,unsigned char psrPriority);
    INTC_InstallINTCInterruptHandler(PIT0ISR, 59, 6);
    INTC_InstallINTCInterruptHandler(PIT1ISR, 60, 7);
    INTC_InstallINTCInterruptHandler(PWMISR, 183, 10);
    INTC_InstallINTCInterruptHandler(eTimer_0, 157, 14);



    SIU.PCR[48].R = 0x0100;

    /* Loop forever */
    for (;;)
    {
        FMSTR_Recorder();
        FMSTR_Poll();

//		ADCRead_1();

        ADCRead_0();
        i++;
        test1 = i;
        test1 = test2;
        /*if(i> 100000)
        {
            SIU.GPDO[55].B.PDO =~GPDO[55].B.PDO;
            i = 0;
        }*/

        LED_Ctr();
//		Switch();
//		PWM_out();

        Errdetect();

    }
}


void init_GPIO(void)
{
    SIU.PCR[52].R = 0x0E00;  //LED1
    SIU.PCR[53].R = 0x0200;	 //LED2
    SIU.PCR[54].R = 0x0200;  //LED3
    SIU.PCR[55].R = 0x0200;  //LED4

    SIU.PCR[48].R = 0x0100;  //Switch1
    SIU.PCR[49].R = 0x0100;	 //SW2
    SIU.PCR[50].R = 0x0100;  //SW3
    SIU.PCR[51].R = 0x0100;  //SW4

    SIU.PCR[64].R = 0x2000;  //ADC_1 AN[5]
    SIU.PCR[64].R = 0x2000;  //ADC_1 AN[5]


    SIU.PCR[12].R = 0x0A00;  //PWMA[2]
    SIU.PCR[13].R = 0x0A00;  //PWMB[2]

    SIU.PCR[0].R = 0x0500;	 //ETC 0 -> Encoder A
    SIU.PCR[1].R = 0x0500;	 //ETC 1 -> Encoder B

    SIU.PCR[58].R = 0x0200;  //EPWM1A, GPIO HIGH
    SIU.PCR[59].R = 0x0200;	 //EPWM1B, GPIO LOW
    SIU.PCR[61].R = 0x0200;  //SR, GPIO LOW
    SIU.PCR[62].R = 0x0600;  //PHASE, FlewPWM0 B1

    SIU.PCR[40].R = 0x0200;	 // PWM RESET -> GPIO HIGH = enable, GPIO LOW = disable
    SIU.PCR[29].R = 0x0100;	 // GATE ERR2, GPIO Input
    SIU.PCR[30].R = 0x0100;	 // GATE ERR1, GPIO Input

    SIU.PCR[23].R = 0x2000;	 // ADC0 AN[0], VBAT(12V)

    SIU.PCR[34].R = 0x2000;	 // ADC0 AN[3], IdcFdb

}

void LED_Ctr(void)
{
  /*  if (R_adc <= 250) {
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



    if (0 <= Pit0cnt <= 9) {
      LED1 = 0; LED2 = 1; LED3 = 1; LED4 = 1;
    } else if (10 <= Pit0cnt <= 19) {
        LED1 = 1; LED2 = 0; LED3 = 1; LED4 = 1;
    } else if (20 <= Pit0cnt <= 29) {
        LED1 = 1; LED2 = 1; LED3 = 0; LED4 = 1;
    } else if (30 <= Pit0cnt <= 39) {
        LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 0;
    } else {
        LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 1;
    }



    SIU.GPDO[52].B.PDO = LED1;
    SIU.GPDO[53].B.PDO = LED2;
    SIU.GPDO[54].B.PDO = LED3;
    SIU.GPDO[55].B.PDO = LED4;
    */

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

    ADC_1.NCMR[0].R = 0x00000009;	//Select ANS 0,3 inputs for conversion

    ADC_1.CDR[5].R = 0x00000000;	//Channel [5] Set default
    ADC_1.CDR[3].R = 0x00000000;	//Channel [3] Set default

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

    ADC_0.NCMR[0].R = 0x00000009;	//Select ANS0 inputs for conversion

    ADC_0.CDR[0].R = 0x00000000;	//Channel[0] Set default
    ADC_0.CDR[3].R = 0x00000000;	//Channel[0] Set default

    ADC_0.MCR.B.ABORT = 0;			// Exit Abort state

}


void ADCRead_0(void)
{
    // 		## SPI 통신
    ADC_0.MCR.B.NSTART = 1; // Module 1 Conversion Start
    asm("nop");

    while(ADC_0.MCR.B.NSTART) asm("nop");
    R_adc_0 = (float)(0.004888*ADC_0.CDR[1].B.CDATA);


    PWM1_X = (unsigned short)(R_adc * 3200.0 / 1024.0);




    // DC 모터 제어
    ADC_0.MCR.B.NSTART = 1; // Module 1 Conversion Start
    asm("nop");


    while(ADC_0.MCR.B.NSTART) asm("nop");


    if(ADC_0.CDR[0].B.VALID == 1)
    {

        VdcPre = (float)ADC_0.CDR[0].B.CDATA*VdcScale;

    }

    if(ADC_0.CDR[3].B.VALID == 1)
    {

        IdcPre = -((float)ADC_0.CDR[3].B.CDATA-512.0f)*iScale;

    }



    VdcScale = 0.01955; // 5/1023*4

    IdcFdb = IdcPre - IdcOffset;

}


void init_PIT(void)
{
    PIT.PITMCR.R = 0x00000001;			// MDIS = 0(module enable), FRZ = 1(debug mode일 떄 정지)
    PIT.CH[0].LDVAL.R = 320000; 	// 6,400,000 // Timeout = (6.4M) x 1sec/ 64M sysclks = 100ms
    PIT.CH[0].TCTRL.R = 0x00000003; 	// Enable PIT0 interrupt & start PIT counting
                                        // TIE = 1(interrupt enable), TEN = 1(time on)


}

void init_PIT1(void)
{
    PIT.PITMCR.R = 0x00000001;			// MDIS = 0(module enable), FRZ = 1(debug mode일 떄 정지)
    PIT.CH[1].LDVAL.R = 640; 	// 6,400,000 // Timeout = (6.4M) x 1sec/ 64M sysclks = 100ms
    PIT.CH[1].TCTRL.R = 0x00000003; 	// Enable PIT0 interrupt & start PIT counting
                                        // TIE = 1(interrupt enable), TEN = 1(time on)

}




void PIT0ISR(void)
{
    if(ModeVari == 1) MasterDSPI1();
    Pit0cnt++;

    EncoderFdb();
/*

    if(Pit0cnt == 40) Pit0cnt = 0;
    Pit0cnt++;

    if (0 <= Pit0cnt <= 9) {
      LED1 = 0; LED2 = 1; LED3 = 1; LED4 = 1;
    } else if (10 <= Pit0cnt <= 19) {
        LED1 = 1; LED2 = 0; LED3 = 1; LED4 = 1;
    } else if (20 <= Pit0cnt <= 29) {
        LED1 = 1; LED2 = 1; LED3 = 0; LED4 = 1;
    } else if (30 <= Pit0cnt <= 39) {
        LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 0;
    } else {
        LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 1;
    }



    SIU.GPDO[52].B.PDO = LED1;
    SIU.GPDO[53].B.PDO = LED2;
    SIU.GPDO[54].B.PDO = LED3;
    SIU.GPDO[55].B.PDO = LED4;

    PIT.CH[0].TFLG.B.TIF = 1; 	// Clear PIT0 flag

*/
    SpeedCon();
    PIT.CH[0].TFLG.B.TIF = 1; // Clear PIT1 flag
}


void FLEXPWM0_Init(void)
{
    //FLEXPWM_0.OUTEN.B.PWMA_EN = 0b0111;
    //FLEXPWM_0.OUTEN.B.PWMB_EN = 0b0111;
    FLEXPWM_0.OUTEN.R = 0x0220;

    /*
    // 0
    FLEXPWM_0.SUB[0].INIT.R = 0xFFFF - FSW_CLOCK_PERIOD+1; // -3200
    FLEXPWM_0.SUB[0].VAL[0].R = 0; // 중간값 설정 0이 아니여도 된다.
    FLEXPWM_0.SUB[0].VAL[1].R = FSW_CLOCK_PERIOD; // 3200 총 6400


    FLEXPWM_0.SUB[0].CTRL2.R = 0x0000;
    FLEXPWM_0.SUB[0].CTRL.R = 0x0C00;
    FLEXPWM_0.SUB[0].OCTRL.R = 0x0000; //출력 반전기능 그러나 사용 안함-> 0000 기본도 0이여서 작성 안해도 무방

    FLEXPWM_0.SUB[0].DISMAP.B.DISX = 0b1111;
    FLEXPWM_0.SUB[0].DISMAP.B.DISB = 0b0000;
    FLEXPWM_0.SUB[0].DISMAP.B.DISA = 0b0000;
    // FLEXPWM_0.SUB[0].DISMAP.R = 0xFF00; 위 3줄과 같은 의미

    FLEXPWM_0.SUB[0].DTCNT0.R = 0; // dead time -> PWM이 동시에 켜지는걸 방지하는 기능 지금은 불필요 -> 0
    FLEXPWM_0.SUB[0].DTCNT1.R = 0;
    */

    //1
    FLEXPWM_0.SUB[1].INIT.R = 0xFFFF - FSW_CLOCK_PERIOD+1; // -3200
    FLEXPWM_0.SUB[1].VAL[0].R = 0; // 중간값 설정 0이 아니여도 된다.
    FLEXPWM_0.SUB[1].VAL[1].R = FSW_CLOCK_PERIOD; // 3200 총 6400


    FLEXPWM_0.SUB[1].CTRL2.R = 0x2000;
    FLEXPWM_0.SUB[1].CTRL.R = 0x0C00;
    FLEXPWM_0.SUB[1].OCTRL.R = 0x0000; //출력 반전기능 그러나 사용 안함-> 0000 기본도 0이여서 작성 안해도 무방

    FLEXPWM_0.SUB[1].DISMAP.B.DISX = 0b1111;
    FLEXPWM_0.SUB[1].DISMAP.B.DISB = 0b0000;
    FLEXPWM_0.SUB[1].DISMAP.B.DISA = 0b0000;
    // FLEXPWM_0.SUB[0].DISMAP.R = 0xFF00; 위 3줄과 같은 의미

    FLEXPWM_0.SUB[1].DTCNT0.R = 0; // dead time -> PWM이 동시에 켜지는걸 방지하는 기능 지금은 불필요 -> 0
    FLEXPWM_0.SUB[1].DTCNT1.R = 0;

    FLEXPWM_0.SUB[1].INTEN.R = 0x0001;

    /*
    //2
    FLEXPWM_0.SUB[2].INIT.R = 0xFFFF - FSW_CLOCK_PERIOD+1; // -3200
    FLEXPWM_0.SUB[2].VAL[0].R = 0; // 중간값 설정 0이 아니여도 된다.
    FLEXPWM_0.SUB[2].VAL[1].R = FSW_CLOCK_PERIOD; // 3200 총 6400


    FLEXPWM_0.SUB[2].CTRL2.R = 0x0000;
    FLEXPWM_0.SUB[2].CTRL.R = 0x0C00;
    FLEXPWM_0.SUB[2].OCTRL.R = 0x0000; //출력 반전기능 그러나 사용 안함-> 0000 기본도 0이여서 작성 안해도 무방

    FLEXPWM_0.SUB[2].DISMAP.B.DISX = 0b1111;
    FLEXPWM_0.SUB[2].DISMAP.B.DISB = 0b0000;
    FLEXPWM_0.SUB[2].DISMAP.B.DISA = 0b0000;
    // FLEXPWM_0.SUB[0].DISMAP.R = 0xFF00; 위 3줄과 같은 의미

    FLEXPWM_0.SUB[2].DTCNT0.R = 0; // dead time -> PWM이 동시에 켜지는걸 방지하는 기능 지금은 불필요 -> 0
    FLEXPWM_0.SUB[2].DTCNT1.R = 0;

    //3
    FLEXPWM_0.SUB[3].INIT.R = 0xFFFF - FSW_CLOCK_PERIOD+1; // -3200
    FLEXPWM_0.SUB[3].VAL[0].R = 0; // 중간값 설정 0이 아니여도 된다.
    FLEXPWM_0.SUB[3].VAL[1].R = FSW_CLOCK_PERIOD; // 3200 총 6400


    FLEXPWM_0.SUB[3].CTRL2.R = 0x2000;
    FLEXPWM_0.SUB[3].CTRL.R = 0x0C00;
    FLEXPWM_0.SUB[3].OCTRL.R = 0x0000; //출력 반전기능 그러나 사용 안함-> 0000 기본도 0이여서 작성 안해도 무방

    FLEXPWM_0.SUB[3].DISMAP.B.DISX = 0b1111;
    FLEXPWM_0.SUB[3].DISMAP.B.DISB = 0b0000;
    FLEXPWM_0.SUB[3].DISMAP.B.DISA = 0b0000;
    // FLEXPWM_0.SUB[0].DISMAP.R = 0xFF00; 위 3줄과 같은 의미

    FLEXPWM_0.SUB[3].DTCNT0.R = 0; // dead time -> PWM이 동시에 켜지는걸 방지하는 기능 지금은 불필요 -> 0
    FLEXPWM_0.SUB[3].DTCNT1.R = 0;
    */


    FLEXPWM_0.MCTRL.B.LDOK |= 0x2;
    FLEXPWM_0.MCTRL.B.RUN |= 0x2;

}


void PWM_out(void)
{

    FLEXPWM_0.SUB[0].VAL[2].R = (unsigned short)-PWM1_X;
    FLEXPWM_0.SUB[0].VAL[3].R = (unsigned short) PWM1_X;

    FLEXPWM_0.SUB[1].VAL[2].R = (unsigned short)-PWM1_Y;
    FLEXPWM_0.SUB[1].VAL[3].R = (unsigned short) PWM1_Y;

    FLEXPWM_0.SUB[2].VAL[2].R = (unsigned short)-PWM1_Z;
    FLEXPWM_0.SUB[2].VAL[3].R = (unsigned short) PWM1_Z;

    FLEXPWM_0.SUB[2].VAL[2].R = (unsigned short)-PWM1_Z;
    FLEXPWM_0.SUB[2].VAL[3].R = (unsigned short) PWM1_Z;


    FLEXPWM_0.SUB[3].VAL[4].R = (unsigned short)-PWM1_X;
    FLEXPWM_0.SUB[3].VAL[5].R = (unsigned short) PWM1_X;

    // 설정 로딩
    FLEXPWM_0.MCTRL.B.LDOK |= 0xF;
    FLEXPWM_0.MCTRL.B.RUN |= 0xF;

}

void init_DSPI_1(void)
{
    #if SPImode
        DSPI_1.MCR.R = 0x80010001;
        DSPI_1.CTAR[0].R = 0x7A0A7727;
        DSPI_1.MCR.B.HALT = 0x0;

        //DSPI_1.RSER.B.RFDFRE

        SIU.PCR[5].R = 0x0604; // output
        SIU.PCR[6].R = 0x0604; // output
        SIU.PCR[7].R = 0x0604; // output
        SIU.PCR[8].R = 0x0103; // input

    #else
        DSPI_1.MCR.R = 0x00010001;
        DSPI_1.CTAR[0].R = 0x7A000000;
        DSPI_1.RSER.B.RFDFRE = 1;
        DSPI_1.MCR.B.HALT = 0x0; // 통신해도 좋다 정지 해제

        SIU.PCR[5].R = 0x0504; // input
        SIU.PCR[6].R = 0x0504; // input
        SIU.PCR[7].R = 0x0604; // output
        SIU.PCR[8].R = 0x0103; // input

        INTC_InstallINTCInterruptHandler(SlaveDSPI1, 98, 5);
    #endif
}


void MasterDSPI1(void)
{
    MasterCnt++;
    DSPI_1.PUSHR.R = (0x00010000|Send_Data); // Send_Data 16bit 0x0001은 설정 0000은 Data 0000이랑 Send_Data랑 OR 연산
    Read_Data = (uint16_t)(DSPI_1.POPR.R&0xFFFF);
    DSPI_1.SR.R = 0x80020000; // Interrupt 플래그 회수
}

void SlaveDSPI1(void)
{

    SlaveCnt++;
    DSPI_1.PUSHR.R = Send_Data;
    Read_Data = (uint16_t)(DSPI_1.POPR.R&0xFFFF);
    DSPI_1.SR.R = 0x80020000;
}

void init_eTimer(void)
{

    ETIMER_0.ENBL.R = 0b000000;		// channel disable

    ETIMER_0.CHANNEL[0].CTRL.R = 0x8100;
    ETIMER_0.CHANNEL[0].FILT.R = 0x0201;
/*
    ETIMER_0.CHANNEL[0].CTRL.B.CNTMODE = 0b001; 	// rising edges count mode
    ETIMER_0.CHANNEL[0].CTRL.B.PRISRC = 0b111111;	// From Table 'Count source values', 64MHz/128 = 500kHz, 2usec
    ETIMER_0.CHANNEL[0].CTRL.B.ONCE = 0;			// Count repeatedly, 반복
    ETIMER_0.CHANNEL[0].CTRL.B.LENGTH = 0;			// Countinue counting to roll over
    ETIMER_0.CHANNEL[0].CTRL.B.DIR = 0;				// Count up 방향
    ETIMER_0.CHANNEL[0].CTRL.B.SECSRC = 0b00000;	// Counter #0 input pin

    // CCC = Compare and Capture Control register
    ETIMER_0.CHANNEL[0].CCCTRL.B.CPT2MODE = 0b01;	// Capture falling edges
    ETIMER_0.CHANNEL[0].CCCTRL.B.CPT1MODE = 0b10;	// Capture rising edges
    ETIMER_0.CHANNEL[0].CCCTRL.B.ONESHOT = 0;		// repeat
    ETIMER_0.CHANNEL[0].CCCTRL.B.ARM = 1;			// enable

    ETIMER_0.CHANNEL[0].CTRL3.B.ROC = 0b01;			// Reload the counter on a capture 1 event

    ETIMER_0.CHANNEL[0].INTDMA.B.ICF2IE = 1;		// Input Capture 2 flag -> interrupt Enable
*/

    ETIMER_0.ENBL.R = 0b000001;		// channel[0] enable

// 	SIU.PCR[0].R = 0x0500;
}


void eTimer_0(void)
{
    ETIMER_0.CHANNEL[0].STS.B.ICF2 = 1; // flag clear

    echo = ETIMER_0.CHANNEL[0].CAPT2.R;	//

    // time = 2usec * echo
    // distance = (time/2)usec * (1s/1,000,000s) * 343m/s * (100cm/m)

    distance =  343*echo*0.0001;		// [cm] 단위
}


void PIT1ISR(void)
{
    Pit1cnt++;
    PIT.CH[1].TFLG.B.TIF = 1; // Clear PIT1 flag

    if(Pit1cnt == 10000) // 0.1usec
    {
        SIU.GPDO[55].B.PDO = 1;
        Pit1cnt = 0;
    }
    else
    {
        SIU.GPDO[55].B.PDO = 0;
    }
}


void Errdetect(void)
{
    Err1 = SIU.GPDI[30].B.PDI;
    Err2 = SIU.GPDI[29].B.PDI;

    if(Err1 && !Err2) 	ErrCode = 100;		// Error: Over Temperature or Voltage
    else if(!Err1 && Err2) 	ErrCode = 101;	// Error: Short Circuit detection
//	else if(Err1 && Err2)	ErrCode = 102; 	// Error: UnderVoltage detection

    if(ErrCode)	DSPRUN = 0;
}


void init_Variable(void)
{
    FadcScale = 5.0f / 1023.0f;
    FiHwScale = 1.0f/10.0f;

    PWM_Peak = 1600;

    PWM_Scale = PWM_Peak / 12; 	//VBAT = 12V

    iScale = (float)(FadcScale/FiHwScale);

    FILFCT(5.0f, rpmFilLpfFct, 200.0f);
}



void H_BridgeRun(void)
{
    /* current PI controller */
    if(DSPRUN) // on
    {
        IdcErr = IdcRef - IdcFdb;
        VdcPterm = Kpc * IdcErr;
        VdcIterm += Kic * IdcErr;	// 원칙적으로는 Kic -> Kic * Tsamp
        VdcIterm = bound(VdcIterm, vRefUlim);
        VdcPiterm = VdcPterm + VdcIterm;
        VdcRef = bound(VdcPiterm, vRefUlim);

        PWM_B = VdcRef*PWM_Scale+PWM_Peak;

        FLEXPWM_0.SUB[1].VAL[4].R = 0x10000-PWM_B;
        FLEXPWM_0.SUB[1].VAL[5].R = PWM_B;

        FLEXPWM_0.MCTRL.B.LDOK |= 0x2;
        FLEXPWM_0.OUTEN.B.PWMB_EN = 0b0010;	//PWM B Enable

        SIU.GPDO[40].B.PDO = 1; // PWM RESET / GPIO Output
        SIU.GPDO[58].B.PDO = 1; // EPWM1A / GPIO Output
        SIU.GPDO[59].B.PDO = 1; // EPWM1B / GPIO Output
        SIU.GPDO[61].B.PDO = 1; // PHASE / GPIO Output
    }

    else // off
    {
        SIU.GPDO[40].B.PDO = 0; // PWM RESET / GPIO Output
        SIU.GPDO[58].B.PDO = 0; // EPWM1A / GPIO Output
        SIU.GPDO[59].B.PDO = 0; // EPWM1B / GPIO Output
        SIU.GPDO[61].B.PDO = 0; // PHASE / GPIO Output

        FLEXPWM_0.OUTEN.B.PWMB_EN = 0x0;	//PWM B Disable

        VdcRef = 0.0f;
        IdcRef = 0.0f;
        VdcIterm = 0.0f;
    }
}


void PWMISR(void)
{
    mainisr++;
    ADCRead_0();
    Errdetect;

    H_BridgeRun();

    FLEXPWM_0.SUB[1].STS.R = 0x0001;
}



void EncoderFdb(void)
{
    mnew = ETIMER_0.CHANNEL[0].CNTR.R;
    delta_m = ((mnew - mold) << 16) >> 16;
    rpm = (float)(delta_m)*EncoderScale / SamplingTime *rad2rpm;
    mold = mnew;



    LPF(rpmFil, rpm, rpmFilLpfFct);
//	rpmFil = (float)(wrFil*rad2rpm);

}



void SpeedCon(void)
{

    if((SpdCon==1)&&(DSPRUN==1))
    {

        rpmErr = rpmRef - rpmFil;
        spdPterm = kpSpd * rpmErr;
        spdIterm += kiSpd * rpmErr;	// 원칙적으로는 Kic -> Kic * Tsamp
        spdIterm = bound(spdIterm, 6);
        spdPiterm = spdPterm + spdIterm;
        IdcRef = bound(spdPiterm, 6);

    }
    else
    {
        rpmRef = 0.0f;
        spdIterm = 0.0f;
    }
}
