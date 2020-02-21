/*
 * mpu9250.c
 *
 * Created: 2019/09/24 20:50:42
 *  Author: takeshi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include "twi.h"
#include "usart.h"
#include "mpu9250.h"
//
#define ACC_FS 2.0f
#define GYR_FS 250.0f
#define CMP_MSENSE 6.665f

//
typedef struct
{
	int16_t acc_x;
	int16_t acc_y;
	int16_t acc_z;
	int16_t gyr_x;
	int16_t gyr_y;
	int16_t gyr_z;
	int16_t cmp_x;
	int16_t cmp_y;
	int16_t cmp_z;
	int16_t temp;
} mpu9250_raw_data_t;
//
int16_t gyr_ofst[3];

// 
void mpu9250_init()
{
	twi_write_reg(0x68,0x6B,0x80);	// PWR_MGMT_1: Reset
	_delay_ms(100);
	twi_write_reg(0x68,0x6B,0x00);	// PWR_MGMT_1:
	twi_write_reg(0x68,0x37,0x02);	// INT_PIN_CFG: BYPASS_EN
	twi_write_reg(0x68,0x19,0x00);	// Sample Rate Divider: 1kHz
	twi_write_reg(0x68,0x1A,2);		// Gyroscope:DLPF:2 (Sampling Rate:1kHz)
	twi_write_reg(0x68,0x1D,2);		// Accelerometer:DLPF:2 (Sampling Rate:1kHz)
	twi_write_reg(0x68,0x1B,0);		// Gyroscope:FS:250dps
	twi_write_reg(0x68,0x1C,0);		// Accelerometer:FS:2g
	twi_write_reg(0x0C,0x0A,0x16);	// 100Hz
}
//
void mpu9250_read_acc( mpu9250_data_t *d )
{
	mpu9250_raw_data_t d0 = {0};
	uint8_t *buf;
	// 
	buf = twi_read_reg(0x68,0x3B,14);
	d0.acc_x = buf[0]<<8 | buf[1];
	d0.acc_y = buf[2]<<8 | buf[3];
	d0.acc_z = buf[4]<<8 | buf[5];
	d0.temp  = buf[6]<<8 | buf[7];
	d0.gyr_x = buf[8]<<8 | buf[9];
	d0.gyr_y = buf[10]<<8 | buf[11];
	d0.gyr_z = buf[12]<<8 | buf[13];
	// 
	d0.gyr_x -= gyr_ofst[0];
	d0.gyr_y -= gyr_ofst[1];
	d0.gyr_z -= gyr_ofst[2];
	// 
	d->acc_x = (float)d0.acc_x / (float)INT16_MAX * ACC_FS;
	d->acc_y = (float)d0.acc_y / (float)INT16_MAX * ACC_FS;
	d->acc_z = (float)d0.acc_z / (float)INT16_MAX * ACC_FS;
	d->gyr_x = (float)d0.gyr_x / (float)INT16_MAX * GYR_FS / 360.0f * 2.0f * M_PI;
	d->gyr_y = (float)d0.gyr_y / (float)INT16_MAX * GYR_FS / 360.0f * 2.0f * M_PI;
	d->gyr_z = (float)d0.gyr_z / (float)INT16_MAX * GYR_FS / 360.0f * 2.0f * M_PI;
	d->temp  = (float)d0.temp / 333.87f + 21.0f;
}
//
void mpu9250_read_cmp( mpu9250_data_t *d )
{
	mpu9250_raw_data_t d0 = {0};
	uint8_t *buf = twi_read_reg(0x0C,0x03,7);
	d0.cmp_x = buf[1]<<8 | buf[0];
	d0.cmp_y = buf[3]<<8 | buf[2];
	d0.cmp_z = buf[5]<<8 | buf[4];
	//
	d->cmp_x = (float)d0.cmp_x / CMP_MSENSE;
	d->cmp_y = (float)d0.cmp_y / CMP_MSENSE;
	d->cmp_z = (float)d0.cmp_z / CMP_MSENSE;
}
//
void mpu9250_print( mpu9250_data_t *d )
{
	/*
	usart_write_str("%+6.3f, %+6.3f, %+6.3f, ",d->acc_x,d->acc_y,d->acc_z);
	usart_write_str("%+6.3f, %+6.3f, %+6.3f, ",d->gyr_x,d->gyr_y,d->gyr_z);
	usart_write_str("%+6.2f, %+6.2f, %+6.2f",d->cmp_x,d->cmp_y,d->cmp_z);
	//usart_write_str("%3.3f\n",d->temp);
	usart_write_str("\n");
	*/
	usart_write_str("%f,%f,%f,%f,%f,%f,%f,%f,%f\r\n",
		d->acc_x,d->acc_y,d->acc_z,
		d->gyr_x,d->gyr_y,d->gyr_z,
		d->cmp_x,d->cmp_y,d->cmp_z
		);
}
// 
void mpu9250_read_average( uint8_t reg, int16_t *dst )
{
	int32_t sum[3] = {0};
	int32_t cnt = 200;
	//
	for( int i=0; i<cnt; i++ ){
		twi_read_reg(0x68,reg,6);
		uint8_t *buf = twi_rx_buf();
		sum[0] += (int32_t)(buf[0]<<8 | buf[1]);
		sum[1] += (int32_t)(buf[2]<<8 | buf[3]);
		sum[2] += (int32_t)(buf[4]<<8 | buf[5]);
		_delay_ms(1);
	}
	for( int i=0; i<3; i++ ){
		dst[i] = sum[i] / cnt;
	}
}
// 
void mpu9250_carib()
{
	mpu9250_read_average(0x43,gyr_ofst);
}
// 
void mpu9250_self_test()
{
	int16_t acc0[3] = {0};
	int16_t gyr0[3] = {0};
	int16_t acc1[3] = {0};
	int16_t gyr1[3] = {0};
	//
	twi_write_reg(0x68,0x6B,0x80);	// PWR_MGMT_1: Reset
	_delay_ms(100);
	twi_write_reg(0x68,0x6B,0x00);	// PWR_MGMT_1:
	twi_write_reg(0x68,0x37,0x02);	// INT_PIN_CFG: BYPASS_EN
	twi_write_reg(0x68,0x19,0x00);	// Sample Rate Divider: 1kHz
	twi_write_reg(0x68,0x1A,2);		// Gyroscope:DLPF:2 (Sampling Rate:1kHz)
	twi_write_reg(0x68,0x1D,2);		// Accelerometer:DLPF:2 (Sampling Rate:1kHz)
	twi_write_reg(0x68,0x1B,0);		// Gyroscope:FS:250dps
	twi_write_reg(0x68,0x1C,0);		// Accelerometer:FS:2g
	//
	mpu9250_read_average(0x3B,acc0);
	mpu9250_read_average(0x43,gyr0);
	//
	twi_write_reg(0x68,0x1B,0b11100000);
	twi_write_reg(0x68,0x1C,0b11100000);
	_delay_ms(20);
	//
	mpu9250_read_average(0x3B,acc1);
	mpu9250_read_average(0x43,gyr1);
	//
	twi_write_reg(0x68,0x1B,0);		// Gyroscope:FS:250dps
	twi_write_reg(0x68,0x1C,0);		// Accelerometer:FS:2g
	_delay_ms(20);
	//
	uint8_t st_code[3];
	float st_otp[3];
	float st_res[3];
	float gyr_offset[3];
	//
	uint8_t _st[3] = {0};
	//twi_write_reg(0x68,0x02,0);
	//twi_wait();
	uint8_t *buf = twi_read_reg(0x68,0x00,3);
	memcpy(st_code,buf,3);
	//
	usart_write_str("%d, %d, %d\r\n",gyr0[0],gyr0[1],gyr0[2]);
	usart_write_str("%d, %d, %d\r\n",gyr1[0],gyr1[1],gyr1[2]);
	usart_write_str("%d, %d, %d\r\n",acc0[0],acc0[1],acc0[2]);
	usart_write_str("%d, %d, %d\r\n",acc1[0],acc1[1],acc1[2]);
	//
	for( int i=0; i<3; i++ ){
		st_res[i] = gyr1[i]-gyr0[i];
		st_otp[i] = (float)(2620) * pow( 1.01 , ((float)st_code[i] - 1.0) );
	}
	//
	for( int i=0; i<3; i++ ){
		usart_write_str("GYRO[%d]: %d, %f, %f\r\n",i,st_code[i],st_res[i],st_otp[i]);
		//
		if( st_otp[i] != 0 ){
			if( st_res[i] / st_otp[i] <= 0.5f ){
				//usart_write_str("GYRO[%d] ERR: %f, %f\r\n",i,st_res[i],st_otp[i]);
			}
		}else{
			if( abs((float)(250.0/32768.0) * st_res[i]) < 60 ){
				//usart_write_str("GYRO[%d] ERR: %f, %f\r\n",i,st_res[i],st_otp[i]);
			}
		}
	}
}
