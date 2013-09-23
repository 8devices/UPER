/**
 * @file	SFPFunction.h
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
 * SFPFunction - class-like SFPFunction structure and functions for creating,
 * deleting and manipulating the contents of the SFPFunction.
 *
 */

#ifndef SFPFUNCTION_H_
#define SFPFUNCTION_H_

#include <stdint.h>
#include "SFPStream.h"

typedef enum {
	SFP_OK=0,
	SFP_ERR=1,
	SFP_ERR_ALLOC_FAILED,
	SFP_ERR_FORMAT,
	SFP_ERR_ARG_COUNT,
	SFP_ERR_ARG_TYPE,
	SFP_ERR_ARG_VALUE
} SFPResult;

typedef enum {
	SFP_FUNC_TYPE_AUTO = 0,
	SFP_FUNC_TYPE_BIN = 1,
	SFP_FUNC_TYPE_TEXT = 2
} SFPFunctionType;

typedef enum {
	SFP_ARG_VOID = 0,
	SFP_ARG_INT = 'i',
	SFP_ARG_STRING = 's',
	SFP_ARG_BYTE_ARRAY = 'b',
} SFPArgumentType;

typedef struct _SFPFunction SFPFunction;

SFPFunction*	SFPFunction_new(void);
void			SFPFunction_delete(SFPFunction *func);

SFPResult		SFPFunction_setName(SFPFunction *func, const char* str);
char*			SFPFunction_getName(SFPFunction *func);

void			SFPFunction_setID(SFPFunction *func, uint32_t id);
uint32_t		SFPFunction_getID(SFPFunction *func);

void			SFPFunction_setType(SFPFunction *func, SFPFunctionType type);
SFPFunctionType	SFPFunction_getType(SFPFunction *func);

SFPResult		SFPFunction_addArgument_int32(SFPFunction *func, int32_t i);
SFPResult		SFPFunction_addArgument_string(SFPFunction *func, const char* s);
SFPResult		SFPFunction_addArgument_barray(SFPFunction *func, uint8_t *data, uint32_t size);

SFPResult		SFPFunction_setArgument_int32(SFPFunction *func, uint32_t pos, int32_t i);

uint32_t		SFPFunction_getArgumentCount(SFPFunction *func);
SFPArgumentType	SFPFunction_getArgumentType(SFPFunction *func, uint32_t position);

int32_t			SFPFunction_getArgument_int32 (SFPFunction *func, uint32_t position);
char*			SFPFunction_getArgument_string(SFPFunction *func, uint32_t position);
uint8_t*		SFPFunction_getArgument_barray(SFPFunction *func, uint32_t position, uint32_t *size);

void			SFPFunction_send(SFPFunction *func, SFPStream *stream);

#endif /* SFPFUNCTION_H_ */
