/*
 * timer.c
 *
 * Created: 2019/09/24 20:51:54
 *  Author: takeshi
 */ 

#include "common.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

//
ISR(TIMER1_OVF_vect)
{

}
//
ISR(TIMER1_COMPA_vect)
{
	TICK_COUNT++;
	// 
	timer_tick_fp();
}
//
ISR(TIMER1_CAPT_vect)
{

}
//
void timer_init( void (*fp)(void) )
{
	timer_tick_fp = fp;
	// 
	TCCR1A = (0<<WGM11)|(0<<WGM10);
	//TCCR1B = (1<<WGM12)|(1<<CS12)|(0<<CS11)|(1<<CS10);		// 1/1024
	TCCR1B = (0<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);		// 1/1
	TIMSK1 = (1<<OCIE1A)|(0<<ICIE1)|(0<<TOIE1);	//
	//OCR1A = F_CPU / 1024;	// 15625(5^6): 1000msec
	OCR1A = F_CPU / 1000L;	// 1000: 1msec
}
// 
uint64_t get_tick_count()
{
	return TICK_COUNT;
}
uint8_t timer_check_interval( timer_interval_t *d, uint64_t current )
{
	if( current - d->last_tick > d->interval ){
		d->last_tick += d->interval;
		return 1;
	}
	return 0;
}
