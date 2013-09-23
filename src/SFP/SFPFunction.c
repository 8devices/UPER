/**
 * @file	SFPFunction.c
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

#include "SFP/SFPFunction.h"

#include <stdlib.h>
#include <string.h>

#include <MemoryManager/MemoryManager.h>

#include "SFP/SFPMisc.h"


#define SFP_PREALLOC_SIZE	16

typedef struct _SFPArgument {
	uint32_t size;
	SFPArgumentType type;
	union {
		int32_t i;	// integer
		float f;	// float
		char* s;	// string
		uint8_t* b;	// blob
	} data;
} SFPArgument;


typedef struct _SFPFunction {
	SFPFunctionType type;

	char *name;			/* String containing function name */
	uint32_t nameSize;	/* Size (length) of the allocated *name array */

	uint32_t functionID;

	uint32_t argumentCount;		/* Number of arguments */
	SFPArgument **arguments;	/* Argument pointer array */
} SFPFunction;

/*
 * Private functions
 */

SFPResult SFPFunction_addArgument(SFPFunction *sfpFunction, SFPArgument *sfpArgument);


/*
 *
 */

SFPFunction* SFPFunction_new(void) {
	SFPFunction *func = (SFPFunction*)MemoryManager_malloc(sizeof(SFPFunction));

	if (func == NULL)
		return NULL;

	func->type = SFP_FUNC_TYPE_AUTO;

	func->name = NULL;
	func->nameSize = 0;

	func->functionID = -1;

	func->arguments = NULL;
	func->argumentCount = 0;

	return func;
}

void SFPFunction_delete(SFPFunction *func) {
	MemoryManager_free(func->name);

	uint32_t i;
	for (i=0; i<func->argumentCount; i++) {
		if (func->arguments[i]->type == SFP_ARG_STRING || func->arguments[i]->type == SFP_ARG_BYTE_ARRAY) {
			MemoryManager_free(func->arguments[i]->data.b);
		}
		MemoryManager_free(func->arguments[i]);
	}
	MemoryManager_free(func->arguments);

	MemoryManager_free(func);
}

SFPResult SFPFunction_setName(SFPFunction *func, const char* str) {
	uint32_t len = strlen(str); /* Address length */

	if (func->nameSize < len+1) {
		uint32_t newSize = SFP_PREALLOC_SIZE*(len/SFP_PREALLOC_SIZE + 1); // TODO: make it faster (replace div and mult with bit shifts)
		char* newAddress = (char*)MemoryManager_malloc(newSize);

		if (newAddress == NULL) return SFP_ERR_ALLOC_FAILED;

		MemoryManager_free(func->name);
		func->name = newAddress;
		func->nameSize = newSize;
	}

	strcpy(func->name, str);
	func->name[len] = '\0';

	return SFP_OK;
}

char* SFPFunction_getName(SFPFunction *func) {
	return func->name;
}

void SFPFunction_setID(SFPFunction *func, uint32_t id) {
	func->functionID = id;
}

uint32_t SFPFunction_getID(SFPFunction *func) {
	return func->functionID;
}

void SFPFunction_setType(SFPFunction *func, SFPFunctionType type) {
	func->type = type;
}

SFPFunctionType	SFPFunction_getType(SFPFunction *func) {
	return func->type;
}

SFPResult SFPFunction_addArgument(SFPFunction *func, SFPArgument *sfpArgument) {
	SFPArgument** newArguments = (SFPArgument**)MemoryManager_realloc(func->arguments, sizeof(SFPArgument*)*(func->argumentCount+1));

	if (newArguments == NULL) return SFP_ERR_ALLOC_FAILED;

	func->arguments = newArguments;

	/* Done alloc, now set things up */

	func->arguments[func->argumentCount] = sfpArgument;
	func->argumentCount++;

	return SFP_OK;
}

SFPResult SFPFunction_addArgument_int32(SFPFunction *func, int32_t i) {
	SFPArgument* sfpArgument = (SFPArgument*)MemoryManager_malloc(sizeof(SFPArgument));

	if (sfpArgument == NULL) return SFP_ERR_ALLOC_FAILED;

	sfpArgument->size = 4;
	sfpArgument->type = SFP_ARG_INT;
	sfpArgument->data.i = i;

	SFPResult res = SFPFunction_addArgument(func, sfpArgument);

	if (res != SFP_OK)
		MemoryManager_free(sfpArgument);

	return res;
}

