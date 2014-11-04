/**
 * @file	LPC_GPIO.c
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

#include "Modules/LPC_GPIO.h"

uint8_t const LPC_PIN_IDS[] = {
		0+20,	0+2,	24+26,	24+27,	24+20,	0+21,	24+23,	24+24,	// 8
		0+7,	24+28,	24+31,	24+21,	0+8,	0+9,	0+10,	24+29,	// 16
		24+19,	24+25,	24+16,	0+19,	0+18,	0+17,	24+15,	0+23,	// 24
		0+22,	0+16,	0+15,	24+22,	24+14,	24+13,	0+14,	0+13,	// 32
		0+12,	0+11,	// 34
};

volatile uint32_t * const LPC_PIN_REGISTERS[] = {
		&LPC_IOCON->PIO0_20, 	&LPC_IOCON->PIO0_2 , 	&LPC_IOCON->PIO1_26, 	&LPC_IOCON->PIO1_27,
		&LPC_IOCON->PIO1_20, 	&LPC_IOCON->PIO0_21, 	&LPC_IOCON->PIO1_23, 	&LPC_IOCON->PIO1_24,		// 8
		&LPC_IOCON->PIO0_7 , 	&LPC_IOCON->PIO1_28, 	&LPC_IOCON->PIO1_31, 	&LPC_IOCON->PIO1_21,
		&LPC_IOCON->PIO0_8 ,	&LPC_IOCON->PIO0_9 , 	&LPC_IOCON->SWCLK_PIO0_10, &LPC_IOCON->PIO1_29,		// 16
		&LPC_IOCON->PIO1_19, 	&LPC_IOCON->PIO1_25,	&LPC_IOCON->PIO1_16, 	&LPC_IOCON->PIO0_19,
		&LPC_IOCON->PIO0_18,	&LPC_IOCON->PIO0_17,	&LPC_IOCON->PIO1_15, 	&LPC_IOCON->PIO0_23, 		// 24
		&LPC_IOCON->PIO0_22,	&LPC_IOCON->PIO0_16,	&LPC_IOCON->SWDIO_PIO0_15,&LPC_IOCON->PIO1_22,
		&LPC_IOCON->PIO1_14, 	&LPC_IOCON->PIO1_13, 	&LPC_IOCON->TRST_PIO0_14,&LPC_IOCON->TDO_PIO0_13,	// 32
		&LPC_IOCON->TMS_PIO0_12,&LPC_IOCON->TDI_PIO0_11,// 34
};

#define LPC_PIN_FUNCTION_MASK	(BIT7 | 7)		// FUNC bits + AD bit
#define LPC_PIN_MODE_MASK		(3 << 3)

uint8_t const LPC_PIN_PRIMARY_FUNCTION[] = {	// GPIO function, All AD bits = 1
		0x80,	0x80,	0x80,	0x80,		0x80,	0x80,	0x80,	0x80,	// 8
		0x80,	0x80,	0x80,	0x80,		0x80,	0x80,	0x81,	0x80,	// 16
		0x80,	0x80,	0x80,	0x80,		0x80,	0x80,	0x80,	0x80,	// 24
		0x80,	0x80,	0x81,	0x80,		0x80,	0x80,	0x81,	0x81,	// 32
		0x81,	0x81,	// 34
};

uint8_t const LPC_PIN_SECONDARY_FUNCTION[] = {
		0x80 /* GPIO */,	0x80 /* GPIO */,		0x81 /* CT32B0 MAT2 */,	0x80 /* GPIO */,
		0x82 /* SPI1 SCK */,0x82 /* SPI1 MOSI */,	0x80 /* GPIO */,		0x81 /* CT32B0 MAT0 */,	// 8
		0x80 /* GPIO */,	0x80 /* GPIO */,		0x80 /* GPIO */,		0x82 /* SPI1 MISO */,
		0x81 /* SPI0 MISO*/,0x81 /* SPI0 MOSI */,	0x82 /* SPI0 SCK */,	0x80 /* GPIO */,	// 16
		0x80 /* GPIO */,	0x81 /* CT32B0 MAT1 */,	0x80 /* GPIO */,		0x81 /* UART TX */,
		0x81 /* UART RX */,	0x80 /* GPIO */,		0x82 /* PWM16_2 */,		0x01 /* ADC7 */,	// 24
		0x01 /* ADC6 */,	0x01 /* ADC5 */,		0x02 /* ADC4 */,		0x80 /* GPIO */,
		0x82 /* PWM16_1 */,	0x82 /* PWM16_0 */,		0x02 /* ADC3 */,		0x02 /* ADC2 */,	// 32
		0x02 /* ADC1 */,	0x02 /* ADC0 */,// 34
};

