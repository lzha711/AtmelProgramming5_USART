/*
 * UART_transmit_only.c
 * Atmega328P send characters and strings to virtual terminal in Proteus
 * Created: 5/18/2021 2:50:25 PM
 * Author : lzha711
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BIT_IS_SET(byte, bit) (byte & (1<<bit)) //if bit is set
#define BIT_IS_CLEAR(byte, bit) (!(byte & (1<<bit))) // if bit is clear

#define FOSC 8000000 //clock speed
#define BAUD 9600
#define MyUBRR (FOSC/16/BAUD - 1)
/*====== for FOSC = 1M, BAUD=9600, MyUBRR is calculated 5.51, after rounding up there is 
  ====== a big offset. Communication will not be successful.  */

/* define initialization functions */
void USART0_init(unsigned int ubrr){
	// set baud rate
	UBRR0H = (unsigned char)(ubrr>>8); //high 8 bits
	UBRR0L = (unsigned char)ubrr;
	//enable receiver and transmitter
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
	//set data frame 8N1
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);
}

void USART0_transmit_char(unsigned char data){
	//wait for the transmit buffer to be empty
	while(BIT_IS_CLEAR(UCSR0A, UDRE0)) ; //when this bit is 1, the buffer is empty
	UDR0 = data;
}

void USART0_transmit_string(char* s){
	//transmit until NULL is received
	while(*s > 0) USART0_transmit_char(*s++);
}

int main(void)
{
    USART0_init(MyUBRR);
    while (1) 
    {
		USART0_transmit_string("HeLLO WOrLd!"); 
		USART0_transmit_char('\r');
		_delay_ms(1000); // send the above message every second
    }
	return(0); // should never get here
}

