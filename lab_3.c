/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;S
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_device_registers.h"

unsigned int nr_overflows = 0;

unsigned char decoderD[10] = {
		0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B }; //hex values from 0 to 9 on 7 seg display
unsigned char decoderC[10] = {
		0xBE, 0x30, 0xAD, 0xB9, 0x33, 0x9B, 0x9F, 0xB0, 0xBF, 0xBB};

void FTM3_IRQHandler(void);

void main(void) {

	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;	// Port C clock enable
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;	//Port D clock enable
	SIM_SCGC3 |= SIM_SCGC3_FTM3_MASK;	// FTM3 clock enable

	PORTC_PCR10 = 0x300;	// Port C Pin 10 as FTM3_CH6 (ALT3)
	FTM3_MODE = 0x5;	// Enable FTM3
	FTM3_MOD = 0xFFFF;
	FTM3_SC = 0x0E;	//System clock / 64

	PORTC_GPCLR = 0x01BF0100;	//Configure for GPIO
	PORTD_GPCLR = 0x00FF0100;

	GPIOC_PDDR = 0x000001BF;	//PORTC [0:5] and [7:8] for output
	GPIOD_PDDR = 0x000000FF;	//PORTD[7:0]  for output

	GPIOC_PCOR = 0x1BF;
	GPIOD_PCOR = 0xFF;	//clears output on PORTD [0:7]

	NVIC_EnableIRQ(FTM3_IRQn);	//Enable FTM3 interrupts
	FTM3_SC |= 0x40;	//Enable TOF

	unsigned int t1, t2, t3, period, pulse_width, duty_cycle, val1, val2;
	while (1) {
		//First edge
		FTM3_CNT = 0; nr_overflows = 0;	//initialize counters
		FTM3_C6SC = 0x4;	// rising edge
		while(!(FTM3_C6SC & 0x80));	// wait for CHF
		FTM3_C6SC &= ~(1 << 7);	// clear CHF
		t1 = FTM3_C6V;	// first edge

		//second edge
		FTM3_C6SC = 0x8;
		while(!(FTM3_C6SC & 0x80));	// wait for CHF
		FTM3_C6SC = 0;	// stop C6
		t2 = FTM3_C6V; // second edge

		//Third edge
		FTM3_C6SC = 0x4;	// rising edge
		while(!(FTM3_C6SC & 0x80));	// wait for CHF
		FTM3_C6SC &= ~(1 << 7);	// clear CHF
		t3 = FTM3_C6V;	// third edge


		if(t2 >= t1){
			pulse_width = (nr_overflows << 16) + (t2 - t1);
		}else{
			pulse_width = ((nr_overflows-1) << 16) + (t2 - t1);
		}

		nr_overflows = 0;

		if(t3 >= t1){
			period = (nr_overflows << 16) + (t3 - t1);
		}else{
			period = ((nr_overflows-1) << 16) + (t3 - t1);
		}

		//Outputting on 7-seg LEDs

		duty_cycle = (unsigned int) ((pulse_width *100.0) / period); //duty_cycle == to counter

		val1 = duty_cycle / 10; //gets 10s place
		val2 = duty_cycle % 10; //gets 1s place

		GPIOC_PCOR = 0x1BF; //Clear output
		GPIOC_PSOR = (unsigned int) decoderC[val1];

		GPIOD_PCOR = 0xFF; //Clear output
		GPIOD_PSOR = (unsigned int) decoderD[val2];

		//for (int i = 0; i < 100000; i++); //delay

	}
}

void FTM3_IRQ_Handler(void){
	nr_overflows++;
	uint32_t SC_VAL = FTM3_SC;
	FTM3_SC &= 0x07; //clear TOF (Timer Overflow Flag)
}
