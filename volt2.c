#define DIGIT1 PORTB.F0
#define DIGIT2 PORTB.F1
#define DIGIT3 PORTB.F2
#define DIGIT4 PORTB.F3

//**  Configuration :
//      7Seg = Bit
//        a  =  2      |  0
//        b  =  3      |  1
//        c  =  6      |  2
//        d  =  5      |  3
//        e  =  4      |  4
//        f  =  1      |  5
//        g  =  0      |  6
//       dip =  7      |  u
//**
const unsigned int _pA=0b00000001;
const unsigned int _pB=0b00000010;
const unsigned int _pC=0b00000100;
const unsigned int _pD=0b00001000;
const unsigned int _pE=0b00010000;
const unsigned int _pF=0b00100000;
const unsigned int _pG=0b01000000;
const unsigned int _pH=0b10000000;

long tlong;
unsigned int adc_rd;
unsigned long int Cnt=0;
unsigned char Flag=1;
unsigned char D1,D2,D3,D4=0;

//**
//  Number to 7-Seg:
//**
unsigned char Display(unsigned char no){
    unsigned char Pattern;
    unsigned char SEGMENT[]={
		0xFF&(~_pG)&(~_pH),              // 0 = All but G and H
		_pB|_pC,                         // 1 = B and C
		0xFF&(~_pC)&(~_pF)&(~_pH),       // 2 = All but C, F and H
		0xFF&(~_pE)&(~_pF)&(~_pH),       // 3 = All but E, F and H
		_pB|_pC|_pF|_pG,                 // 4 = B, C, F and G
		0xFF&(~_pB)&(~_pE)&(~_pH),       // 5 = All but B, E and H
		0xFF&(~_pB)&(~_pH),              // 6 = All but B and H
		_pA|_pB|_pC,                     // 7 = A, B and C
		0xFF&(~_pH),                     // 8 = All but H
		0xFF&(~_pE)&(~_pH),              // 9 = All but H
    };
    Pattern=SEGMENT[no];
    return(Pattern);
}

//**
//  TMR0 timer interrupt service routine
//  The program jumps to ISR at every 5ms
//**
void interrupt(){
	TMR0L=100;                         // Reload TMR0L with 100
	//**
	//  Interrup Re-Configuration:
	//  0b
	//  - Disable all interrupts (0)
	//  - Disable all interrupts (0)
	//  - Enable the TMR0 overflow interrupt (1)
	//  - Disables the INT0 external interrupt (0)
	//  - Disables the RB port change interrupt (0)
	//  - TMR0 register did not overflow (0)
	//  - The INT0 external interrupt did not occur (0)
	//  - None of the RB<7:4> pins have changed state (0)
	//  - Total : 0b00100000;
	//**
	INTCON=0x20;                       // Set TMR0IE and clear TMR0IF
	Flag++;
	if(Flag>4){
			Flag=1;
	}
	DIGIT1=DIGIT2=DIGIT3=DIGIT4=0;
	D4=Cnt/1000;
	D3=(Cnt-D4*1000)/100;
	D2=(Cnt-D4*1000-D3*100)/10;
	D1=(Cnt-D4*1000-D3*100-D2*10);
	switch(Flag){
		case 1:
			PORTC=Display(D1);
			DIGIT1=1;
		break;

		case 2:
			PORTC=Display(D2);
			if(Cnt>=10){
				DIGIT2=1;
			}
		break;

		case 3:
			PORTC=Display(D3);
			if(Cnt>=100){
				DIGIT3=1;
			}
		break;

		case 4:
			PORTC=Display(D4);
			if(Cnt>=1000){
				DIGIT4=1;
			}
		break;
	}
}

void main() {
	//**
	//  Oscillator Configuration:
	//  0b
	//  - Device enters sleep on SLEEP instruction (0)
	//  - 4 MHz (110)
	//  - Oscillator Start-up Timer (OST) time-out has expired; primary oscillator is running (1)
	//  - Stable frequency (1)
	//  - Internal oscillator block (10)
	//  - Total : 0b01101110;
	//**
	OSCCON=0x6E;

	//**
	//  Analogue/Digital Configuration:
	//  0b00
	//  - VREF- is connected to VSS (0)
	//  - VREF+ is connected to VDD (0)
	//  - All analogue (0010)
	//  - Total : 0b00000010;
	//**
	ADCON1=0x02;
	//**
	//  Analogue/Digital Configuration:
	//  0b
	//  - Right Justified (1)
	//  0
	//  Don't care (000000)
	//  - Total : 0b10000000;
	//**
	ADCON2=ADCON2|10000000;

	TRISA=0xFF;
	TRISB=0x00;
	TRISC=0x00;

	DIGIT1=0;
	DIGIT2=0;
	DIGIT3=0;

	// Configure TMR0 timer interrup
	// Formula : Time in microseconds = (4 x Clock Period in microseconds) x Prescaler x (256 - TMR0L)
	//**
	//  Timer0 Configuration:
	//  0b
	//  - Enable Timer0 (1)
	//  - Timer0 is configured as an 8-bit timer/counter (1)
	//  - Internal instruction cycle clock (0)
	//  - Increment on low-to-high transition on T0CKI pin (0)
	//  - Timer0 prescaler is assigned. Timer0 clock input comes from prescaler output (0)
	//  - 1:32 Prescale value (100)
	//  - Total : 0b11000100;
	//**
	T0CON=0xC4;
	TMR0L=100;            // Load TMR0L with 100
	//**
	//  Interrup Configuration:
	//  0b
	//  - Enables all unmasked interrupts (1)
	//  - Disable all interrupts (0)
	//  - Enable the TMR0 overflow interrupt (1)
	//  - Disables the INT0 external interrupt (0)
	//  - Disables the RB port change interrupt (0)
	//  - TMR0 register did not overflow (0)
	//  - The INT0 external interrupt did not occur (0)
	//  - None of the RB<7:4> pins have changed state (0)
	//  - Total : 0b10100000;
	//**
	INTCON=0xA0;
	Delay_ms(1000);

	while(1){
		adc_rd  = ADC_read(7);                 // get ADC value from 0th channel
		tlong = (long)adc_rd * 5000;           // covert adc reading to milivolts
		tlong = tlong / 1023;                  // 0..1023 -> 0-5000mV
		Cnt   = tlong;
		Delay_ms(250);
	}
}
