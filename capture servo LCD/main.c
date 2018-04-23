//c Alex Kallergis 2017
//Copyright (c) 2017 Alex Kallergis

//based on examples from:
// Copyright (c) 2012, Texas Instruments Incorporated
// All rights reserved.

/*This test project uses TIMER0 working from the 1Mhz DCO clock to precisely capture the high-pulse time
 * input to  the servo via edge-triggered interrrupts. The result in ms is printed to an lcd.
 */
//NOTE:LIBRARY FUNCTION ASSUMPTION FOR PRINTF/SPRINTF CHANGED TO FULL
//UNDER BUILD OPTIONS



//servo signal connected to p1.1

/* uC and LCD Connections
	TP1 - Vcc (+5v)
	TP3 - Vss (Gnd)
	P1.0 - D4
	P1.1 - D5
	P1.2 - D6
	P1.3 - D7
	P1.4 - EN
	P1.5 - RS
	Gnd  - RW
	Gnd  - Vee through 1K Resistor	- this value determines contrast - i.e. direct connection to Gnd means all dots displayed
	Gnd  - K (LED-)
	Vcc  - A (LED+) +5V
	Clock: 1MHz             */

#include <msp430.h>
#include <stdio.h>
#include <math.h>

// uC Port definitions
#define lcd_port    	P2OUT
#define lcd_port_dir 	P2DIR

// LCD Registers masks based on pin to which it is connected
#define LCD_EN     	BIT4
#define LCD_RS      BIT5

void lcd_reset()
{
	lcd_port_dir = 0xff;	// output mode
	lcd_port = 0xff;
	__delay_cycles(20000);
	lcd_port = 0x03+LCD_EN;
	lcd_port = 0x03;
	__delay_cycles(10000);
	lcd_port = 0x03+LCD_EN;
	lcd_port = 0x03;
	__delay_cycles(1000);
	lcd_port = 0x03+LCD_EN;
	lcd_port = 0x03;
	__delay_cycles(1000);
	lcd_port = 0x02+LCD_EN;
	lcd_port = 0x02;
	__delay_cycles(1000);
}

void lcd_cmd (char cmd)
{
	// Send upper nibble
	lcd_port = ((cmd >> 4) & 0x0F)|LCD_EN;
	lcd_port = ((cmd >> 4) & 0x0F);

	// Send lower nibble
	lcd_port = (cmd & 0x0F)|LCD_EN;
	lcd_port = (cmd & 0x0F);

	__delay_cycles(2000);
}

void lcd_cmd_fast (char cmd)		//routine for fast commands,smaller delay used
{
	// Send upper nibble
	lcd_port = ((cmd >> 4) & 0x0F)|LCD_EN;
	lcd_port = ((cmd >> 4) & 0x0F);

	// Send lower nibble
	lcd_port = (cmd & 0x0F)|LCD_EN;
	lcd_port = (cmd & 0x0F);

	__delay_cycles(100);
}


void lcd_init ()
{
	lcd_reset();         // Call LCD reset
	lcd_cmd(0x28);       // 4-bit mode - 2 line - 5x7 font.
	lcd_cmd(0x0C);       // Display no cursor - no blink.
	lcd_cmd(0x06);       // Automatic Increment - No Display shift.
	lcd_cmd(0x80);       // Address DDRAM with 0 offset 80h.
	lcd_cmd(0x01);		 // Clear screen
}


void lcd_data (unsigned char dat)
{
	// Send upper nibble
	lcd_port = (((dat >> 4) & 0x0F)|LCD_EN|LCD_RS);
	lcd_port = (((dat >> 4) & 0x0F)|LCD_RS);

	// Send lower nibble
	lcd_port = ((dat & 0x0F)|LCD_EN|LCD_RS);
	lcd_port = ((dat & 0x0F)|LCD_RS);

	__delay_cycles(100); // a small delay may result in missing char display
}

void display_line(char *line)
{
	while (*line)
		lcd_data(*line++);
}


volatile unsigned int diff,temp,of,again;
float tdms;
char value[16];
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                    // Set range
  DCOCTL = CALDCO_1MHZ;                     // Set DCO step + modulation */
  //BCSCTL2 &=~0x08;    //smclk from dco
  P1DIR &= ~BIT1;
  P1SEL |= BIT1;
  TA0CCTL0 = CAP + CM_3 + CCIE + SCS + CCIS_0;
  TA0CTL |= TASSEL_2 + MC_2 + TACLR+ TAIE;        // SMCLK, Cont Mode; start timer

  // Initialize LCD
  	lcd_init();
  	display_line("                 ");
  	__delay_cycles(1000000);
  	lcd_cmd(0x81);			//cursor @0,1
  	display_line("servo capture");
  	lcd_cmd(0xC0);			//cursor @2,3

while(1){
  __bis_SR_register(CPUOFF|GIE);                // CPU off
  tdms=diff/1000.0;		//calculate time difference
  display_line("                 ");
  sprintf(value,"ms= %1.2f",tdms);
  lcd_cmd_fast(0xc0);
  display_line(value);
  //__no_operation();                     // For debugger

}
}

// Timer_A3 Interrupt Vector (TA0IV) handler
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A(void)
{
	if (TA0IV==TA0IV_TAIFG){
		while (of!=0);	//trap. remove later
		if (of==0){
			of=1;
		}
		//TACTL &=~(TAIFG);	//not needed,we have accessed taiv
	}

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{	//CCTL0&=~(CCIFG);	//not needed
	if (again>0)
	{
		again=0;
		diff=TA0CCR0-temp;
		of=0;
		__bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
	}
	else if (TA0CCTL0 & CCI){		//MAKE SURE CCI IS HIGH
		temp=TA0CCR0;
		again=1;
	}
}
