/*
 * timer.h
 *
 * Created: 2019/09/25 2:04:13
 *  Author: takeshi
 */ 


#ifndef TIMER_H_
#define TIMER_H_

volatile uint64_t TICK_COUNT;

typedef struct {
	uint64_t last_tick;
	uint16_t interval;
} timer_interval_t;

void (*timer_tick_fp)(void);

void timer_init( void (*fp)(void) );
uint64_t get_tick_count(void);
uint8_t timer_check_interval(timer_interval_t*,uint64_t);

#endif /* TIMER_H_ */