SFPResult SFPFunction_addArgument_string(SFPFunction *func, const char* s) {
	/* Allocate SFPArgument */
	SFPArgument* sfpArgument = (SFPArgument*)MemoryManager_malloc(sizeof(SFPArgument));

	if (sfpArgument == NULL)
		return SFP_ERR_ALLOC_FAILED;

	/* Allocate string */
	uint32_t len = strlen(s);
	char* stringCopy = (char*)MemoryManager_malloc(len+1);

	if (stringCopy == NULL) {
		MemoryManager_free(sfpArgument);
		return SFP_ERR_ALLOC_FAILED;
	}

	/* Do the rest */

	sfpArgument->size = len;
	sfpArgument->type = SFP_ARG_STRING;
	sfpArgument->data.s = stringCopy;
	strcpy(sfpArgument->data.s, s);

	SFPResult res = SFPFunction_addArgument(func, sfpArgument);

	if (res != SFP_OK)
		MemoryManager_free(sfpArgument);

	return res;
}

SFPResult SFPFunction_addArgument_barray(SFPFunction *func, uint8_t *data, uint32_t size) {
	/* Allocate SFPArgument */
	SFPArgument *sfpArgument = (SFPArgument*)MemoryManager_malloc(sizeof(SFPArgument));

	if (sfpArgument == NULL)
		return SFP_ERR_ALLOC_FAILED;

	/* Allocate array memory */
	uint8_t *newData = (uint8_t*)MemoryManager_malloc(size);

	if (newData == NULL) {
		MemoryManager_free(sfpArgument);
		return SFP_ERR_ALLOC_FAILED;
	}

	/* Do the rest */

	sfpArgument->size = size;
	sfpArgument->type = SFP_ARG_BYTE_ARRAY;
	sfpArgument->data.b = newData;
	memcpy(sfpArgument->data.b, data, size);

	SFPResult res = SFPFunction_addArgument(func, sfpArgument);

	if (res != SFP_OK)
		MemoryManager_free(sfpArgument);

	return res;
}

SFPResult SFPFunction_setArgument_int32(SFPFunction *func, uint32_t pos, int32_t i) {
	if (pos >= func->argumentCount) return SFP_ERR;

	if (func->arguments[pos]->type != SFP_ARG_INT) return SFP_ERR;

	func->arguments[pos]->data.i = i;

	return SFP_OK;
}

uint32_t SFPFunction_getArgumentCount(SFPFunction *func) {
	return func->argumentCount;
}

SFPArgumentType SFPFunction_getArgumentType(SFPFunction *func, uint32_t position) {
	if (position < func->argumentCount)
		return func->arguments[position]->type;

	return SFP_ARG_VOID; // no argument
}

int32_t SFPFunction_getArgument_int32(SFPFunction *func, uint32_t position) {
	if (position < func->argumentCount) {
		return func->arguments[position]->data.i;
	}

	return 0;
}

char* SFPFunction_getArgument_string(SFPFunction *func, uint32_t position) {
	if (position < func->argumentCount) {
		return func->arguments[position]->data.s;
	}

	return NULL;
}

uint8_t* SFPFunction_getArgument_barray(SFPFunction *func, uint32_t position, uint32_t *size) {
	if (position < func->argumentCount) {
		*size = func->arguments[position]->size;
		return func->arguments[position]->data.b;
	}

	*size = 0;
	return NULL;
}