uint8_t const LPC_PORT_MAPPING[4][8] = {
		{20, 19, 13, 12, 14, 1,  8,  21}, // PORT 0
		{5,  11, 4,  0, 18, 16, 27, 6},  // PORT 1
		{3,  9,  29, 28, 22, 7,  17, 2},  // PORT 2
		{33, 32, 31, 30, 26, 25, 24, 23}  // PORT 3
};

static volatile SFPFunctionType LPC_INTERRUPT_FUNCTION_TYPE[LPC_INTERRUPT_COUNT];
static uint32_t LPC_INTERRUPT_DOWNTIME[LPC_INTERRUPT_COUNT];

void lpc_config_gpioInit() {
	uint8_t pin;
	for (pin=0; pin<LPC_PIN_COUNT; pin++)
		*LPC_PIN_REGISTERS[pin] = (*LPC_PIN_REGISTERS[pin] & ~LPC_PIN_FUNCTION_MASK) | LPC_PIN_PRIMARY_FUNCTION[pin];
}

SFPResult lpc_config_setPrimary(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1) return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT) return SFP_ERR_ARG_TYPE;

	uint8_t pin = SFPFunction_getArgument_int32(msg, 0);

	if (pin >= LPC_PIN_COUNT) return SFP_ERR_ARG_VALUE;

	*LPC_PIN_REGISTERS[pin] = (*LPC_PIN_REGISTERS[pin] & ~LPC_PIN_FUNCTION_MASK) | LPC_PIN_PRIMARY_FUNCTION[pin];

	return SFP_OK;
}

SFPResult lpc_config_setSecondary(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1) return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT) return SFP_ERR_ARG_TYPE;

	uint8_t pin = SFPFunction_getArgument_int32(msg, 0);

	if (pin >= LPC_PIN_COUNT) return SFP_ERR_ARG_VALUE;

	*LPC_PIN_REGISTERS[pin] = (*LPC_PIN_REGISTERS[pin] & ~LPC_PIN_FUNCTION_MASK) | LPC_PIN_SECONDARY_FUNCTION[pin];

	return SFP_OK;
}


SFPResult lpc_pinMode(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2) return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT || SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t pin = SFPFunction_getArgument_int32(msg, 0);
	uint8_t mode = SFPFunction_getArgument_int32(msg, 1);

	if (pin >= LPC_PIN_COUNT) return SFP_ERR_ARG_VALUE;
	if (mode > 4 || mode == 3) return SFP_ERR_ARG_VALUE;

	uint8_t port = 0;
	uint8_t pinNum = LPC_PIN_IDS[pin];
	if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
		port = 1;
		pinNum -= 24;
	}

	*LPC_PIN_REGISTERS[pin] &= ~LPC_PIN_MODE_MASK;	// Remove pull-up/down resistors

	if (mode == 1) {
		LPC_GPIO->DIR[port] |= (1 << pinNum);	// Set direction bit (output)
	} else {
		*LPC_PIN_REGISTERS[pin] |= (mode << 2) & LPC_PIN_MODE_MASK;		// Setup resistors
		LPC_GPIO->DIR[port] &= ~(1 << pinNum);	// Clear direction bit (input)
	}

	return SFP_OK;
}

SFPResult lpc_digitalWrite(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT || SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t pin = SFPFunction_getArgument_int32(msg, 0);
	uint8_t value = SFPFunction_getArgument_int32(msg, 1);

	if (pin >= LPC_PIN_COUNT) return SFP_ERR_ARG_VALUE;

	uint8_t port = 0;
	uint8_t pinNum = LPC_PIN_IDS[pin];
	if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
		port = 1;
		pinNum -= 24;
	}

	if (value == 0) {
		LPC_GPIO->CLR[port] = (1 << pinNum);
	} else {
		LPC_GPIO->SET[port] = (1 << pinNum);
	}

	return SFP_OK;
}

