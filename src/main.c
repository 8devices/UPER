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

SFPResult lpc_system_getDeviceInfo(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 0) return SFP_ERR_ARG_COUNT;

	SFPFunction *func = SFPFunction_new();

	if (func == NULL) return SFP_ERR_ALLOC_FAILED;

	SFPFunction_setType(func, SFPFunction_getType(msg));
	SFPFunction_setID(func, UPER_FID_GETDEVICEINFO);
	SFPFunction_setName(func, UPER_FNAME_GETDEVICEINFO);
	SFPFunction_addArgument_int32(func, UPER_FIRMWARE_VERSION);
	SFPFunction_addArgument_barray(func, (uint8_t*)&GUID[0], 16);
	SFPFunction_addArgument_int32(func, UPER_PART_NUMBER);
	SFPFunction_addArgument_int32(func, UPER_BOOT_CODE_VERSION);
	SFPFunction_send(func, &stream);
	SFPFunction_delete(func);

	return SFP_OK;
}

SFPResult lpc_system_restart(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 0) return SFP_ERR_ARG_COUNT;

	Time_delay(1000); // Delay 1s before restarting
	NVIC_SystemReset();

	return SFP_OK; // This code should not be reached
}

int main(void) {
	SystemCoreClockUpdate();

	// Read IAP before any interrupts are enabled
	UPER_PART_NUMBER		= IAP_GetPartNumber();
	UPER_BOOT_CODE_VERSION	= IAP_GetBootCodeVersion();
	IAP_GetSerialNumber(GUID);

	// Init the rest of the system
	Time_init();

	while (CDC_Init(&stream, GUID) != LPC_OK); // Load SFPPacketStream

	LPC_SYSCON->SYSAHBCLKCTRL |= BIT6 | BIT16 | BIT19; // Enable clock for GPIO, IOConfig and Pin Interrupts

#ifndef DEBUG
	// Disabled for debugging (JTAG)
	lpc_config_gpioInit();
#endif

	// PIO0_4 and PIO0_5 forced to I2C
	LPC_IOCON->PIO0_4 |= 1;	// I2C SCL
	LPC_IOCON->PIO0_5 |= 1;	// I2C SDA

	/* SFP initialization, configuration and launch */
	SFPServer *server = SFPServer_new(&stream);

	SFPServer_setDataTimeout(server, 30000); // 300k is about a second (30k ~100ms)

	/* GPIO/Pin functions */
	SFPServer_addFunctionHandler(server, UPER_FNAME_SETPRIMARY,	 UPER_FID_SETPRIMARY,	lpc_config_setPrimary);
	SFPServer_addFunctionHandler(server, UPER_FNAME_SETSECONDARY, UPER_FID_SETSECONDARY,	lpc_config_setSecondary);

	SFPServer_addFunctionHandler(server, UPER_FNAME_PINMODE,      UPER_FID_PINMODE,		lpc_pinMode);
	SFPServer_addFunctionHandler(server, UPER_FNAME_DIGITALWRITE, UPER_FID_DIGITALWRITE,	lpc_digitalWrite);
	SFPServer_addFunctionHandler(server, UPER_FNAME_DIGITALREAD,  UPER_FID_DIGITALREAD,	lpc_digitalRead);

	SFPServer_addFunctionHandler(server, UPER_FNAME_ATTACHINTERRUPT, UPER_FID_ATTACHINTERRUPT, lpc_attachInterrupt);
	SFPServer_addFunctionHandler(server, UPER_FNAME_DETACHINTERRUPT, UPER_FID_DETACHINTERRUPT, lpc_detachInterrupt);

	SFPServer_addFunctionHandler(server, UPER_FNAME_PULSEIN, UPER_FID_PULSEIN, lpc_pulseIn);

	SFPServer_addFunctionHandler(server, UPER_FNAME_PORTMODE, UPER_FID_PORTMODE, lpc_portMode);
	SFPServer_addFunctionHandler(server, UPER_FNAME_PORTWRITE, UPER_FID_PORTWRITE, lpc_portWrite);
	SFPServer_addFunctionHandler(server, UPER_FNAME_PORTREAD, UPER_FID_PORTREAD, lpc_portRead);

	/* ADC functions */
	SFPServer_addFunctionHandler(server, UPER_FNAME_ANALOGREAD, UPER_FID_ANALOGREAD, lpc_analogRead);

	/* SPI functions */
	SFPServer_addFunctionHandler(server, UPER_FNAME_SPI0BEGIN, UPER_FID_SPI0BEGIN, lpc_spi0_begin);
	SFPServer_addFunctionHandler(server, UPER_FNAME_SPI0TRANS, UPER_FID_SPI0TRANS, lpc_spi0_trans);
	SFPServer_addFunctionHandler(server, UPER_FNAME_SPI0END,   UPER_FID_SPI0END, lpc_spi0_end);

	SFPServer_addFunctionHandler(server, UPER_FNAME_SPI1BEGIN, UPER_FID_SPI1BEGIN, lpc_spi1_begin);
	SFPServer_addFunctionHandler(server, UPER_FNAME_SPI1TRANS, UPER_FID_SPI1TRANS, lpc_spi1_trans);
	SFPServer_addFunctionHandler(server, UPER_FNAME_SPI1END,   UPER_FID_SPI1END, lpc_spi1_end);

	/* I2C functions */
	SFPServer_addFunctionHandler(server, UPER_FNAME_I2CBEGIN, UPER_FID_I2CBEGIN, lpc_i2c_begin);
	SFPServer_addFunctionHandler(server, UPER_FNAME_I2CTRANS, UPER_FID_I2CTRANS, lpc_i2c_trans);
	SFPServer_addFunctionHandler(server, UPER_FNAME_I2CEND,   UPER_FID_I2CEND, lpc_i2c_end);

	/* PWM functions */
	SFPServer_addFunctionHandler(server, UPER_FNAME_PWM0BEGIN, UPER_FID_PWM0BEGIN, lpc_pwm0_begin);
	SFPServer_addFunctionHandler(server, UPER_FNAME_PWM0SET,   UPER_FID_PWM0SET, lpc_pwm0_set);
	SFPServer_addFunctionHandler(server, UPER_FNAME_PWM0END,   UPER_FID_PWM0END, lpc_pwm0_end);

	SFPServer_addFunctionHandler(server, UPER_FNAME_PWM1BEGIN, UPER_FID_PWM1BEGIN, lpc_pwm1_begin);
	SFPServer_addFunctionHandler(server, UPER_FNAME_PWM1SET,   UPER_FID_PWM1SET, lpc_pwm1_set);
	SFPServer_addFunctionHandler(server, UPER_FNAME_PWM1END,   UPER_FID_PWM1END, lpc_pwm1_end);

	/* Special sensors functions */
	SFPServer_addFunctionHandler(server, UPER_FNAME_DHTXXREAD, UPER_FID_DHTXXREAD, lpc_dhtxxRead);

	/* Other functions */
	SFPServer_addFunctionHandler(server, UPER_FNAME_RESTART,       UPER_FID_RESTART, lpc_system_restart);
	SFPServer_addFunctionHandler(server, UPER_FNAME_GETDEVICEINFO,  UPER_FID_GETDEVICEINFO, lpc_system_getDeviceInfo);


	SFPServer_loop(server);

	SFPServer_delete(server);

	while (1);
}
