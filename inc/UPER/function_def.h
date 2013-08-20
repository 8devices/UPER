/**
 * @file	main.h
 * @author  Giedrius Medzevicius <giedrius@8devices.com>
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Giedrius Medzevicius
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
 * Functions IDs of the incoming packets
 */
#define UPER_FUNCTION_ID_IN_SETPRIMARY		1
#define UPER_FUNCTION_ID_IN_SETSECONDARY	2
#define UPER_FUNCTION_ID_IN_PINMODE			3
#define UPER_FUNCTION_ID_IN_DIGITALWRITE	4
#define UPER_FUNCTION_ID_IN_DIGITALREAD		5
#define UPER_FUNCTION_ID_IN_ATTACHINTERRUPT	6
#define UPER_FUNCTION_ID_IN_DETACHINTERRUPT	7

#define UPER_FUNCTION_ID_IN_ANALOGREAD		10

#define UPER_FUNCTION_ID_IN_SPI0BEGIN		20
#define UPER_FUNCTION_ID_IN_SPI0TRANS		21
#define UPER_FUNCTION_ID_IN_SPI0END			22

#define UPER_FUNCTION_ID_IN_SPI1BEGIN		30
#define UPER_FUNCTION_ID_IN_SPI1TRANS		31
#define UPER_FUNCTION_ID_IN_SPI1END			32

#define UPER_FUNCTION_ID_IN_I2CBEGIN		40
#define UPER_FUNCTION_ID_IN_I2CTRANS		41
#define UPER_FUNCTION_ID_IN_I2CEND			42

#define UPER_FUNCTION_ID_IN_PWM0BEGIN		50
#define UPER_FUNCTION_ID_IN_PWM0SET			51
#define UPER_FUNCTION_ID_IN_PWM0END			52

#define UPER_FUNCTION_ID_IN_PWM1BEGIN		60
#define UPER_FUNCTION_ID_IN_PWM1SET			61
#define UPER_FUNCTION_ID_IN_PWM1END			62

#define UPER_FUNCTION_ID_IN_REGISTERWRITE	100
#define UPER_FUNCTION_ID_IN_REGISTERREAD	101

/*
 * Function IDs of the outgoing packets
 */

#define UPER_FUNCTION_ID_OUT_DIGITALREAD	8
#define UPER_FUNCTION_ID_OUT_INTERRUPT		9

#define UPER_FUNCTION_ID_OUT_ANALOGREAD		11

#define UPER_FUNCTION_ID_OUT_SPI0TRANS		23

#define UPER_FUNCTION_ID_OUT_SPI1TRANS		33

#define UPER_FUNCTION_ID_OUT_I2CTRANS		43

#define UPER_FUNCTION_ID_OUT_REGISTERREAD	102


/*
 * Incoming function names
 */

#define UPER_FUNCTION_NAME_IN_SETPRIMARY		"setPrimary"
#define UPER_FUNCTION_NAME_IN_SETSECONDARY		"setSecondary"
#define UPER_FUNCTION_NAME_IN_PINMODE			"pinMode"
#define UPER_FUNCTION_NAME_IN_DIGITALWRITE		"digitalWrite"
#define UPER_FUNCTION_NAME_IN_DIGITALREAD		"digitalRead"
#define UPER_FUNCTION_NAME_IN_ATTACHINTERRUPT	"attachInterrupt"
#define UPER_FUNCTION_NAME_IN_DETACHINTERRUPT	"detachInterrupt"

#define UPER_FUNCTION_NAME_IN_ANALOGREAD		"analogRead"

#define UPER_FUNCTION_NAME_IN_SPI0BEGIN			"spi0_begin"
#define UPER_FUNCTION_NAME_IN_SPI0TRANS			"spi0_trans"
#define UPER_FUNCTION_NAME_IN_SPI0END			"spi0_end"

#define UPER_FUNCTION_NAME_IN_SPI1BEGIN			"spi1_begin"
#define UPER_FUNCTION_NAME_IN_SPI1TRANS			"spi1_trans"
#define UPER_FUNCTION_NAME_IN_SPI1END			"spi1_end"

#define UPER_FUNCTION_NAME_IN_I2CBEGIN			"i2c_begin"
#define UPER_FUNCTION_NAME_IN_I2CTRANS			"i2c_trans"
#define UPER_FUNCTION_NAME_IN_I2CEND			"i2c_end"

#define UPER_FUNCTION_NAME_IN_PWM0BEGIN			"pwm0_begin"
#define UPER_FUNCTION_NAME_IN_PWM0SET			"pwm0_set"
#define UPER_FUNCTION_NAME_IN_PWM0END			"pwm0_end"

#define UPER_FUNCTION_NAME_IN_PWM1BEGIN			"pwm1_begin"
#define UPER_FUNCTION_NAME_IN_PWM1SET			"pwm1_set"
#define UPER_FUNCTION_NAME_IN_PWM1END			"pwm1_end"

#define UPER_FUNCTION_NAME_IN_REGISTERWRITE		"registerWrite"
#define UPER_FUNCTION_NAME_IN_REGISTERREAD		"registerRead"

/*
 * Outgoing function names
 */

#define UPER_FUNCTION_NAME_OUT_DIGITALREAD		"digitalRead"
#define UPER_FUNCTION_NAME_OUT_INTERRUPT		"interrupt"

#define UPER_FUNCTION_NAME_OUT_ANALOGREAD		"analogRead"

#define UPER_FUNCTION_NAME_OUT_SPI0TRANS		"spi0_trans"

#define UPER_FUNCTION_NAME_OUT_SPI1TRANS		"spi1_trans"

#define UPER_FUNCTION_NAME_OUT_I2CTRANS			"i2c_trans"

#define UPER_FUNCTION_NAME_OUT_REGISTERREAD		"registerRead"

#endif /* FUNCTION_DEF_H_ */
