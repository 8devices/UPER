/**
 * @file	main.c
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

#include "main.h"

#include "CDC/CDC.h"

#include "Modules/LPC_GPIO.h"
#include "Modules/LPC_ADC.h"
#include "Modules/LPC_SPI.h"
#include "Modules/LPC_I2C.h"
#include "Modules/LPC_PWM.h"

#include "IAP.h"

void SysTick_Handler() {
	uint8_t i;
	for (i=0; i<INTERRUPT_COUNT; i++) {
		if (timer_interrupts[i] != TIMER_STOP) {
			timer_interrupts[i]--;
			if (timer_interrupts[i] == 0) {
				GPIO_EnableInterrupt(i);
			}
		}
	}
}

inline uint8_t isValidRegisterAddress(uint32_t addr) {

	if (addr >= 0x40000000 && addr < 0x40020000) return 1;	// I2C, WWDT, USART, timers, ADC

	if (addr >= 0x40038000 && addr < 0x40050000) return 1;	// PMU, flash/EEPROM, SSP0, IOCON, system control, GPIO interrupts

	if (addr >= 0x40058000 && addr < 0x40064000) return 1;	// SSP1, GPIO Group interrupts

	if (addr >= 0x40080000 && addr < 0x40084000) return 1;	// USB

	if (addr >= 0x50000000 && addr < 0x50004000) return 1;	// GPIO

	return 0;
}

SFPResult lpc_system_registerWrite(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2) return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT
			|| SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint32_t p_addr = SFPFunction_getArgument_int32(msg, 0);
	uint32_t p_value = SFPFunction_getArgument_int32(msg, 1);

	if (!isValidRegisterAddress(p_addr)) return SFP_ERR_ARG_VALUE;

	*((volatile uint32_t *)p_addr) = p_value;

	return SFP_OK;
}

SFPResult lpc_system_registerRead(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 1) return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT) return SFP_ERR_ARG_TYPE;

	uint32_t p_addr = SFPFunction_getArgument_int32(msg, 0);

	if (!isValidRegisterAddress(p_addr)) return SFP_ERR_ARG_VALUE;

	uint32_t value = *((volatile uint32_t *)p_addr);

	SFPFunction *func = SFPFunction_new();

	if (func == NULL) return SFP_ERR_ALLOC_FAILED;

	SFPFunction_setType(func, SFPFunction_getType(msg));
	SFPFunction_setID(func, UPER_FUNCTION_ID_OUT_REGISTERREAD);
	SFPFunction_setName(func, UPER_FUNCTION_NAME_OUT_REGISTERREAD);
	SFPFunction_addArgument_int32(func, p_addr);
	SFPFunction_addArgument_int32(func, value);
	SFPFunction_send(func, &stream);
	SFPFunction_delete(func);

	return SFP_OK;
}

SFPResult lpc_system_getDeviceInfo(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 0) return SFP_ERR_ARG_COUNT;

	SFPFunction *func = SFPFunction_new();

	if (func == NULL) return SFP_ERR_ALLOC_FAILED;

	SFPFunction_setType(func, SFPFunction_getType(msg));
	SFPFunction_setID(func, UPER_FUNCTION_ID_OUT_GETDEVICEINFO);
	SFPFunction_setName(func, UPER_FUNCTION_NAME_OUT_GETDEVICEINFO);
	SFPFunction_addArgument_int32(func, UPER_FIRMWARE_VERSION);
	SFPFunction_addArgument_barray(func, (uint8_t*)&GUID[0], 16);
	SFPFunction_addArgument_int32(func, IAP_GetPartNumber());
	SFPFunction_addArgument_int32(func, IAP_GetBootCodeVersion());
	SFPFunction_send(func, &stream);
	SFPFunction_delete(func);

	return SFP_OK;
}

SFPResult LedCallback(SFPFunction *msg) {
	LPC_GPIO->NOT[0] |= BIT7;

	SFPFunction *outFunc = SFPFunction_new();
	if (outFunc != NULL) {
		SFPFunction_setType(outFunc, SFPFunction_getType(msg));
		SFPFunction_setID(outFunc, 0xE1);
		SFPFunction_setName(outFunc, "testukas");
		SFPFunction_addArgument_int32(outFunc, (int32_t)outFunc);

		uint32_t i;
		for (i=0; i<SFPFunction_getArgumentCount(msg); i++) {
			SFPArgumentType t = SFPFunction_getArgumentType(msg, i);
			if (t == SFP_ARG_INT) {
				SFPFunction_addArgument_int32(outFunc, SFPFunction_getArgument_int32(msg, i));
			} else if (t == SFP_ARG_BYTE_ARRAY) {
				uint32_t len = 0;
				uint8_t *data = SFPFunction_getArgument_barray(msg, i, &len);
				SFPFunction_addArgument_barray(outFunc, data, len);
			}
		}

		SFPFunction_send(outFunc, &stream);
		SFPFunction_delete(outFunc);
	}

	return SFP_OK;
}

int main(void) {
	SystemCoreClockUpdate();

	uint8_t i;
	for (i=0; i<INTERRUPT_COUNT; i++)
		timer_interrupts[i] = TIMER_STOP;

	SysTick_Config(SystemCoreClock/1000);	// Configure Systick to run at 1kHz (1ms)

	IAP_GetSerialNumber(GUID);

	while (CDC_Init(&stream, GUID) != LPC_OK); // Load SFPPacketStream

	LPC_SYSCON->SYSAHBCLKCTRL |= BIT6 | BIT16 | BIT19; // Enable clock for GPIO, IOConfig and Pin Interrupts

#ifndef DEBUG
	// Disabled for debugging (JTAG)
	lpc_config_gpioInit();
#endif

	// PIO0_4 and PIO0_5 forced to I2C
	LPC_IOCON->PIO0_4 |= 1;	// I2C SCL
	LPC_IOCON->PIO0_5 |= 1;	// I2C SDA

	/* Temporary and test configs */
	// XXX: LED config - leave it for now
	LPC_GPIO->DIR[0] |= BIT7;
	LPC_GPIO->CLR[0] |= BIT7;


	/* SFP initialization, configuration and launch */
	SFPServer *server = SFPServer_new(&stream);

	/* Test functions (temporary) */
	SFPServer_addFunctionHandler(server, "led", 0, LedCallback);

	/* GPIO/Pin functions */
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_SETPRIMARY,	 UPER_FUNCTION_ID_IN_SETPRIMARY,	lpc_config_setPrimary);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_SETSECONDARY, UPER_FUNCTION_ID_IN_SETSECONDARY,	lpc_config_setSecondary);

	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_PINMODE,      UPER_FUNCTION_ID_IN_PINMODE,		lpc_pinMode);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_DIGITALWRITE, UPER_FUNCTION_ID_IN_DIGITALWRITE,	lpc_digitalWrite);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_DIGITALREAD,  UPER_FUNCTION_ID_IN_DIGITALREAD,	lpc_digitalRead);

	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_ATTACHINTERRUPT, UPER_FUNCTION_ID_IN_ATTACHINTERRUPT, lpc_attachInterrupt);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_DETACHINTERRUPT, UPER_FUNCTION_ID_IN_DETACHINTERRUPT, lpc_detachInterrupt);

	/* ADC functions */
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_ANALOGREAD, UPER_FUNCTION_ID_IN_ANALOGREAD, lpc_analogRead);

	/* SPI functions */
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_SPI0BEGIN, UPER_FUNCTION_ID_IN_SPI0BEGIN, lpc_spi0_begin);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_SPI0TRANS, UPER_FUNCTION_ID_IN_SPI0TRANS, lpc_spi0_trans);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_SPI0END,   UPER_FUNCTION_ID_IN_SPI0END, lpc_spi0_end);

	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_SPI1BEGIN, UPER_FUNCTION_ID_IN_SPI1BEGIN, lpc_spi1_begin);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_SPI1TRANS, UPER_FUNCTION_ID_IN_SPI1TRANS, lpc_spi1_trans);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_SPI1END,   UPER_FUNCTION_ID_IN_SPI1END, lpc_spi1_end);

	/* I2C functions */
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_I2CBEGIN, UPER_FUNCTION_ID_IN_I2CBEGIN, lpc_i2c_begin);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_I2CTRANS, UPER_FUNCTION_ID_IN_I2CTRANS, lpc_i2c_trans);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_I2CEND,   UPER_FUNCTION_ID_IN_I2CEND, lpc_i2c_end);

	/* PWM functions */
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_PWM0BEGIN, UPER_FUNCTION_ID_IN_PWM0BEGIN, lpc_pwm0_begin);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_PWM0SET,   UPER_FUNCTION_ID_IN_PWM0SET, lpc_pwm0_set);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_PWM0END,   UPER_FUNCTION_ID_IN_PWM0END, lpc_pwm0_end);

	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_PWM1BEGIN, UPER_FUNCTION_ID_IN_PWM1BEGIN, lpc_pwm1_begin);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_PWM1SET,   UPER_FUNCTION_ID_IN_PWM1SET, lpc_pwm1_set);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_PWM1END,   UPER_FUNCTION_ID_IN_PWM1END, lpc_pwm1_end);


	/* Other functions */
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_REGISTERWRITE, UPER_FUNCTION_ID_IN_REGISTERWRITE, lpc_system_registerWrite);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_REGISTERREAD,  UPER_FUNCTION_ID_IN_REGISTERREAD, lpc_system_registerRead);
	SFPServer_addFunctionHandler(server, UPER_FUNCTION_NAME_IN_GETDEVICEINFO,  UPER_FUNCTION_ID_IN_GETDEVICEINFO, lpc_system_getDeviceInfo);


	SFPServer_loop(server);

	SFPServer_delete(server);

	while (1);
}