SFPResult lpc_digitalRead(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t pin = SFPFunction_getArgument_int32(msg, 0);

	if (pin >= LPC_PIN_COUNT)
		return SFP_ERR_ARG_VALUE;

	uint8_t port = 0;
	uint8_t pinNum = LPC_PIN_IDS[pin];
	if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
		port = 1;
		pinNum -= 24;
	}

	uint8_t val = 0;

	if (LPC_GPIO->PIN[port] & (1 << pinNum))
		val = 1;

	SFPFunction *outFunc = SFPFunction_new();

	if (outFunc == NULL) return SFP_ERR_ALLOC_FAILED;

	SFPFunction_setType(outFunc, SFPFunction_getType(msg));
	SFPFunction_setID(outFunc, UPER_FID_DIGITALREAD);
	SFPFunction_setName(outFunc, UPER_FNAME_DIGITALREAD);
	SFPFunction_addArgument_int32(outFunc, pin);
	SFPFunction_addArgument_int32(outFunc, val);
	SFPFunction_send(outFunc, &stream);
	SFPFunction_delete(outFunc);

	return SFP_OK;
}

SFPResult lpc_portMode(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2) return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT || SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t port = SFPFunction_getArgument_int32(msg, 0);
	uint8_t mode = SFPFunction_getArgument_int32(msg, 1);

	if (port >= LPC_PORT_COUNT) return SFP_ERR_ARG_VALUE;
	if (mode > 4 || mode == 3) return SFP_ERR_ARG_VALUE;

	uint8_t i = 0;
	uint8_t lpc_port = 0;
	uint8_t pin = 0;
	for (i = 0; i < 8; i++) {
		lpc_port = 0;
		pin = LPC_PORT_MAPPING[port][i];
		uint8_t pinNum = LPC_PIN_IDS[pin];
		if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
			lpc_port = 1;
			pinNum -= 24;
		}

		*LPC_PIN_REGISTERS[pin] &= ~LPC_PIN_MODE_MASK;	// Remove pull-up/down resistors

		if (mode == 1) {
			LPC_GPIO->DIR[lpc_port] |= (1 << pinNum);	// Set direction bit (output)
		} else {
			*LPC_PIN_REGISTERS[pin] |= (mode << 2) & LPC_PIN_MODE_MASK;// Setup resistors
			LPC_GPIO->DIR[lpc_port] &= ~(1 << pinNum);	// Clear direction bit (input)
		}
	}
	return SFP_OK;
}

SFPResult lpc_portWrite(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t port = SFPFunction_getArgument_int32(msg, 0);
	uint8_t value = SFPFunction_getArgument_int32(msg, 1);

	if (port >= LPC_PORT_COUNT) return SFP_ERR_ARG_VALUE;

	uint8_t lpc_port = 0;
	uint8_t i;
	/* NOTE : The port mapping of WeIO doesn't reflect the real port of LPC.
	 * The portWrite doesn't set the whole port in the same time, but pin per pin.
	 */
	for (i = 0; i < 8; i++) {
		lpc_port = 0;
		// Get the current pin
		uint8_t pinNum = LPC_PIN_IDS[LPC_PORT_MAPPING[port][i]];
		if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
			lpc_port = 1;
			pinNum -= 24;
		}

		// Output the value
		if (((value >> i) & 0x01) == 0) {
			LPC_GPIO->CLR[lpc_port] = (1 << pinNum);
		} else {
			LPC_GPIO->SET[lpc_port] = (1 << pinNum);
		}
	}
	return SFP_OK;
}

