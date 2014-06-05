#ifndef _ADC_H_
#define _ADC_H_
//#include <avr/io.h>
#include <avr/pgmspace.h>
#include "ffft.h"
#include <SpekMatrix.h>



#define cbi(reg, bit_number) reg &= ~(1<<(bit_number))
#define sbi(reg, bit_number) reg |= (1<<(bit_number))

//   adc_init: sets up the uC for analog to digital.
//   Check uC to see if ADATE or ADFR is used for compatibility
//   Check which PORTs house the ADC register for compatibility
void adc_init()
{	
	DDRA = 0x00; PORTA = 0x00;  // PORTA is ADC port in ATmega1284: enable PORTA as IN
	sbi(ADCSRA,ADEN);           // set ADEN bit in ADCSRA to 1: enable ADC       ref 21.3
	cbi(ADCSRA,ADATE);          // set ADATE bit in ADCSRA to 0: enable SSConv.  ref 21.4			
	ADCSRA = ((ADCSRA & 0b11111000) | 0b00000110); // CK/64                      ref 21.5
}

void adc_capture (unsigned char channel, unsigned int *voltage)
{
	unsigned int i;
	
	// ADMUX can select gain in ATMega1284
	// ADMUX[1:0] are channel select for A[1:0]
	// ADMUX[3] == 1... select gain of x10
	// ADMUX[4:1] == 0101... select gain of x200
	// ADMUX [4:2] == 100... no gain 
	
	ADMUX = channel;    	// sets channel and gain (I use CH-0 in call)
							// Right adjusted... ADLAR left out -> 10 bit resolution for optimal accuracy
	
	for (i = 0; i < FFT_N; i++)            // averaging the ADC results
	{
		sbi(ADCSRA,ADSC);                  // start a conversion by writing a one to the ADSC bit (bit 6)
		while(!(ADCSRA & 0x10));           // wait for conversion to complete: ADIF == 1
		*voltage++ = ((ADCL) | (ADCH<<8)); // 10 bit tailoring
	}
}

//das it mane call it in main, mane.
void adc_fft (unsigned char channel)
{
	unsigned int i;
	unsigned int voltage[FFT_N];   // unsigned voltage rating
	int16_t capture[FFT_N];		   // voltage rating adjusted to signed
	complex_t bfly_buff[FFT_N];	   // FFT butterfly buffer */
	uint16_t spektrum[FFT_N/2];	   // Spektrum value buffer as unsigned 16 bit integer
	
	// ADC 5V to 10-bit digital (0x00 - 0x3FF)
	adc_capture(channel, voltage);

	// change 5v to +/-2.5v
	for (i = 0; i < FFT_N; i++)
	{
		capture[i] = voltage[i] - 0x200;
	}

	// Fixed-point FFT routines for megaAVRs, (C)ChaN, 2005
	// http://www.embedds.com/avr-audio-spectrum-monitor-on-graphical-lcd/
	fft_input(capture, bfly_buff);
	fft_execute(bfly_buff);
	fft_output(bfly_buff, spektrum);
	
	// now show the results
	spek_print(spektrum);
}

#endif