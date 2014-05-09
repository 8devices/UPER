/**
 * @file	main.h
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

#ifndef MAIN_H_
#define MAIN_H_


#define UPER_DEVICE_TYPE		'U'
#define UPER_FW_VERSION_MAJOR	0
#define UPER_FW_VERSION_MINOR	0
#define UPER_FIRMWARE_VERSION		((UPER_DEVICE_TYPE << 24) | (UPER_FW_VERSION_MAJOR << 16) | UPER_FW_VERSION_MINOR)

#include "LPC11Uxx.h"

#include "lpc_def.h"

#include "UPER/function_def.h"

#include "SFP/SFP.h"

#include <MemoryManager/MemoryManager.h>

#include "time.h"

SFPStream stream;

uint32_t GUID[4];
uint32_t UPER_PART_NUMBER;
uint32_t UPER_BOOT_CODE_VERSION;

#endif /* MAIN_H_ */