SFPResult lpc_portRead(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t port = SFPFunction_getArgument_int32(msg, 0);

	if (port >= LPC_PORT_COUNT)
		return SFP_ERR_ARG_VALUE;

	uint8_t lpc_port = 0;
	uint8_t i;
	uint8_t port_val = 0;

	for (i = 0; i < 8; i++) {
		lpc_port = 0;
		// Get the current pin
		uint8_t pinNum = LPC_PIN_IDS[LPC_PORT_MAPPING[port][i]];
		if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
			lpc_port = 1;
			pinNum -= 24;
		}
		if (LPC_GPIO->PIN[lpc_port] & (1 << pinNum))
			port_val |= (1 << i);
	}

	SFPFunction *outFunc = SFPFunction_new();

	if (outFunc == NULL) return SFP_ERR_ALLOC_FAILED;

	SFPFunction_setType(outFunc, SFPFunction_getType(msg));
	SFPFunction_setID(outFunc, UPER_FID_PORTREAD);
	SFPFunction_setName(outFunc, UPER_FNAME_PORTREAD);
	SFPFunction_addArgument_int32(outFunc, port);
	SFPFunction_addArgument_int32(outFunc, port_val);
	SFPFunction_send(outFunc, &stream);
	SFPFunction_delete(outFunc);

	return SFP_OK;
}

SFPResult lpc_dhtxxRead(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1) return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t dht_data = SFPFunction_getArgument_int32(msg, 0);

	if (dht_data >= LPC_PIN_COUNT) return SFP_ERR_ARG_VALUE;

	uint8_t port = 0;
	uint8_t pinNum = LPC_PIN_IDS[dht_data];
	if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
		port = 1;
		pinNum -= 24;
	}
	uint8_t data[5] = {0, 0, 0, 0, 0}; // 40 bits
		
    /* Remove all pullup and pulldown */
    *LPC_PIN_REGISTERS[dht_data] &= ~LPC_PIN_MODE_MASK;

    /* Begin transaction with DHT11 : Set pin as output, HIGH */
    LPC_GPIO->DIR[port] |= (1 << pinNum);
    LPC_GPIO->SET[port] = (1 << pinNum);

    uint32_t startTimeUs = Time_getSystemTime_us();
    uint32_t passedTimeUs = 0;

    /* -- Start condition --
     * ¯¯¯¯¯¯¯¯\________/¯¯¯¯¯¯¯¯
     * | ~20ms | ~20ms  | ~40us |
     */
    /* Pull high for ~25ms */
    while ((Time_getSystemTime_us()-startTimeUs) <= 25000);
    LPC_GPIO->CLR[port] = (1 << pinNum);
    startTimeUs = Time_getSystemTime_us();
    /* Pull low for ~25ms */
    while ((Time_getSystemTime_us()-startTimeUs) <= 25000);
    /* Pull high for ~40us */
    LPC_GPIO->SET[port] = (1 << pinNum);
    startTimeUs = Time_getSystemTime_us();
    while ((Time_getSystemTime_us()-startTimeUs) <= 30);

    /* -- Response from sensor --
     * \_______/¯¯¯¯¯¯\______/¯¯¯¯¯¯\______/¯¯¯¯¯¯¯¯¯¯¯¯\___...
     * | 80us  | 80us | 50us | 27us | 50us |  70us      | .....
     * | start burst  | Answer : 0  | Answer : 1        | .....
     */
    /* Input, pullup */
    LPC_GPIO->CLR[port] = (1 << pinNum);
    *LPC_PIN_REGISTERS[dht_data] |= (1 << 2) & LPC_PIN_MODE_MASK;
    LPC_GPIO->DIR[port] &= ~(1 << pinNum);

    uint8_t i;
    uint8_t j = 0;
    /* Store the last pin state. */

    uint32_t cnt = 0;
    uint32_t cnt_compare = 0;

    startTimeUs = Time_getSystemTime_us();
    passedTimeUs = 0;
    uint8_t error = 0;
    while (!(LPC_GPIO->PIN[port] & (1 << pinNum))) {
    	if ((passedTimeUs=Time_getSystemTime_us()-startTimeUs) >= 1000) {
    		error = 1;
    		break;
    	}
    }

    startTimeUs = Time_getSystemTime_us();
    passedTimeUs = 0;
    while ((LPC_GPIO->PIN[port] & (1 << pinNum))) {
    	if ((passedTimeUs=Time_getSystemTime_us()-startTimeUs) >= 1000) {
    		error = 1;
    		break;
    	}
    }
    if (!error) {
		for (i = 0; i < 40; i++) {
			cnt = 0;
			cnt_compare = 0;
			while (!(LPC_GPIO->PIN[port] & (1 << pinNum)))
				cnt++;
			while ((LPC_GPIO->PIN[port] & (1 << pinNum)))
				cnt_compare++;

			data[j/8] <<= 1;
			if (cnt < cnt_compare)
				data[j/8] |= 1;
			j++;
		}
    }

	SFPFunction *outFunc = SFPFunction_new();

	if (outFunc == NULL) return SFP_ERR_ALLOC_FAILED;

	SFPFunction_setType(outFunc, SFPFunction_getType(msg));
	SFPFunction_setID(outFunc, UPER_FID_DHTXXREAD);
	SFPFunction_setName(outFunc, UPER_FNAME_DHTXXREAD);
	SFPFunction_addArgument_int32(outFunc, dht_data);
	SFPFunction_addArgument_int32(outFunc, data[0]);
	SFPFunction_addArgument_int32(outFunc, data[1]);
	SFPFunction_addArgument_int32(outFunc, data[2]);
	SFPFunction_addArgument_int32(outFunc, data[3]);
	SFPFunction_addArgument_int32(outFunc, data[4]);
	SFPFunction_send(outFunc, &stream);
	SFPFunction_delete(outFunc);

	return SFP_OK;
}

