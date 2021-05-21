/*
 * UART.c
 * The first attempt on using Atmega328P to send strings to com port using UART
 * Simulate in Proteus v8
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

/* define initialization function */
void USART0_init(unsigned int ubrr){
	// set baud rate
	UBRR0H = (unsigned char)(ubrr>>8); //high 8 bits
	UBRR0L = (unsigned char)ubrr;
	//enable receiver and transmitter
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
	//set data frame 8N1
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);
}

/* TRANSMIT A CHARACTER */
void USART0_transmit_char(unsigned char data){
	//wait for the transmit buffer to be empty
	while(BIT_IS_CLEAR(UCSR0A, UDRE0)) 
	; //when this bit is 1, the buffer is empty
	UDR0 = data;
}

/* TRANSMIT A STRING LINE */
void USART0_transmit_string(char* s){
	//transmit until NULL is received
	while(*s > 0) USART0_transmit_char(*s++);
}

/* RECEIVE A CHARACTER */
unsigned char USART0_receive_char(){
	// wait for data to be ready
	while (BIT_IS_CLEAR(UCSR0A, RXC0))
	; //when receive buffer is empty, RXC0 is cleared
	return UDR0;
}

/* RECEIVE A STRING LINE */
void USART0_receive_string(char* buf, uint8_t n){
	uint8_t bufIdx = 0;
	unsigned char c;
	//while received character is not carriage return
	//and end of buffer has not been reached
	do{
		c = USART0_receive_char();
		buf[bufIdx] = c;
		bufIdx++;
	}
	while( (bufIdx<n) && (c !='\r') );
	
	//ensure buffer is null terminated
	buf[bufIdx] = 0;
}


// main program
int main(void)
{
    USART0_init(MyUBRR);
	uint8_t bufSize = 20;
	char buf[bufSize];
	
    while (1) 
    {
		// get line
		USART0_receive_string(buf, bufSize);
		// echo input 
		USART0_transmit_string("The following has been received:\r\n");
		USART0_transmit_string(buf);
    }
	return(0); // should never get here
}

