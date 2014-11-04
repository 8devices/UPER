/**
 * @file	LPC_GPIO.h
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

#ifndef LPC_GPIO_H_
#define LPC_GPIO_H_

#include "main.h"

#define LPC_PIN_COUNT	34
#define LPC_INTERRUPT_COUNT 8
#define LPC_PORT_COUNT	4

inline void GPIO_EnableInterrupt(uint8_t intID);

void FLEX_INT0_IRQHandler(void);
void FLEX_INT1_IRQHandler(void);
void FLEX_INT2_IRQHandler(void);
void FLEX_INT3_IRQHandler(void);
void FLEX_INT4_IRQHandler(void);
void FLEX_INT5_IRQHandler(void);
void FLEX_INT6_IRQHandler(void);
void FLEX_INT7_IRQHandler(void);

void lpc_config_gpioInit(void);

SFPResult lpc_config_setPrimary(SFPFunction *msg);

SFPResult lpc_config_setSecondary(SFPFunction *msg);

SFPResult lpc_pinMode(SFPFunction *msg);

SFPResult lpc_digitalWrite(SFPFunction *msg);

SFPResult lpc_digitalRead(SFPFunction *msg);

SFPResult lpc_portMode(SFPFunction *msg);

SFPResult lpc_portWrite(SFPFunction *msg);

SFPResult lpc_portRead(SFPFunction *msg);

SFPResult lpc_attachInterrupt(SFPFunction *msg);

SFPResult lpc_detachInterrupt(SFPFunction *msg);

SFPResult lpc_pulseIn(SFPFunction *msg);

SFPResult lpc_dhtxxRead(SFPFunction *msg);


#endif /* LPC_GPIO_H_ */
