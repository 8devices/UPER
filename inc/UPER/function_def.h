/**
 * @file	function_def.h
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
 * UPER board function name and ID defintions
 *
 */

#ifndef FUNCTION_DEF_H_
#define FUNCTION_DEF_H_

/*
 * Functions IDs
 */
#define UPER_FID_SETPRIMARY			1
#define UPER_FID_SETSECONDARY		2
#define UPER_FID_PINMODE			3
#define UPER_FID_DIGITALWRITE		4
#define UPER_FID_DIGITALREAD		5
#define UPER_FID_ATTACHINTERRUPT	6
#define UPER_FID_DETACHINTERRUPT	7
#define UPER_FID_INTERRUPT			8
#define UPER_FID_PULSEIN			9

#define UPER_FID_ANALOGREAD			10

#define UPER_FID_SPI0BEGIN			20
#define UPER_FID_SPI0TRANS			21
#define UPER_FID_SPI0END			22

#define UPER_FID_SPI1BEGIN			30
#define UPER_FID_SPI1TRANS			31
#define UPER_FID_SPI1END			32

#define UPER_FID_I2CBEGIN			40
#define UPER_FID_I2CTRANS			41
#define UPER_FID_I2CEND				42

#define UPER_FID_PWM0BEGIN			50
#define UPER_FID_PWM0SET			51
#define UPER_FID_PWM0END			52

#define UPER_FID_PWM1BEGIN			60
#define UPER_FID_PWM1SET			61
#define UPER_FID_PWM1END			62

#define UPER_FID_PORTWRITE			70
#define UPER_FID_PORTREAD			71
#define UPER_FID_PORTMODE			72

#define UPER_FID_DHTXXREAD			200

#define UPER_FID_RESTART			251

#define UPER_FID_GETDEVICEINFO		255

/*
 * Function names
 */

#define UPER_FNAME_SETPRIMARY		"setPrimary"
#define UPER_FNAME_SETSECONDARY		"setSecondary"
#define UPER_FNAME_PINMODE			"pinMode"
#define UPER_FNAME_DIGITALWRITE		"digitalWrite"
#define UPER_FNAME_DIGITALREAD		"digitalRead"
#define UPER_FNAME_ATTACHINTERRUPT	"attachInterrupt"
#define UPER_FNAME_DETACHINTERRUPT	"detachInterrupt"
#define UPER_FNAME_INTERRUPT		"interrupt"
#define UPER_FNAME_PULSEIN			"pulseIn"

#define UPER_FNAME_ANALOGREAD		"analogRead"

#define UPER_FNAME_SPI0BEGIN		"spi0_begin"
#define UPER_FNAME_SPI0TRANS		"spi0_trans"
#define UPER_FNAME_SPI0END			"spi0_end"

#define UPER_FNAME_SPI1BEGIN		"spi1_begin"
#define UPER_FNAME_SPI1TRANS		"spi1_trans"
#define UPER_FNAME_SPI1END			"spi1_end"

#define UPER_FNAME_I2CBEGIN			"i2c_begin"
#define UPER_FNAME_I2CTRANS			"i2c_trans"
#define UPER_FNAME_I2CEND			"i2c_end"

#define UPER_FNAME_PWM0BEGIN		"pwm0_begin"
#define UPER_FNAME_PWM0SET			"pwm0_set"
#define UPER_FNAME_PWM0END			"pwm0_end"

#define UPER_FNAME_PWM1BEGIN		"pwm1_begin"
#define UPER_FNAME_PWM1SET			"pwm1_set"
#define UPER_FNAME_PWM1END			"pwm1_end"

#define UPER_FNAME_PORTWRITE		"portWrite"
#define UPER_FNAME_PORTREAD			"portRead"
#define UPER_FNAME_PORTMODE			"portMode"

#define UPER_FNAME_DHTXXREAD		"dhtRead"

#define UPER_FNAME_RESTART			"restart"

#define UPER_FNAME_GETDEVICEINFO	"GetDeviceInfo"

#endif /* FUNCTION_DEF_H_ */
