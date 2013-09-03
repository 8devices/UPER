/**
 * @file	LPC_PWM.c
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

#include "Modules/LPC_PWM.h"

void lpc_pwm0_begin(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1)
		return;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT) return;

	uint32_t p_cyclePeriod = (SFPFunction_getArgument_int32(msg, 0)-1) & 0xFFFF;	// 16 bit value in microseconds

	LPC_SYSCON->SYSAHBCLKCTRL |= BIT7;	// enable clock for CT16B0

	// MAT3 is configured as the driving clock for PWM
	LPC_CT16B0->TCR = BIT0 | BIT1;	// Enable timer, but keep in reset state
	LPC_CT16B0->PR = 48-1;	// 48MHz/48 = 1MHz (1us)

	LPC_CT16B0->MCR = BIT10; // Reset timer on MR3
	LPC_CT16B0->MR3 = p_cyclePeriod; // Set PWM cycle period
	LPC_CT16B0->MR2 = 0; // Set no low time for PWM2
	LPC_CT16B0->MR1 = 0; // Set no low time for PWM1
	LPC_CT16B0->MR0 = 0; // Set no low time for PWM0

	LPC_CT16B0->EMR = 0x02A0; // External outputs enabled: channels 0-2 set high on match
	LPC_CT16B0->PWMC = 0x7;   // PWM channels enabled: set channels 0-2 to PWM control

	LPC_CT16B0->TCR &= ~BIT1;	// disable reset
}

void lpc_pwm0_set(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2)
			return;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT || SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT) return;

	uint32_t p_channelID = SFPFunction_getArgument_int32(msg, 0);			// PWM channel ID [0-2]
	uint32_t p_highTime = SFPFunction_getArgument_int32(msg, 1);	// PWM signal high time in microseconds

	if (p_channelID > 2) return;

	if (LPC_CT16B0->MR3 < p_highTime)
		LPC_CT16B0->MR[p_channelID] = 0;	// Set full high time (0 low time)
	else
		LPC_CT16B0->MR[p_channelID] = LPC_CT16B0->MR3 + 1 - p_highTime;	// Set PWM low time
}

void lpc_pwm0_end(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 0)
		return;

	LPC_CT16B0->TCR = 0;	// Disable timer
	LPC_SYSCON->SYSAHBCLKCTRL &= ~BIT7;	// Disable clock for CT16B0
}

/* 32 bit timer CT32B0 */

void lpc_pwm1_begin(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1)
		return;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT) return;

	uint32_t p_cyclePeriod = SFPFunction_getArgument_int32(msg, 0);	// 32 bit value in microseconds

	LPC_SYSCON->SYSAHBCLKCTRL |= BIT9;	// enable clock for CT32B0

	// MAT3 is configured as the driving clock for PWM
	LPC_CT32B0->TCR = BIT0 | BIT1;	// Enable timer, but keep in reset state
	LPC_CT32B0->PR = 48;	// 48MHz/48 = 1MHz (1us)

	LPC_CT32B0->MCR = BIT10; // Reset timer on MR3
	LPC_CT32B0->MR3 = p_cyclePeriod;	// Set PWM cycle period
	//LPC_CT32B0->MR2 = p_cyclePeriod; // Set full low time for PWM2
	//LPC_CT32B0->MR1 = p_cyclePeriod; // Set full low time for PWM1
	//LPC_CT32B0->MR0 = p_cyclePeriod; // Set full low time for PWM0

	LPC_CT32B0->EMR = 0;	// All external outputs disabled
	LPC_CT32B0->PWMC = 0;	// All PWM channels disabled

	LPC_CT32B0->TCR &= ~BIT1;	// disable reset
}

void lpc_pwm1_set(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2)
			return;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT || SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT) return;

	uint32_t p_channelID = SFPFunction_getArgument_int32(msg, 0);	// PWM channel ID [0-2]
	uint32_t p_highTime = SFPFunction_getArgument_int32(msg, 1); 	// PWM signal high time in microseconds

	if (p_channelID > 2) return;

	LPC_CT32B0->MR[p_channelID] = (LPC_CT32B0->MR3 & 0xFFFF) - p_highTime;	// Set PWM low time
	LPC_CT32B0->EMR |= ((1 << p_channelID) | (0x2 << (p_channelID+p_channelID+4))); // Connect channel output to pin and set pin high on match
	LPC_CT32B0->PWMC |= (1 << p_channelID);	// Enable PWM mode for the channel
}

void lpc_pwm1_end(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 0)
		return;

	LPC_CT32B0->TCR = 0;	// Disable timer
	LPC_SYSCON->SYSAHBCLKCTRL &= ~BIT9;	// Disable clock for CT16B0
}
