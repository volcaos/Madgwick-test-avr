/*
 * usart.h
 *
 * Created: 2019/09/24 23:21:50
 *  Author: takeshi
 */ 


#ifndef USART_H_
#define USART_H_

#define USART_BUF_SIZE 128

void usart_init( uint32_t baud, uint32_t fosc );
void usart_write( const uint8_t *data, uint8_t len );
void usart_write_str( const char *format, ... );


#endif /* USART_H_ */