SFPResult lpc_pulseIn(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 3) return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT
			|| SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT
			|| SFPFunction_getArgumentType(msg, 2) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t pin = SFPFunction_getArgument_int32(msg, 0);
	uint8_t levelMask = (SFPFunction_getArgument_int32(msg, 1) == 0 ? 0 : 1);
	uint32_t timeout = SFPFunction_getArgument_int32(msg, 2);

	if (pin >= LPC_PIN_COUNT) return SFP_ERR_ARG_VALUE;

	uint8_t port = 0;
	uint8_t pinNum = LPC_PIN_IDS[pin];
	if (pinNum > 23) {	// if not PIO0_0 to PIO0_23
		port = 1;
		pinNum -= 24;
	}

	levelMask <<= pinNum; // shift BIT0 to pin place

	uint32_t startTimeUs = Time_getSystemTime_us();
	uint32_t passedTimeUs = 0;

	while ((LPC_GPIO->PIN[port] & (1 << pinNum)) == levelMask) {	// Wait while signal is on
		if ((passedTimeUs=Time_getSystemTime_us()-startTimeUs) >= timeout)
			break;
	}

	while ((LPC_GPIO->PIN[port] & (1 << pinNum)) != levelMask) { // Wait while signal is off
		if ((passedTimeUs=Time_getSystemTime_us()-startTimeUs) >= timeout)
			break;
	}

	uint32_t signalStartTime = Time_getSystemTime_us();

	while ((LPC_GPIO->PIN[port] & (1 << pinNum)) == levelMask) {	// Wait while signal is on
		if ((passedTimeUs=Time_getSystemTime_us()-startTimeUs) >= timeout)
			break;
	}
	uint32_t signalDuration = Time_getSystemTime_us()-signalStartTime;

	SFPFunction *outFunc = SFPFunction_new();

	if (outFunc == NULL) return SFP_ERR_ALLOC_FAILED;

	SFPFunction_setType(outFunc, SFPFunction_getType(msg));
	SFPFunction_setID(outFunc, UPER_FID_PULSEIN);
	SFPFunction_setName(outFunc, UPER_FNAME_PULSEIN);
	SFPFunction_addArgument_int32(outFunc, (passedTimeUs < timeout ? signalDuration : 0));
	SFPFunction_send(outFunc, &stream);
	SFPFunction_delete(outFunc);

	return SFP_OK;
}

