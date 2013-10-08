/**
 * @file	LPC_ADC.c
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


#include "Modules/LPC_ADC.h"

SFPResult lpc_analogRead(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t pin = SFPFunction_getArgument_int32(msg, 0);

	if (pin > 7)
		return SFP_ERR_ARG_VALUE;

	LPC_SYSCON->PDRUNCFG &= ~BIT4;			// power up ADC
	LPC_SYSCON->SYSAHBCLKCTRL |= BIT13;		// enable ADC clock
	LPC_ADC->CR = (1 << pin) | (15 << 8) | (1 << 24);	//ADn, clock divider = 15+1, and start conversion

	while (!(LPC_ADC->DR[pin] & BIT31));	// wait for conversion to end
	uint16_t val = (LPC_ADC->DR[pin] >> 6) & 0x3FF;		// read value

	LPC_ADC->CR = 0;						// stop ADC
	LPC_SYSCON->SYSAHBCLKCTRL &= ~BIT13;	// stop ADC clock
	LPC_SYSCON->PDRUNCFG |= BIT4;			// power down ADC

	SFPFunction *outFunc = SFPFunction_new();

	if (outFunc == NULL) return SFP_ERR_ALLOC_FAILED;

	SFPFunction_setType(outFunc, SFPFunction_getType(msg));
	SFPFunction_setID(outFunc, UPER_FID_ANALOGREAD);
	SFPFunction_setName(outFunc, UPER_FNAME_ANALOGREAD);
	SFPFunction_addArgument_int32(outFunc, pin);
	SFPFunction_addArgument_int32(outFunc, val);
	SFPFunction_send(outFunc, &stream);
	SFPFunction_delete(outFunc);

	return SFP_OK;
}
