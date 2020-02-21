/*
 * GccApplication1.c
 *
 * Created: 2019/09/12 11:44:12
 * Author : Takeshi Yamaguchi
 */ 


#include "common.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "twi.h"
#include "mpu9250.h"
#include "timer.h"
#include "marg_filter.h"

volatile uint8_t tick=0;

mpu9250_data_t mpu_data;


void tick_func( void )
{
	tick = 1;
}
//
int main(void)
{
	// Pin Direction (0:IN,1:OUT)
	DDRB = (1<<PINB0);
	DDRD = (1<<PIND5)|(1<<PIND1)|(1<<PIND0);
	//
	PORTB |= (1<<PINB0);
	PORTD |= (1<<PIND5);
	PORTD |= (1<<PIND0);
	PORTD |= (1<<PIND1);
	// 
	timer_init(tick_func);
	usart_init(1000000UL,F_CPU);
	twi_init(400000UL,F_CPU);
	sei();
	mpu9250_init();
	mpu9250_carib();
	//
    while(1) 
    {
		static float q[4] = {1.0f,0.0f,0.0f,0.0f};
		// 
		if(tick){
			uint64_t crnt_tick = get_tick_count();
			// 1msec: MPU-9250
			{
				mpu9250_read_acc(&mpu_data);
			}
			// 10msec: AK8963
			{
				static timer_interval_t itv = {0,10};
				//
				if( timer_check_interval(&itv,crnt_tick) ){
					mpu9250_read_cmp(&mpu_data);
					// 
					//filter_update();
					// 
					//float q[4] = {0};
					//read_quaternion(q);
					// 
					//usart_write_str("%f,%f,%f,%f\r\n",q[0],q[1],q[2],q[3]);
					mpu9250_print(&mpu_data);
				}
			}
			// LED: (10 in 1000msec)
			{
				static uint64_t last_tick = 0;
				static uint8_t state = 0;
				uint64_t d = crnt_tick - last_tick;
				//
				if( state == 0 && d < 10 ){
					PORTB &= ~(1<<PINB0);
					PORTD &= ~(1<<PIND5);
					state = 1;
				}else if( state == 1 && d >= 10 ){
					PORTB |= (1<<PINB0);
					PORTD |= (1<<PIND5);
					state = 0;
				}else if( d > 1000 ){
					last_tick += 1000;
				}
			}
			// 
			tick = 0;
		}
    }
}


