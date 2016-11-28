/**
 * @file	LPC_1WIRE.c
 * @author  Giedrius Medzevicius <giedrius@8devices.com>
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 UAB 8devices
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


#include "Modules/LPC_1WIRE.h"
#include "Modules/LPC_GPIO.h"

extern uint8_t const LPC_PIN_IDS[];
extern volatile uint32_t * const LPC_PIN_REGISTERS[];
#define LPC_PIN_MODE_MASK		(3 << 3)

static int pin;

SFPResult lpc_1wire_begin(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1)
			return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT)
			return SFP_ERR_ARG_TYPE;

	pin = SFPFunction_getArgument_int32(msg, 0);

	uint8_t port = 0;
	uint8_t pinNum = LPC_PIN_IDS[pin];
	if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
		port = 1;
		pinNum -= 24;
	}

	*LPC_PIN_REGISTERS[pin] &= ~LPC_PIN_MODE_MASK;	// Remove pull-up/down resistors
	LPC_GPIO->DIR[port] |= (1 << pinNum);	// Set direction bit (output)
	LPC_GPIO->CLR[port] = (1 << pinNum);

	return SFP_OK;
}

SFPResult lpc_1wire_trans(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_BYTE_ARRAY)
		return SFP_ERR_ARG_TYPE;

	uint8_t port = 0;
	uint8_t pinNum = LPC_PIN_IDS[pin];
	if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
		port = 1;
		pinNum -= 24;
	}

	uint32_t len = 0;
	uint8_t *buf = SFPFunction_getArgument_barray(msg, 0, &len);

	int delay_high, delay_low;

	__disable_irq();
	while (len--) {
		uint8_t b = *buf++;
		int n;

#define BIT0H  4
#define BIT0L  19
#define BIT1H  16
#define BIT1L  4-2

		for (n=7; n>=0; n--) { // MSB first
			if (b & (1 << n)) {
				delay_high = BIT1H;
				delay_low = BIT1L;
			} else {
				delay_high = BIT0H;
				delay_low = BIT0L;
			}

			LPC_GPIO->SET[port] = (1 << pinNum);
			while (delay_high--) __asm("nop");
			LPC_GPIO->CLR[port] = (1 << pinNum);
			while (delay_low--) __asm("nop");
		}

	}
	__enable_irq();

	return SFP_OK;
}
