/*
 * twi.c
 *
 * Created: 2019/09/24 20:28:49
 *  Author: takeshi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <string.h>
#include "twi.h"

uint8_t TWI_TX_BUF[TWI_BUF_SIZE];
uint8_t TWI_RX_BUF[TWI_BUF_SIZE];
volatile uint8_t twi_tx_len = 0;
volatile uint8_t twi_rx_len = 0;
//
void twi_init( uint32_t fscl, uint32_t fcpu )
{
	uint8_t twbr = ((fcpu/fscl-16)/2);
	TWSR = 0;	// PreScaler:1
	TWBR = twbr;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);
}
//
void twi_wait()
{
	while( TWCR & (1<<TWIE) );
}
//
void twi_write( uint8_t *data, uint8_t len )
{
	twi_tx_len = len;
	memcpy((void*)TWI_TX_BUF,data,len);
	twi_rx_len = 0;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);
}
//
void twi_read( uint8_t addr, uint8_t len )
{
	twi_tx_len = 1;
	TWI_TX_BUF[0] = addr<<1|1;
	twi_rx_len = len;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);
}
//
void twi_write_reg( uint8_t addr, uint8_t reg, uint8_t data )
{
	uint8_t d[] = {addr<<1,reg,data};
	memcpy((void*)TWI_TX_BUF,d,3);
	twi_tx_len = 3;
	twi_rx_len = 0;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);
	twi_wait();
}
//
uint8_t* twi_read_reg( uint8_t addr, uint8_t reg, uint8_t len )
{
	uint8_t d[] = {addr<<1,reg};
	twi_wait();
	memcpy((void*)TWI_TX_BUF,d,2);
	twi_tx_len = 2;
	twi_rx_len = len;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);
	twi_wait();
	return TWI_RX_BUF;
}
// 
uint8_t* twi_rx_buf()
{
	return TWI_RX_BUF;
}
// 
ISR(TWI_vect)
{
	static uint8_t tx_pos;
	static uint8_t rx_pos;
	uint8_t stat = TWSR;
	//
	switch(stat)
	{
		case TW_START:			// 0x08 : 送信開始
		case TW_REP_START:		// 0x10 : 再送開始
			tx_pos = 0;
			rx_pos = 0;
		case TW_MT_SLA_ACK:		// 0x18 : 送信アドレス応答
		case TW_MT_DATA_ACK:	// 0x28 : 送信データ応答
			if( tx_pos < twi_tx_len ){
				TWDR = TWI_TX_BUF[tx_pos++];
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);
			}else{
				// restart : 読込シーケンス開始
				if( twi_rx_len > 0 ){
					TWI_TX_BUF[0] = TWI_TX_BUF[0]|1;
					twi_tx_len = 1;
					TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);	// REP_START
				}else{
					TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);		// STOP
				}
			}
			break;
		//
		case TW_MR_DATA_ACK:	// 0x50 : 受信データ応答
			TWI_RX_BUF[rx_pos++] = TWDR;
		case TW_MR_SLA_ACK:		// 0x40 : 受信アドレス応答
			if( rx_pos < twi_rx_len-1 ){
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);	// ACK
			}else{
				TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);	// NACK
			}
			break;
		case TW_MR_DATA_NACK:	// 0x58 : 受信データ非応答
			TWI_RX_BUF[rx_pos] = TWDR;
			TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);
			break;
		case TW_MT_ARB_LOST:
			TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA);
			break;
		case TW_MT_SLA_NACK:
		case TW_MR_SLA_NACK:
		case TW_MT_DATA_NACK:
		case TW_BUS_ERROR:
		default:
			TWCR = (1<<TWEN);
			break;
	}
}