void SFPFunction_send(SFPFunction *func, SFPStream *stream) {
	if (func->type == SFP_FUNC_TYPE_BIN) {
		uint32_t packetSize = 1;

		uint32_t i;
		for (i=0; i<func->argumentCount; i++) {
			switch (func->arguments[i]->type) {
				case SFP_ARG_INT: {
					uint32_t argInt = func->arguments[i]->data.i;
					if (argInt == 0)
						packetSize += 1;
					else if (argInt < 0x100)
						packetSize += 2;
					else if (argInt < 0x10000)
						packetSize += 3;
					else if (argInt < 0x1000000)
						packetSize += 4;
					else
						packetSize += 5;

					break;
				}
				case SFP_ARG_STRING: {
					uint32_t argLen = func->arguments[i]->size;

					if (argLen == 0)
						packetSize += 1;
					else if (argLen < 0x100)
						packetSize += 2 + argLen;
					else if (argLen < 0x10000)
						packetSize += 3 + argLen;
					else if (argLen < 0x1000000)
						packetSize += 4 + argLen;
					else
						packetSize += 5 + argLen;

					break;
				}
				case SFP_ARG_BYTE_ARRAY: {
					uint32_t argLen = func->arguments[i]->size;

					if (argLen == 0)
						packetSize += 1;
					else if (argLen < 0x100)
						packetSize += 2 + argLen;
					else if (argLen < 0x10000)
						packetSize += 3 + argLen;
					else if (argLen < 0x1000000)
						packetSize += 4 + argLen;
					else
						packetSize += 5 + argLen;

					break;
				}
				case SFP_ARG_VOID:
					break;
			}
		}

		uint8_t *data = (uint8_t*)MemoryManager_malloc(packetSize+3); // 1b header + 2b length

		if (data == NULL) return;

		uint8_t *ptr = data;

		*ptr++ = 0xD4;
		*ptr++ = (packetSize >> 8);
		*ptr++ = (packetSize);

		*ptr++ = func->functionID;

		for (i=0; i<func->argumentCount; i++) {
			switch (func->arguments[i]->type) {
				case SFP_ARG_INT: {
					uint32_t argInt = func->arguments[i]->data.i;
					if (argInt == 0) {
						*ptr++ = 0x80;
					} else if (argInt < 0x100) {
						*ptr++ = 0x81;
						*ptr++ = argInt;
					} else if (argInt < 0x10000) {
						*ptr++ = 0x82;
						*ptr++ = (argInt >> 8);
						*ptr++ = argInt;
					} else if (argInt < 0x1000000) {
						*ptr++ = 0x83;
						*ptr++ = (argInt >> 16);
						*ptr++ = (argInt >> 8);
						*ptr++ = argInt;
					} else {
						*ptr++ = 0x84;
						*ptr++ = (argInt >> 24);
						*ptr++ = (argInt >> 16);
						*ptr++ = (argInt >> 8);
						*ptr++ = argInt;
					}

					break;
				}
				case SFP_ARG_STRING: {
					uint32_t argLen = func->arguments[i]->size;

					if (argLen == 0) {
						*ptr++ = 0x90;
					} else if (argLen < 0x100) {
						*ptr++ = 0x91;
						*ptr++ = argLen;
					} else if (argLen < 0x10000) {
						*ptr++ = 0x92;
						*ptr++ = (argLen >> 8);
						*ptr++ = argLen;
					} else if (argLen < 0x1000000) {
						*ptr++ = 0x93;
						*ptr++ = (argLen >> 16);
						*ptr++ = (argLen >> 8);
						*ptr++ = argLen;
					} else {
						*ptr++ = 0x94;
						*ptr++ = (argLen >> 24);
						*ptr++ = (argLen >> 16);
						*ptr++ = (argLen >> 8);
						*ptr++ = argLen;
					}

					uint8_t *ptrStr = func->arguments[i]->data.b;
					while (argLen--)
						*ptr++ = *ptrStr++;

					break;
				}
				case SFP_ARG_BYTE_ARRAY: {
					uint32_t argLen = func->arguments[i]->size;

					if (argLen == 0) {
						*ptr++ = 0xA0;
					} else if (argLen < 0x100) {
						*ptr++ = 0xA1;
						*ptr++ = argLen;
					} else if (argLen < 0x10000) {
						*ptr++ = 0xA2;
						*ptr++ = (argLen >> 8);
						*ptr++ = argLen;
					} else if (argLen < 0x1000000) {
						*ptr++ = 0xA3;
						*ptr++ = (argLen >> 16);
						*ptr++ = (argLen >> 8);
						*ptr++ = argLen;
					} else {
						*ptr++ = 0xA4;
						*ptr++ = (argLen >> 24);
						*ptr++ = (argLen >> 16);
						*ptr++ = (argLen >> 8);
						*ptr++ = argLen;
					}

					uint8_t *ptrStr = func->arguments[i]->data.b;
					while (argLen--)
						*ptr++ = *ptrStr++;

					break;
				}
				case SFP_ARG_VOID:
					break;
			}
		}

		stream->write(data, packetSize+3);
		MemoryManager_free(data);
	} else {	// ASCII format
		uint32_t packetSize = strlen(func->name) + 1 + 1 + 1; // name + '(' + ')' + '\n'

		uint32_t i;
		for (i=0; i<func->argumentCount; i++) {
			if (i < func->argumentCount-1)
				packetSize += 2; // space + comma
			else
				packetSize += 1; // only space

			switch (func->arguments[i]->type) {
				case SFP_ARG_INT: {
					uint32_t argInt = func->arguments[i]->data.i;
					if (argInt < 0x100)
						packetSize += 2+2;
					else if (argInt < 0x10000)
						packetSize += 2+4;
					else if (argInt < 0x1000000)
						packetSize += 2+6;
					else
						packetSize += 2+8;

					break;
				}
				case SFP_ARG_STRING: {
					uint32_t argLen = func->arguments[i]->size;

					packetSize += argLen + 2; // chars + two double quotes
					break;
				}
				case SFP_ARG_BYTE_ARRAY: {
					uint32_t argLen = func->arguments[i]->size;

					if (argLen > 1) {
						packetSize += 6*argLen; // 2 brackets + 4*N hex data + (N-1) commas + (N-1) spaces
					} else {
						packetSize += 2 + 4*argLen; // 2 brackets + 4*N hex data
					}
					break;
				}
				case SFP_ARG_VOID:
					break;
			}
		}

		uint8_t *data = (uint8_t*)MemoryManager_malloc(packetSize);

		if (data == NULL) return;

		uint8_t *ptr = data;

		strcpy((char*)ptr, func->name);
		ptr += strlen(func->name);
		*ptr++ = '(';

		for (i=0; i<func->argumentCount; i++) {
			*ptr++ = ' ';

			switch (func->arguments[i]->type) {
				case SFP_ARG_INT: {
					uint32_t argInt = func->arguments[i]->data.i;
					*ptr++ = '0';
					*ptr++ = 'x';

					if (argInt > 0xFFFFFF) {
						*ptr++ = SFPMisc_hexChar(argInt >> 28);
						*ptr++ = SFPMisc_hexChar(argInt >> 24);
					}
					if (argInt > 0xFFFF) {
						*ptr++ = SFPMisc_hexChar(argInt >> 20);
						*ptr++ = SFPMisc_hexChar(argInt >> 16);
					}
					if (argInt > 0xFF) {
						*ptr++ = SFPMisc_hexChar(argInt >> 12);
						*ptr++ = SFPMisc_hexChar(argInt >> 8);
					}
					*ptr++ = SFPMisc_hexChar(argInt >> 4);
					*ptr++ = SFPMisc_hexChar(argInt);


					break;
				}
				case SFP_ARG_STRING: {
					*ptr++ = '"';
					strcpy((char*)ptr, func->arguments[i]->data.s);
					ptr += strlen(func->arguments[i]->data.s);
					*ptr++ = '"';
					break;
				}
				case SFP_ARG_BYTE_ARRAY: {
					*ptr++ = '[';
					uint8_t *bArray = func->arguments[i]->data.b;
					uint32_t bLen = func->arguments[i]->size;
					uint32_t j;
					for (j=0; j<bLen; j++) {
						if (j) {
							*ptr++ = ',';
							*ptr++ = ' ';
						}
						*ptr++ = '0';
						*ptr++ = 'x';
						*ptr++ = SFPMisc_hexChar(bArray[j] >> 4);
						*ptr++ = SFPMisc_hexChar(bArray[j]);
					}
					*ptr++ = ']';
					break;
				}
				case SFP_ARG_VOID:
					break;
			}

			if (i < func->argumentCount-1)
				*ptr++ = ',';
		}
		*ptr++ = ')';
		*ptr++ = '\n';

		stream->write(data, packetSize);
		MemoryManager_free(data);
	}
}
