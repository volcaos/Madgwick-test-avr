/*
 * twi.h
 *
 * Created: 2019/09/24 23:41:17
 *  Author: takeshi
 */ 


#ifndef TWI_H_
#define TWI_H_

#define TWI_BUF_SIZE 64

void twi_init( uint32_t fscl, uint32_t fcpu );
void twi_wait();
void twi_write( uint8_t *data, uint8_t len );
void twi_read( uint8_t addr, uint8_t len );
void twi_write_reg( uint8_t addr, uint8_t reg, uint8_t data );
uint8_t* twi_read_reg( uint8_t addr, uint8_t reg, uint8_t len );
uint8_t* twi_rx_buf();


#endif /* TWI_H_ */