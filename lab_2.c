#include "fsl_device_registers.h"
void software_delay(unsigned long delay)
{
	while (delay > 0) delay--;
}

int main(void)
{
	//Schematic of Board on page 19
	//Enable Ports as Clock Gate Control
	//Need to enable clock Gates in order to work in every module in the MCU
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK; /*Enable Port A Clock Gate Control*/
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK; /*Enable Port C Clock Gate Control*/
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; //Enable Port D Clock Gate Control

	PORTD_GPCLR = 0x00FF0100;	//Configure Port D Pins 0-7 for GPIO
	PORTC_GPCLR = 0x01BF0100;	//Configure Port C Pins 0-5 and 7-8 for GPIO 
	PORTA_GPCLR = 0X00010100;	//Configure Port A Pin 1 GPIO 

	GPIOA_PDDR = 0x00000000;	//Configure Pin 1 for Input; Input = 0; Output = 1;
	GPIOD_PDDR = 0x000000FF;	//Configure Port D Pins 0-7 for Output;
	GPIOC_PDDR = 0x000001BF;	//Configure Port C Pins 0-5 and 7-8 for Output;  //0001 1011 1111 <- PINS 0-5, 7-8


	GPIOC_PDOR = 0x00000000;//Initialize Port C to 0;
	GPIOD_PDOR = 0x00000001;//Initialize Port D such that only 1 bit but is ON;

	int i = 0;
	unsigned long Delay = 0x10000;


	while (1) {

		software_delay(Delay);

		for (i = 0; i < 1000000; i++); //delay for the effect of soft clock

		long CNT_DIR = GPIOA_PDIR; //Read Port A
		long ROT_DIR = GPIOA_PDIR;

		if (CNT_DIR == 0x00) {
			GPIOC_PDOR = GPIOC_PDOR + 1;//increment PORT C
		}
		else {
			GPIOC_PDOR = GPIOC_PDOR - 1; //decrement PORT C
		}
		
		if (ROT_DIR == 0x00) {
			GPIOD_PDOR = GPIOD_PDOR + 1; //left-rotate Port D
		}
		else {
			GPIOD_PDOR = GPIOD_PDOR - 1;//right-rotate Port D
		}
	}
	return 0;
}