SFPResult lpc_attachInterrupt(SFPFunction *func) {
	if (SFPFunction_getArgumentCount(func) != 4)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(func, 0) != SFP_ARG_INT
			|| SFPFunction_getArgumentType(func, 1) != SFP_ARG_INT
			|| SFPFunction_getArgumentType(func, 2) != SFP_ARG_INT
			|| SFPFunction_getArgumentType(func, 3) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint8_t p_intID = SFPFunction_getArgument_int32(func, 0);	// interrupt ID
	uint8_t p_pin = SFPFunction_getArgument_int32(func, 1);	// pin ID
	uint8_t p_mode = SFPFunction_getArgument_int32(func, 2);	// interrupt mode
	uint32_t p_downtime = SFPFunction_getArgument_int32(func, 3); // down time

	if (p_pin >= LPC_PIN_COUNT || p_intID >= LPC_INTERRUPT_COUNT || p_mode > 4) return SFP_ERR_ARG_VALUE;

	NVIC_DisableIRQ(p_intID);	// Disable interrupt. XXX: Luckily FLEX_INTx_IRQn == x, so it can be used this way, otherwise BE AWARE!

	LPC_SYSCON->PINTSEL[p_intID] = LPC_PIN_IDS[p_pin]; 	// select which pin will cause the interrupts

	// XXX: using SI/CI ENF and ENR registers could probably save few instructions
	switch (p_mode) {
		case 0: {	// LOW level mode
			LPC_GPIO_PIN_INT ->ISEL |= (1 << p_intID);	// Set PMODE=level sensitive
			LPC_GPIO_PIN_INT ->IENR |= (1 << p_intID);	// Enable level interrupt.
			LPC_GPIO_PIN_INT ->IENF &= ~(1 << p_intID);	// Set active level LOW.
			break;
		}
		case 1: {	// HIGH level mode
			LPC_GPIO_PIN_INT ->ISEL |= (1 << p_intID);	// Set PMODE=level sensitive
			LPC_GPIO_PIN_INT ->IENR |= (1 << p_intID);	// Enable level interrupt.
			LPC_GPIO_PIN_INT ->IENF |= (1 << p_intID);	// Set active level HIGH.
			break;
		}
		case 2: {	// Edge CHANGE mode
			LPC_GPIO_PIN_INT ->ISEL &= ~(1 << p_intID);	// Set PMODE=edge sensitive
			LPC_GPIO_PIN_INT ->IENR |= (1 << p_intID);	// Enable rising edge.
			LPC_GPIO_PIN_INT ->IENF |= (1 << p_intID);	// Enable falling edge.
			break;
		}
		case 3: {	// RISING edge mode
			LPC_GPIO_PIN_INT ->ISEL &= ~(1 << p_intID);	// Set PMODE=edge sensitive
			LPC_GPIO_PIN_INT ->IENR |= (1 << p_intID);	// Enable rising edge.
			LPC_GPIO_PIN_INT ->IENF &= ~(1 << p_intID);	// Disable falling edge.
			break;
		}
		case 4: {	// FALLING edge mode
			LPC_GPIO_PIN_INT ->ISEL &= ~(1 << p_intID);	// Set PMODE=edge sensitive
			LPC_GPIO_PIN_INT ->IENR &= ~(1 << p_intID);	// Disable rising edge.
			LPC_GPIO_PIN_INT ->IENF |= (1 << p_intID);	// Enable falling edge.
			break;
		}
	}

	LPC_INTERRUPT_FUNCTION_TYPE[p_intID] = SFPFunction_getType(func);
	LPC_INTERRUPT_DOWNTIME[p_intID] = p_downtime;

	LPC_GPIO_PIN_INT->RISE = (1 << p_intID);	// Clear rising edge (sort of) flag
	LPC_GPIO_PIN_INT->FALL = (1 << p_intID);	// Clear falling edge (sort of) flag
	NVIC_SetPriority(p_intID, 3); // set lowest priority
	NVIC_EnableIRQ(p_intID);	// Enable interrupt. XXX: Luckily FLEX_INTx_IRQn == x, so it can be used this way, otherwise BE AWARE!

	return SFP_OK;
}

SFPResult lpc_detachInterrupt(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT) return SFP_ERR_ARG_TYPE;

	uint8_t p_intID = SFPFunction_getArgument_int32(msg, 0);	// interrupt ID

	NVIC_DisableIRQ(p_intID);	// Disable interrupt. XXX: Luckily FLEX_INTx_IRQn == x, so it can be used this way, otherwise BE AWARE!
	LPC_GPIO_PIN_INT->CIENR = (1 << p_intID);	// Disable rising edge or level interrupt
	LPC_GPIO_PIN_INT->CIENF = (1 << p_intID);	// Disable falling edge interrupt
	LPC_GPIO_PIN_INT->RISE = (1 << p_intID);	// Clear rising edge (sort of) flag
	LPC_GPIO_PIN_INT->FALL = (1 << p_intID);	// Clear falling edge (sort of) flag

	return SFP_OK;
}

