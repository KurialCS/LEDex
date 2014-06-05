/*
 * MFFT_ultra.c
 *
 * Created: 6/4/2014 9:07:55 AM
 *  Author: Yohanan
 */ 


#include <avr/io.h>
#include "adc.h"
#include "usart.h"

int main(void)
{
	//PORTA houses ADC
	adc_init();
	//UART communication on USRn where n = 0
	USART_Init();
	
    while(1)
    {
        adc_fft(0);
    }
}