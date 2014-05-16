/**
 * @file	time.c
 * @author  Giedrius Medzevicius <giedrius@8devices.com>
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 UAB 8devices
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 */

#include "time.h"

#define TIMER_STOP	0
#define TIMER_COUNT	16

volatile time_t 	time_systime;
volatile time_us_t 	time_systime_us;

typedef struct {
	uint32_t timeout;
	TimerCallback callback;
	void* callbackParam;
} Timer_t;

volatile Timer_t time_timers[TIMER_COUNT];

volatile time_t timer_delay;
volatile time_t timer_countdown;

void SysTick_Handler(void) {
	time_systime++;
	time_systime_us += 1000;
	timer_delay--;

	if (timer_countdown != TIMER_STOP) {
		timer_countdown--;
	}

	uint8_t i;
	for (i=0; i<TIMER_COUNT; i++) {
		if (time_timers[i].timeout != TIMER_STOP) {
			if (--time_timers[i].timeout == TIMER_STOP) {
				time_timers[i].callback(time_timers[i].callbackParam);
			}
		}
	}
}

void Time_init(void) {
	time_systime = 0;

	uint8_t i;
	for (i=0; i<TIMER_COUNT; i++) {
		time_timers[i].timeout = TIMER_STOP;
	}

	SysTick_Config(SystemCoreClock/1000);	// Configure Systick to run at 1kHz (1ms)
	NVIC_SetPriority(SysTick_IRQn, 0);
}

time_t Time_getSystemTime(void) {
	return time_systime;
}

time_us_t Time_getSystemTime_us(void) {
	//uint32_t deltaClocks = ((SystemCoreClock/1000 - 1) - SysTick->VAL);
	uint32_t deltaClocks = ((48000 - 1) - SysTick->VAL);
	return time_systime_us + ((deltaClocks*1365) >> 16); // This is equivalent to 1365/65536 ~= 1/48
}

void Time_addTimer(uint32_t timeout, TimerCallback callback, void *param) {
	if (timeout == TIMER_STOP) {
		callback(param);
		return;
	}

	uint8_t i;
	for (i=0; i<TIMER_COUNT; i++) {
		if (time_timers[i].timeout == TIMER_STOP) {
			time_timers[i].callback = callback;
			time_timers[i].callbackParam = param;
			time_timers[i].timeout = timeout;
			return;
		}
	}
}

void Time_delay(uint32_t time) {
	timer_delay = time;
	while (timer_delay);
}

void Time_setCountdown(uint32_t time) {
	timer_countdown = time;
}

uint8_t Time_isCountdownRunning(void) {
	if (timer_countdown == TIMER_STOP) return 0;

	return 1;
}