void GPIO_EnableInterruptCallback(void* ptr) {
	GPIO_EnableInterrupt((uint8_t)(uint32_t)ptr);
}

void GPIO_EnableInterrupt(uint8_t intID) {
	LPC_GPIO_PIN_INT->RISE = (1<<intID);	// Clear rising edge (sort of) flag
	LPC_GPIO_PIN_INT->FALL = (1<<intID);	// Clear falling edge (sort of) flag
	NVIC_EnableIRQ(intID);	// Enable ISR
}

static inline void GPIO_SEND_INT(uint8_t intID, uint32_t intStatus) {
	SFPFunction *func = SFPFunction_new();
	if (func != NULL) {
		SFPFunction_setType(func, LPC_INTERRUPT_FUNCTION_TYPE[intID]);
		SFPFunction_setID(func, UPER_FID_INTERRUPT);
		SFPFunction_setName(func, UPER_FNAME_INTERRUPT);
		SFPFunction_addArgument_int32(func, intID);
		SFPFunction_addArgument_int32(func, intStatus);
		SFPFunction_send(func, &stream);
		SFPFunction_delete(func);
	}
}

static void GPIO_InterruptHandler(uint8_t intID) {
	NVIC_DisableIRQ(intID);		// Disable ISR

	uint8_t intBit = (1 << intID);

	if (LPC_GPIO_PIN_INT->IST & intBit) {

		uint32_t interruptValues = 0;
		uint8_t i;
		for (i=0; i<LPC_INTERRUPT_COUNT; i++) {
			uint8_t port = 0;
			uint8_t pinNum = LPC_SYSCON->PINTSEL[i];
			if (pinNum > 23) {
				port = 1;
				pinNum -= 24;
			}

			if (LPC_GPIO->PIN[port] & (1 << pinNum))
				interruptValues |= (1 << i);
		}


		uint8_t interruptEvent = 0xFF;

		if ((LPC_GPIO_PIN_INT->ISEL & intBit)) {	// if LEVEL mode
			if (LPC_GPIO_PIN_INT->IENR & intBit) {	// if LEVEL interrupts are enabled
				if (LPC_GPIO_PIN_INT->IENF & intBit) {	// HIGH mode
					interruptEvent = 1;
				} else {								// LOW mode
					interruptEvent = 0;
				}
			}
		} else {	// EDGE mode
			if ((LPC_GPIO_PIN_INT->RISE & intBit) && (LPC_GPIO_PIN_INT->IENR & intBit)) {	// Rising edge interrupt
				interruptEvent = 3;
			}
			if ((LPC_GPIO_PIN_INT->FALL & intBit) && (LPC_GPIO_PIN_INT->IENF & intBit)) {	// Falling edge interrupt
				if (interruptEvent == 3)
					interruptEvent = 2;				// Edge CHANGE (RISE+FALL)
				else
					interruptEvent = 4;				// Falling edge
			}
		}

		GPIO_SEND_INT(intID, (interruptValues << 8) | interruptEvent);

		Time_addTimer(LPC_INTERRUPT_DOWNTIME[intID], GPIO_EnableInterruptCallback, (void*)(uint32_t)intID);
		return;
	}

	GPIO_EnableInterrupt(intID);
}

void FLEX_INT0_IRQHandler() {
	GPIO_InterruptHandler(0);
}

void FLEX_INT1_IRQHandler() {
	GPIO_InterruptHandler(1);
}

void FLEX_INT2_IRQHandler() {
	GPIO_InterruptHandler(2);
}

void FLEX_INT3_IRQHandler() {
	GPIO_InterruptHandler(3);
}

void FLEX_INT4_IRQHandler() {
	GPIO_InterruptHandler(4);
}

void FLEX_INT5_IRQHandler() {
	GPIO_InterruptHandler(5);
}

void FLEX_INT6_IRQHandler() {
	GPIO_InterruptHandler(6);
}

void FLEX_INT7_IRQHandler() {
	GPIO_InterruptHandler(7);
}
