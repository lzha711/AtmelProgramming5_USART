/*
 * ADC_USART.c
 * read ADC from ADC1, and transmit the conversion result thru USART 
 * inspired and modified from https://gist.github.com/lrvdijk/803189
 * Modified on: 5/24/2021 12:37:20 PM
 * Editor: lzha711
 */ 

#ifndef F_CPU 
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

#define FOSC 8000000
#define Baud 9600
#define MyUBRR (FOSC/16/Baud - 1)

unsigned char adc_index = 0; //ADC conversion times

// usart initialization function
void USART_init(uint16_t ubrr){
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	//enable receiver, transmitter and RX complete interrupt
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
	//set data frame 8N1
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);
}

// adc initialization function
void ADC_init(void){
	//select ADC1, left adjust, AVcc as reference
	ADMUX |= (1<<REFS0) | (1<<ADLAR) | (1<<MUX0);
	// enable ADC, interrupt
	ADCSRA |= (1<<ADEN) | (1<<ADIE);
	// pre-scaler 64, which gives ADC 125kHz
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1);
}

//transmit functions:
void USART_transmit_char(unsigned char data){
	//wait for the transmit buffer to be empty
	while(!(UCSR0A & (1<< UDRE0)))
	; //when this bit is 1, the buffer is empty
	UDR0 = data;
}

void USART_transmit_string(char* s){
	//transmit until NULL is received
	while(*s > 0) USART_transmit_char(*s++);
}

// execute when data is received from usart
ISR(USART_RX_vect){
	//capture data from receive buffer
	unsigned char data = UDR0; 
	//start ADC conversion if a 'r' character is received
	if (data == 'r') 
	{
		ADC_init();
		ADCSRA |= (1<<ADSC); //start this conversion
	}
}


// execute when adc conversion is complete:
ISR(ADC_vect){
	if (adc_index <6){
		adc_index ++;
		ADC_init();
		ADCSRA |= (1<<ADSC); //start next conversion
	}else{
		adc_index = 0; //reset index
		UCSR0B |= (1<<UDRIE0); //enable transmit interrupt
	}
}

//execute when transmit interrupt is enabled
ISR(USART_UDRE_vect){
	USART_transmit_string("After ");
	USART_transmit_char(adc_index); // cannot show the times now
	USART_transmit_string("times of ADC conversion, the result is:\r");
	USART_transmit_char(ADCH); //transmit the conversion result of adc, this appears as ASCII, how to change it to decimal?
	// Disable ADC conversion
	ADCSRA &= ~(1<<ADEN);
	// Disable this interrupt
	UCSR0B &= ~(1 << UDRIE0);
}


int main(void)
{
    USART_init(MyUBRR);
	ADC_init();
	//enable interrupts
	sei(); 
    while (1) 
    {}
}

