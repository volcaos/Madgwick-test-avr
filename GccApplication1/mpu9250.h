/*
 * mpu9250.h
 *
 * Created: 2019/09/25 1:40:25
 *  Author: takeshi
 */ 


#ifndef MPU9250_H_
#define MPU9250_H_

//
typedef struct
{
	float acc_x;
	float acc_y;
	float acc_z;
	float gyr_x;
	float gyr_y;
	float gyr_z;
	float cmp_x;
	float cmp_y;
	float cmp_z;
	float temp;
} mpu9250_data_t;


void mpu9250_init();
void mpu9250_read_acc( mpu9250_data_t *d );
void mpu9250_read_cmp( mpu9250_data_t *d );
void mpu9250_print( mpu9250_data_t *d );
void mpu9250_carib();


#endif /* MPU9250_H_ */