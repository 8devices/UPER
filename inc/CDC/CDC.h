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
 */

#ifndef CDC_H_
#define CDC_H_

#include "LPC11Uxx.h"
#include "mw_usbd_rom_api.h"
#include "power_api.h"

#include <SFP/SFPStream.h>

#define CDC_CIF1_INT_IN_EP		USB_ENDPOINT_IN(1)
#define CDC_DIF1_BULK_OUT_EP	USB_ENDPOINT_OUT(2)
#define CDC_DIF1_BULK_IN_EP		USB_ENDPOINT_IN(2)

#define CDC_CIF2_INT_IN_EP		USB_ENDPOINT_IN(3)
#define CDC_DIF2_BULK_OUT_EP	USB_ENDPOINT_OUT(4)
#define CDC_DIF2_BULK_IN_EP		USB_ENDPOINT_IN(4)

extern uint8_t VCOM_DeviceDescriptor[];
extern uint8_t VCOM_StringDescriptor[];
extern uint8_t VCOM_ConfigDescriptor[];

ErrorCode_t CDC_Init(SFPStream *stream);

#endif /* CDC_H_ */

