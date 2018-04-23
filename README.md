# MSP430-servo-capture
msp430g2553 capture servo input and show result on a 16x2 LCD


This test project uses TIMER0 working from the 1Mhz DCO clock to precisely capture the high-pulse time
input to  the servo via edge-triggered interrrupts. The result in ms is printed to an lcd.
