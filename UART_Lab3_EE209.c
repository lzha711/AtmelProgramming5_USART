/*
 * UART.c
 * EE209 Lab 3 UART
 * Obtain all primes less than 300 and display them on virtual terminals, 
 * obtain the total number of primes too 
 * Simulate in Proteus v8, clock frequency 8MHz
 * Created: 12/Aug/2021 3:36:25 PM
 * Author : Lily Zhang
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define BIT_IS_SET(byte, bit) (byte & (1<<bit)) //if bit is set
#define BIT_IS_CLEAR(byte, bit) (!(byte & (1<<bit))) // if bit is clear

#define FOSC 8000000 //clock speed
#define BAUD 9600
#define MyUBRR (FOSC/16/BAUD - 1)
/*====== for FOSC = 1M, BAUD=9600, MyUBRR is calculated 5.51, after rounding up the error rate
  ====== is high, communication will not be successful.  */

#define PRIME_RANGE 300 // checking number range
#define RXBufSize 100 //for echoing, not needed in this lab
char RXBuf[RXBufSize];
static uint8_t digits[3] = {0,0,0};	//Initializing a 3 digit array
 

/* define initialization function */
void USART0_init(unsigned int ubrr){
	// set baud rate
	UBRR0H = (unsigned char)(ubrr>>8); //high 8 bits
	UBRR0L = (unsigned char)ubrr;
	//enable receiver and transmitter
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
	//enable receive interrupt
	UCSR0B |= (1<<RXCIE0);
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

/* RECEIVE A CHARACTER (Not needed in this lab)*/
unsigned char USART0_receive_char(){
	// wait for data to be ready
	while (BIT_IS_CLEAR(UCSR0A, RXC0))
	; //when receive buffer is empty, RXC0 is cleared
	return UDR0;
}

/* RECEIVE A STRING LINE (Not needed in this lab)*/
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


//Function returns 1 if chk_no is a prime
uint8_t check_prime(uint16_t chk_no){
	for(uint16_t i=2; i<chk_no; i++){
		if( (chk_no % i) == 0){	 //a number that can be divided by number less than itself is not a prime
			return 0;	 //Return FALSE
		}
	}
	return 1;			 //If chk_no cannot be divided by a number less than itself, it is a prime
}


void print_3digit_number(uint16_t number, uint8_t* digits){
	digits[0] = number%10;			//First digit
	digits[1] = (number/10)%10;		//Second digit
	digits[2] = (number/100)%10;		//Third digit
	
	for(uint8_t i = 3; i>0; i--){
		USART0_transmit_char(digits[i-1]+48);		//Transmitting each digit one at a time
	}
	USART0_transmit_char(10);				//Line feed
	USART0_transmit_char(13);			        //carriage return
}


// main program
int main(void)
{
    	USART0_init(MyUBRR);
	// sei(); //enable interrupt
	
	uint16_t prime_array[62];
	prime_array[0] = 2;
	uint8_t prime_index = 1;
	
	for(uint16_t i=3; i<PRIME_RANGE; i++){		//Check if prime number and print
		if(check_prime(i)){
			prime_array[prime_index] = i;
			prime_index++;
			print_3digit_number(i, digits);
		}	
	}
	
	USART0_transmit_string("Total number of primes are: ");
	print_3digit_number(prime_index, digits); //print the total number of primes
	
    	while (1) {	
		// do nothing	
	}
	return(0); // should never get here
}
