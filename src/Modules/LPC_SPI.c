/**
 * @file	LPC_SPI.c
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


#include "Modules/LPC_SPI.h"

/*
 * SPI0
 */
SFPResult lpc_spi0_begin(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT
			|| SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint32_t divider = (SFPFunction_getArgument_int32(msg, 0)-1) & 0xFF;
	uint32_t mode = SFPFunction_getArgument_int32(msg, 1) & 0x3;

	LPC_SYSCON->PRESETCTRL |= 1; 		// de-assert SPI0
	LPC_SYSCON->SYSAHBCLKCTRL |= BIT11;	// enable SPI0 clock
	LPC_SYSCON->SSP0CLKDIV = 1; //48MHz


	LPC_SSP0->CR1 = 0;		// Master mode, SPI disabled
	LPC_SSP0->CPSR = 24;	// 48MHz/24 = 2MHz
	LPC_SSP0->CR0 = (0x7) | (0 << 4) | (mode << 6) | (divider << 8); // 8bits, SPI mode x
	LPC_SSP0->IMSC = 0;		// Interrupts disabled
	LPC_SSP0->CR1 = BIT1;	//Master mode, SPI enabled

	while (LPC_SSP0->SR & BIT4);	// wait while BUSY (reading or writing)

	while (LPC_SSP0->SR & BIT2) {	// Read while Rx FIFO not empty
		LPC_SSP0->DR;
	}

	return SFP_OK;
}

SFPResult lpc_spi0_trans(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_BYTE_ARRAY
			|| SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint32_t dataSize, writeSize;
	uint8_t *data = SFPFunction_getArgument_barray(msg, 0, &dataSize);

	uint32_t readSize = writeSize = dataSize;
	uint8_t *readBuf = NULL, *readPtr = NULL;

	uint8_t requestRead =  SFPFunction_getArgument_int32(msg, 1) & 0x1;
	if (requestRead) {
		readBuf = (uint8_t*)MemoryManager_malloc(writeSize);
		readPtr = readBuf;

		if (readBuf == NULL)
			return SFP_ERR_ALLOC_FAILED;
	}

	while (writeSize || readSize) {
		while (writeSize && (LPC_SSP0->SR & BIT1)) { // Tx FIFO not full
			LPC_SSP0->DR = *data++;
			writeSize--;
		}

		while (readSize && (LPC_SSP0->SR & BIT2)) { // Rx FIFO not empty
			uint32_t tmp = LPC_SSP0->DR;
			readSize--;
			if (readBuf != NULL)
				*readPtr++ = tmp;
		}
	}

	if (readBuf != NULL) {
		SFPFunction *outFunc = SFPFunction_new();

		if (outFunc == NULL) {
			MemoryManager_free(readBuf);
			return SFP_ERR_ALLOC_FAILED;
		}

		SFPFunction_setType(outFunc, SFPFunction_getType(msg));
		SFPFunction_setID(outFunc, UPER_FID_SPI0TRANS);
		SFPFunction_setName(outFunc, UPER_FNAME_SPI0TRANS);
		SFPFunction_addArgument_barray(outFunc, readBuf, dataSize);
		SFPFunction_send(outFunc, &stream);
		SFPFunction_delete(outFunc);

		MemoryManager_free(readBuf);
	}

	return SFP_OK;
}

SFPResult lpc_spi0_end(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 0)
		return SFP_ERR_ARG_COUNT;

	LPC_SSP0->CR1 = 0;		// SPI disabled
	LPC_SYSCON->SYSAHBCLKCTRL &= ~BIT11;	// disable SPI0 clock
	LPC_SYSCON->PRESETCTRL &= ~1; 			// assert SPI0

	return SFP_OK;
}

/*
 * SPI1
 */
SFPResult lpc_spi1_begin(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_INT
			|| SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint32_t divider = (SFPFunction_getArgument_int32(msg, 0)-1) & 0xFF;
	uint32_t mode = SFPFunction_getArgument_int32(msg, 1) & 0x3;

	LPC_SYSCON->PRESETCTRL |= BIT2; 	// de-assert SPI1
	LPC_SYSCON->SYSAHBCLKCTRL |= BIT18;	// enable SPI1 clock
	LPC_SYSCON->SSP1CLKDIV = 1; 		//48MHz


	LPC_SSP1->CR1 = 0;		// Master mode, SPI disabled
	LPC_SSP1->CPSR = 24;	// 48MHz/24 = 2MHz
	LPC_SSP1->CR0 = (0x7) | (0 << 4) | (mode << 6) | (divider << 8); // 8bits, SPI mode x
	LPC_SSP1->IMSC = 0;		// Interrupts disabled
	LPC_SSP1->CR1 = BIT1;	//Master mode, SPI enabled

	while (LPC_SSP1->SR & BIT4);	// wait while BUSY (reading or writing)

	while (LPC_SSP1->SR & BIT2) {	// Read while Rx FIFO not empty
		LPC_SSP1->DR;
	}

	return SFP_OK;
}

SFPResult lpc_spi1_trans(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 2)
		return SFP_ERR_ARG_COUNT;

	if (SFPFunction_getArgumentType(msg, 0) != SFP_ARG_BYTE_ARRAY
			|| SFPFunction_getArgumentType(msg, 1) != SFP_ARG_INT)
		return SFP_ERR_ARG_TYPE;

	uint32_t dataSize, writeSize;
	uint8_t *data = SFPFunction_getArgument_barray(msg, 0, &dataSize);

	uint32_t readSize = writeSize = dataSize;
	uint8_t *readBuf = NULL, *readPtr = NULL;

	uint8_t requestRead =  SFPFunction_getArgument_int32(msg, 1) & 0x1;
	if (requestRead) {
		readBuf = (uint8_t*)MemoryManager_malloc(writeSize);
		readPtr = readBuf;

		if (readBuf == NULL)
			return SFP_ERR_ALLOC_FAILED;
	}

	while (writeSize || readSize) {
		while (writeSize && (LPC_SSP1->SR & BIT1)) { // Tx FIFO not full
			LPC_SSP1->DR = *data++;
			writeSize--;
		}

		while (readSize && (LPC_SSP1->SR & BIT2)) { // Rx FIFO not empty
			uint32_t tmp = LPC_SSP1->DR;
			readSize--;
			if (readBuf != NULL)
				*readPtr++ = tmp;
		}
	}

	if (readBuf != NULL) {
		SFPFunction *outFunc = SFPFunction_new();

		if (outFunc == NULL) {
			MemoryManager_free(readBuf);
			return SFP_ERR_ALLOC_FAILED;
		}

		SFPFunction_setType(outFunc, SFPFunction_getType(msg));
		SFPFunction_setID(outFunc, UPER_FID_SPI1TRANS);
		SFPFunction_setName(outFunc, UPER_FNAME_SPI1TRANS);
		SFPFunction_addArgument_barray(outFunc, readBuf, dataSize);
		SFPFunction_send(outFunc, &stream);
		SFPFunction_delete(outFunc);

		MemoryManager_free(readBuf);
	}

	return SFP_OK;
}

SFPResult lpc_spi1_end(SFPFunction *msg) {
	if (SFPFunction_getArgumentCount(msg) != 0)
		return SFP_ERR_ARG_COUNT;

	LPC_SSP1->CR1 = 0;		// SPI disabled
	LPC_SYSCON->SYSAHBCLKCTRL &= ~BIT18;	// disable SPI1 clock
	LPC_SYSCON->PRESETCTRL &= ~BIT2;		// assert SPI1

	return SFP_OK;
}
