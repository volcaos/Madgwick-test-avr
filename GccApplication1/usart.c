/*
 * usart.c
 *
 * Created: 2019/09/24 20:31:44
 *  Author: takeshi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include "usart.h"
// 
uint8_t tx_buf[USART_BUF_SIZE];
volatile uint8_t tx_pos = 0, tx_tail = 0;
uint8_t rx_buf[USART_BUF_SIZE];
volatile uint8_t rx_pos = 0, rx_tail = 0;

// 
void usart_init( uint32_t baud, uint32_t fosc )
{
	uint16_t myybrr = (fosc/16/baud-1);
	UBRR1 = myybrr;
	UCSR1B = (1<<TXEN1) | (1<<RXEN1) | (1<<RXCIE1);		// TX,RX,RX_int
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);		// Parity:None, Data:8, Stop:1
}
//
void usart_write( const uint8_t *data, uint8_t len )
{
	uint8_t tail = (tx_tail+len)%USART_BUF_SIZE;
	//
	if( tx_pos < tail ){
		memcpy((void*)&(tx_buf[tx_tail]),data,len);
	}else{
		uint8_t d = len - tail;
		memcpy((void*)tx_buf+tx_tail,data,d);
		memcpy((void*)tx_buf,data+d,len-d);
	}
	//
	if( tx_pos == tx_tail ){
		UCSR1B |= (1<<UDRE1);
	}
	tx_tail = tail;
}
//
void usart_write_str( const char *format, ... )
{
	char buf[USART_BUF_SIZE];
	va_list va;
	//
	va_start(va, format);
	vsprintf((void*)buf,format,va);
	va_end(va);
	usart_write((const uint8_t*)buf,strlen(buf));
}
//
ISR(USART1_RX_vect)
{
	rx_buf[rx_tail] = UDR1;
	rx_tail = (rx_tail+1)%USART_BUF_SIZE;
}
//
ISR(USART1_UDRE_vect)
{
	if( tx_pos != tx_tail ){
		UDR1 = tx_buf[tx_pos];
		tx_pos = (tx_pos+1)%USART_BUF_SIZE;
	}else{
		UCSR1B &= ~(1<<UDRE1);
	}
}