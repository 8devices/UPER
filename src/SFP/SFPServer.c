/**
 * @file	SFPServer.c
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

#include "SFP/SFPServer.h"
#include "SFP/SFPMisc.h"

#include <MemoryManager/MemoryManager.h>

#include <stdlib.h>
#include <string.h>

#define SFP_BINARY_FUNCTION_ID	(0xD4)

typedef struct {
	char* functionName;
	uint32_t functionID;
	SFPCallbackFunction method;
} SFPFunctionHandlerEntry;

typedef enum {
	SFP_STAGE_FUNCTION_START = 0,
	SFP_STAGE_BINARY_SIZE = 1,
	SFP_STAGE_BINARY_BODY = 2,

	SFP_STAGE_PARAM_START = 10,
	SFP_STAGE_PARAM_PARSE_INT_UNKNOWN = 11,
	SFP_STAGE_PARAM_PARSE_INT_DECIMAL = 12,
	SFP_STAGE_PARAM_PARSE_INT_HEX = 13,
	SFP_STAGE_PARAM_PARSE_INT_BINARY = 14,
	SFP_STAGE_PARAM_PARSE_INT_OCTAL = 15,

	SFP_STAGE_PARAM_PARSE_STRING = 30,
	SFP_STAGE_PARAM_PARSE_STRING_ESCAPE = 31,
	SFP_STAGE_PARAM_PARSE_STRING_HEX = 32,
	SFP_STAGE_PARAM_PARSE_STRING_OCTAL = 33,

	SFP_STAGE_PARAM_PARSE_ARRAY = 40,
	SFP_STAGE_PARAM_PARSE_ARRAY_INT_UNKNOWN = 41,
	SFP_STAGE_PARAM_PARSE_ARRAY_INT_DECIMAL = 42,
	SFP_STAGE_PARAM_PARSE_ARRAY_INT_HEX = 43,
	SFP_STAGE_PARAM_PARSE_ARRAY_INT_BINARY = 44,
	SFP_STAGE_PARAM_PARSE_ARRAY_INT_OCTAL = 45,
	SFP_STAGE_PARAM_PARSE_ARRAY_CHAR = 46,
	SFP_STAGE_PARAM_PARSE_ARRAY_END = 49,

	SFP_STAGE_PARAM_END = 99,
} SFPParseStage;

typedef struct _SFPServer {
	SFPStream *stream;

	SFPFunctionHandlerEntry *handlers;
	uint32_t handlerCount;

	SFPCallbackFunction defaultHandler;

	SFPParseStage stage;

	SFPFunction *tmpFunction;

	uint8_t* tmpStringBuf;
	uint32_t tmpStringBufSize;
	uint32_t tmpStringBufPos;

	uint32_t tmpArgInt;
	int32_t tmpLenInt;

} SFPServer;

/*
 * Private functions
 */

void SFPServer_handleParsedFunction(SFPServer *server);


/*
 * Allocation functions
 */

SFPServer*	SFPServer_new(SFPStream *sfpStream) {
	SFPServer *server = (SFPServer*)MemoryManager_malloc(sizeof(SFPServer));

	if (server == NULL)
		return NULL;

	server->stream = sfpStream;

	server->handlers = NULL;
	server->handlerCount = 0;

	server->defaultHandler = NULL;

	server->stage = SFP_STAGE_FUNCTION_START;

	server->tmpFunction = SFPFunction_new();

	if (server->tmpFunction == NULL) {
		SFPServer_delete(server);
		return NULL;
	}

	server->tmpStringBuf = NULL;
	server->tmpStringBufSize = 0;
	server->tmpStringBufPos = 0;

	return server;
}

void SFPServer_delete(SFPServer *sfpServer) {
	if (sfpServer->handlers != NULL) {
		uint32_t i;
		for (i=0; i<sfpServer->handlerCount; i++) {
			MemoryManager_free(sfpServer->handlers[i].functionName);
		}
		MemoryManager_free(sfpServer->handlers);
	}

	SFPFunction_delete(sfpServer->tmpFunction);

	MemoryManager_free(sfpServer);
}

/*
 * Function handling
 */

SFPResult SFPServer_addFunctionHandler(SFPServer *sfpServer, const char* fName, uint32_t fID, SFPCallbackFunction func) {
	//TODO: check if fName is valid
	uint32_t len = strlen(fName);
	char *nameCopy = (char*)MemoryManager_malloc(len+1); // include the null character

	if (nameCopy == NULL)
		return SFP_ERR_ALLOC_FAILED;

	SFPFunctionHandlerEntry *newHandlers = (SFPFunctionHandlerEntry*)MemoryManager_realloc(sfpServer->handlers, sizeof(SFPFunctionHandlerEntry)*(sfpServer->handlerCount+1));

	if (newHandlers == NULL) {
		MemoryManager_free(nameCopy);
		return SFP_ERR_ALLOC_FAILED;
	}

	memcpy(nameCopy, fName, len+1);

	sfpServer->handlers = newHandlers;
	sfpServer->handlers[sfpServer->handlerCount].functionName = nameCopy;
	sfpServer->handlers[sfpServer->handlerCount].functionID = fID;
	sfpServer->handlers[sfpServer->handlerCount].method  = func;
	sfpServer->handlerCount++;

	return SFP_OK;
}

SFPResult SFPServer_removeFunctionHandler(SFPServer *sfpServer, const char* fName, uint32_t fID, SFPCallbackFunction func) {

	uint32_t i;
	for (i=0; i<sfpServer->handlerCount; i++) {
		if (sfpServer->handlers[i].method == func && strcmp(sfpServer->handlers[i].functionName, fName) == 0)
			break;
	}

	if (i == sfpServer->handlerCount)	// handler not found
		return SFP_ERR;

	MemoryManager_free(sfpServer->handlers[i].functionName);

	if (i+1 < sfpServer->handlerCount) { // if it's not the last handler
		memmove(&sfpServer->handlers[i], &sfpServer->handlers[i+1], sizeof(SFPFunctionHandlerEntry)*(sfpServer->handlerCount-i-1));
	}

	SFPFunctionHandlerEntry *newHandlers = (SFPFunctionHandlerEntry*)MemoryManager_realloc(sfpServer->handlers, sizeof(SFPFunctionHandlerEntry)*(sfpServer->handlerCount-1));

	if (newHandlers == NULL) {
		sfpServer->handlerCount--;
		return SFP_ERR_ALLOC_FAILED;
	}

	sfpServer->handlers = newHandlers;
	sfpServer->handlerCount--;

	return SFP_OK;
}

SFPResult SFPServer_setDefaultFunctionHandler(SFPServer *sfpServer, SFPCallbackFunction func) {
	sfpServer->defaultHandler = func;
	return SFP_OK;
}

void SFPServer_handleParsedFunction(SFPServer *server) {
	uint8_t executed = 0;

	SFPFunctionType type = SFPFunction_getType(server->tmpFunction);

	if (type == SFP_FUNC_TYPE_BIN) {
		uint32_t functionID = SFPFunction_getID(server->tmpFunction);

		uint32_t i;
		for (i=0; i<server->handlerCount; i++) {
			if (server->handlers[i].functionID == functionID) {
				server->handlers[i].method(server->tmpFunction);
				executed = 1;
			}
		}
	} else {
		char *functionName = SFPFunction_getName(server->tmpFunction);

		uint32_t i;
		for (i=0; i<server->handlerCount; i++) {
			if (strcmp(server->handlers[i].functionName, functionName) == 0) {
				server->handlers[i].method(server->tmpFunction);
				executed = 1;
			}
		}
	}

	if (!executed) {
		server->defaultHandler(server->tmpFunction);
	}
}

/*
 * Server runtime
 */
#define SFP_PREALLOC_SIZE	16

uint8_t static inline SFPServer_tmpBufferEnsureCapacity(SFPServer *server, uint32_t capacity) {
	if (server->tmpStringBufSize < capacity) {
		uint32_t newSize = (capacity/SFP_PREALLOC_SIZE + 1)*SFP_PREALLOC_SIZE;
		uint8_t* newAddress = (uint8_t*)MemoryManager_realloc(server->tmpStringBuf, newSize);

		if (newAddress == NULL) return 0;

		server->tmpStringBuf = newAddress;
		server->tmpStringBufSize = newSize;
	}
	return 1;
}

void static inline SFPServer_tmpBufferAppendByte(SFPServer *server, uint8_t b) {
	if (SFPServer_tmpBufferEnsureCapacity(server, server->tmpStringBufPos+1)) {
		server->tmpStringBuf[server->tmpStringBufPos++] = b;
	}
}

void static inline SFPServer_tmpBufferClear(SFPServer *server) {
	MemoryManager_free(server->tmpStringBuf);

	server->tmpStringBuf = NULL;
	server->tmpStringBufSize = 0;
	server->tmpStringBufPos = 0;
}

SFPResult SFPServer_cycle(SFPServer *server) {
	SFPStream *stream = server->stream;

	uint32_t available = stream->available();
	while (available > 0) {
		switch (server->stage) {
			case SFP_STAGE_FUNCTION_START:
				while (available) {
					char c = stream->readByte();
					available--;

					if (c == SFP_BINARY_FUNCTION_ID) {
						SFPFunction_delete(server->tmpFunction);
						server->tmpFunction = SFPFunction_new();
						SFPFunction_setType(server->tmpFunction, SFP_FUNC_TYPE_BIN);
						SFPServer_tmpBufferClear(server);

						server->stage = SFP_STAGE_BINARY_SIZE;
						break;
					} else if (SFPMisc_isFunctionNameChar(c)) {
						SFPServer_tmpBufferAppendByte(server, c);
					} else if (c == '(') {
						SFPFunction_delete(server->tmpFunction);
						server->tmpFunction = SFPFunction_new();
						SFPFunction_setType(server->tmpFunction, SFP_FUNC_TYPE_TEXT);
						SFPServer_tmpBufferAppendByte(server, '\0');
						SFPFunction_setName(server->tmpFunction, (char*)server->tmpStringBuf);
						SFPServer_tmpBufferClear(server);

						server->stage = SFP_STAGE_PARAM_START;
						break;
					} else {
						SFPServer_tmpBufferClear(server);
					}
				}
				break;
			case SFP_STAGE_BINARY_SIZE:
				if (available < 2) {
					return SFP_OK;
				}
				server->tmpLenInt = (stream->readByte() << 8) | stream->readByte();
				available -= 2;

				server->stage = SFP_STAGE_BINARY_BODY;

			case SFP_STAGE_BINARY_BODY:
				if (available < server->tmpLenInt) {
					return SFP_OK;
				}

				SFPFunction_setID(server->tmpFunction, stream->readByte());
				server->tmpLenInt--;
				available -= (server->tmpLenInt + 1);

				while (server->tmpLenInt > 0) {
					uint8_t argType = stream->readByte();

					server->tmpArgInt = 0;
					uint8_t i = argType & 0xF;
					while (i--) {
						server->tmpArgInt = (server->tmpArgInt << 8) | stream->readByte();
					}
					server->tmpLenInt -= (1 + (argType & 0xF));	// argType + argData

					switch (argType & 0xF0) {
						case 0x80:
							SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
							break;
						case 0x90:
							if (SFPServer_tmpBufferEnsureCapacity(server, server->tmpArgInt+1)) {
								stream->read((uint8_t*)server->tmpStringBuf, server->tmpArgInt);
								server->tmpStringBuf[server->tmpArgInt] = '\0';
								SFPFunction_addArgument_string(server->tmpFunction, (char*)server->tmpStringBuf);
								SFPServer_tmpBufferClear(server);
							} else {
								server->stage = SFP_STAGE_FUNCTION_START;
								SFPServer_tmpBufferClear(server);
								SFPFunction_delete(server->tmpFunction);
								return SFP_ERR_ALLOC_FAILED;
							}
							break;
						case 0xA0:
							if (SFPServer_tmpBufferEnsureCapacity(server, server->tmpArgInt)) {
								stream->read((uint8_t*)server->tmpStringBuf, server->tmpArgInt);
								SFPFunction_addArgument_barray(server->tmpFunction, server->tmpStringBuf, (uint32_t)server->tmpArgInt);
								SFPServer_tmpBufferClear(server);
							} else {
								server->stage = SFP_STAGE_FUNCTION_START;
								SFPServer_tmpBufferClear(server);
								SFPFunction_delete(server->tmpFunction);
								return SFP_ERR_ALLOC_FAILED;
							}
							break;
						default:
							server->stage = SFP_STAGE_FUNCTION_START;
							SFPServer_tmpBufferClear(server);
							SFPFunction_delete(server->tmpFunction);
							return SFP_ERR_ALLOC_FAILED;
					}
				}

				if (server->tmpLenInt == 0) { // All arguments are read - execute
					SFPServer_handleParsedFunction(server);
					server->stage = SFP_STAGE_FUNCTION_START;
				} else if (server->tmpLenInt < 0) { // Too many bytes were read - error
					server->stage = SFP_STAGE_FUNCTION_START;
					return SFP_ERR_FORMAT;
				}
				break;
			case SFP_STAGE_PARAM_START:
				while (available) {
					char c = stream->readByte();
					available--;

					if ((c >= '1') && (c <= '9')) {
						server->tmpArgInt = c - '0';
						server->stage = SFP_STAGE_PARAM_PARSE_INT_DECIMAL;
						break;
					} else if (c == '0') {
						server->tmpArgInt = 0;
						server->stage = SFP_STAGE_PARAM_PARSE_INT_UNKNOWN;
						break;
					} else if (c == '"') {
						SFPServer_tmpBufferClear(server);
						server->stage = SFP_STAGE_PARAM_PARSE_STRING;
						break;
					} else if (c == '[') {
						SFPServer_tmpBufferClear(server);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY;
						break;
					} else if (c == ')') {
						SFPServer_handleParsedFunction(server);
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					} else if (c == ' ') {
						// Do nothing
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_INT_UNKNOWN:
			{
				char c = stream->readByte();
				available--;

				if (c == 'x') {
					server->stage = SFP_STAGE_PARAM_PARSE_INT_HEX;
					break;
				} else if (c == 'b') {
					server->stage = SFP_STAGE_PARAM_PARSE_INT_BINARY;
					break;
				} else if ((c >= '0') && (c <= '7')) {
					server->tmpArgInt = c - '0';
					server->stage = SFP_STAGE_PARAM_PARSE_INT_OCTAL;
					break;
				} else if (c == ',') {
					SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
					server->stage = SFP_STAGE_PARAM_START;
					break;
				} else if (c == ' ') {
					SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
					server->stage = SFP_STAGE_PARAM_END;
					break;
				} else if (c == ')') {
					SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
					SFPServer_handleParsedFunction(server);
					server->stage = SFP_STAGE_FUNCTION_START;
					break;
				} else {
					server->stage = SFP_STAGE_FUNCTION_START;
					break;
				}
			}
				break;
			case SFP_STAGE_PARAM_PARSE_INT_DECIMAL:
				while (available) {
					available--;
					char c = stream->readByte();

					if ((c >= '0') && (c <= '9')) {
						server->tmpArgInt = server->tmpArgInt*10 + (c-'0');
					} else if (c == ',') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_START;
						break;
					} else if (c == ' ') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else if (c == ')') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						SFPServer_handleParsedFunction(server);
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_INT_HEX:
				while (available) {
					available--;
					char c = stream->readByte();

					if ((c >= '0') && (c <= '9')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'0');
					} else if ((c >= 'a') && (c <= 'f')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'a'+10);
					} else if ((c >= 'A') && (c <= 'F')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'A'+10);
					} else if (c == ',') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_START;
						break;
					} else if (c == ' ') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else if (c == ')') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						SFPServer_handleParsedFunction(server);
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_INT_BINARY:
				while (available) {
					available--;
					char c = stream->readByte();

					if ((c == '0') || (c == '1')) {
						server->tmpArgInt = (server->tmpArgInt << 1) + (c-'0');
					} else if (c == ',') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_START;
						break;
					} else if (c == ' ') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else if (c == ')') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						SFPServer_handleParsedFunction(server);
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_INT_OCTAL:
				while (available) {
					available--;
					char c = stream->readByte();

					if ((c >= '0') && (c <= '7')) {
						server->tmpArgInt = (server->tmpArgInt << 3) + (c-'0');
					} else if (c == ',') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_START;
						break;
					} else if (c == ' ') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else if (c == ')') {
						SFPFunction_addArgument_int32(server->tmpFunction, server->tmpArgInt);
						SFPServer_handleParsedFunction(server);
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					}
				}
				break;

			case SFP_STAGE_PARAM_PARSE_STRING:
				while (available) {
					available--;
					char c = stream->readByte();

					if (c == '\\') {
						server->stage = SFP_STAGE_PARAM_PARSE_STRING_ESCAPE;
						break;
					} else if (c == '"') {
						server->stage = SFP_STAGE_PARAM_END;
						SFPServer_tmpBufferAppendByte(server, 0);
						SFPFunction_addArgument_string(server->tmpFunction, (char*)server->tmpStringBuf);
						SFPServer_tmpBufferClear(server);
						break;
					} else if ((c >= 32) && (c <= 126)) {
						SFPServer_tmpBufferAppendByte(server, c);
					} else { // Error
						server->stage = SFP_STAGE_FUNCTION_START;
						SFPServer_tmpBufferClear(server);
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_STRING_ESCAPE:
				if (available) {
					char c = stream->readByte();
					available--;

					if (c == 'x') {
						server->stage = SFP_STAGE_PARAM_PARSE_STRING_HEX;
						server->tmpArgInt = 0;
						server->tmpLenInt = 0;
					} else if ((c >= '0') && (c <= '7')) {
						server->stage = SFP_STAGE_PARAM_PARSE_STRING_OCTAL;
						server->tmpArgInt = c - '0';
						server->tmpLenInt = 1;
					} else if (c == '\\') {
						SFPServer_tmpBufferAppendByte(server, '\\');
					} else if (c == '"') {
						SFPServer_tmpBufferAppendByte(server, '"');
					} else if (c == '\'') {
						SFPServer_tmpBufferAppendByte(server, '\'');
					} else if (c == 'n') {
						SFPServer_tmpBufferAppendByte(server, '\n');
					} else if (c == 'r') {
						SFPServer_tmpBufferAppendByte(server, '\r');
					} else if (c == 'b') {
						SFPServer_tmpBufferAppendByte(server, '\b');
					} else if (c == 't') {
						SFPServer_tmpBufferAppendByte(server, '\t');
					} else if (c == 'f') {
						SFPServer_tmpBufferAppendByte(server, '\f');
					} else if (c == 'a') {
						SFPServer_tmpBufferAppendByte(server, '\a');
					} else if (c == 'v') {
						SFPServer_tmpBufferAppendByte(server, '\v');
					} else if (c == '?') {
						SFPServer_tmpBufferAppendByte(server, '?');
					} else { // Undefined escape sequence
						SFPServer_tmpBufferAppendByte(server, c);
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_STRING_HEX:
				while (available) {
					char c = stream->readByte();
					available--;

					if ((c >= '0') && (c <= '9')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'0');
						server->tmpLenInt++;
					} else if ((c >= 'a') && (c <= 'f')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'a'+10);
						server->tmpLenInt++;
					} else if ((c >= 'A') && (c <= 'F')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'A'+10);
						server->tmpLenInt++;
					} else {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_STRING;

						if (c == '\\') {
							server->stage = SFP_STAGE_PARAM_PARSE_STRING_ESCAPE;
						} else if (c == '"') {
							server->stage = SFP_STAGE_PARAM_END;
							SFPServer_tmpBufferAppendByte(server, 0);
							SFPFunction_addArgument_string(server->tmpFunction, (char*)server->tmpStringBuf);
							SFPServer_tmpBufferClear(server);
						} else if ((c >= 32) && (c <= 126)) {
							SFPServer_tmpBufferAppendByte(server, c);
						} else { // Error
							server->stage = SFP_STAGE_FUNCTION_START;
							SFPServer_tmpBufferClear(server);
						}
						break;
					}

					if (server->tmpLenInt == 2) {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_STRING;
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_STRING_OCTAL:
				while (available) {
					char c = stream->readByte();
					available--;

					if ((c >= '0') && (c <= '7')) {
						server->tmpArgInt = (server->tmpArgInt << 3) + (c-'0');
						server->tmpLenInt++;
					} else {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_STRING;

						if (c == '\\') {
							server->stage = SFP_STAGE_PARAM_PARSE_STRING_ESCAPE;
						} else if (c == '"') {
							server->stage = SFP_STAGE_PARAM_END;
							SFPServer_tmpBufferAppendByte(server, 0);
							SFPFunction_addArgument_string(server->tmpFunction, (char*)server->tmpStringBuf);
							SFPServer_tmpBufferClear(server);
						} else if ((c >= 32) && (c <= 126)) {
							SFPServer_tmpBufferAppendByte(server, c);
						} else { // Error
							server->stage = SFP_STAGE_FUNCTION_START;
							SFPServer_tmpBufferClear(server);
						}
						break;
					}

					if (server->tmpLenInt == 3) {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_STRING;
						break;
					}
				}
				break;

			case SFP_STAGE_PARAM_PARSE_ARRAY:
				while (available) {
					char c = stream->readByte();
					available--;

					if ((c >= '1') && (c <= '9')) {
						server->tmpArgInt = c - '0';
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_INT_DECIMAL;
						break;
					} else if (c == '0') {
						server->tmpArgInt = 0;
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_INT_UNKNOWN;
						break;
					} else if (c == '\'') {
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_CHAR;
						break;
					} else if (c == ']') {
						SFPFunction_addArgument_barray(server->tmpFunction, server->tmpStringBuf, server->tmpStringBufPos);
						SFPServer_tmpBufferClear(server);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else if (c == ' ') {
						// Do nothing
					} else { // Error
						server->stage = SFP_STAGE_FUNCTION_START;
						SFPServer_tmpBufferClear(server);
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_ARRAY_INT_UNKNOWN:
			{
				char c = stream->readByte();
				available--;

				if (c == 'x') {
					server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_INT_HEX;
					break;
				} else if (c == 'b') {
					server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_INT_BINARY;
					break;
				} else if ((c >= '0') && (c <= '7')) {
					server->tmpArgInt = c - '0';
					server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_INT_OCTAL;
					break;
				} else if (c == ',') {
					SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
					server->stage = SFP_STAGE_PARAM_PARSE_ARRAY;
					break;
				} else if (c == ' ') {
					SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
					server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_END;
					break;
				} else if (c == ']') {
					SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
					SFPFunction_addArgument_barray(server->tmpFunction, server->tmpStringBuf, server->tmpStringBufPos);
					SFPServer_tmpBufferClear(server);
					server->stage = SFP_STAGE_PARAM_END;
					break;
				} else {
					server->stage = SFP_STAGE_FUNCTION_START;
					SFPServer_tmpBufferClear(server);
					break;
				}
			}
				break;
			case SFP_STAGE_PARAM_PARSE_ARRAY_INT_DECIMAL:
				while (available) {
					available--;
					char c = stream->readByte();

					if ((c >= '0') && (c <= '9')) {
						server->tmpArgInt = server->tmpArgInt*10 + (c-'0');
					} else if (c == ',') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY;
						break;
					} else if (c == ' ') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_END;
						break;
					} else if (c == ']') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						SFPFunction_addArgument_barray(server->tmpFunction, server->tmpStringBuf, server->tmpStringBufPos);
						SFPServer_tmpBufferClear(server);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						SFPServer_tmpBufferClear(server);
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_ARRAY_INT_HEX:
				while (available) {
					available--;
					char c = stream->readByte();

					if ((c >= '0') && (c <= '9')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'0');
					} else if ((c >= 'a') && (c <= 'f')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'a'+10);
					} else if ((c >= 'A') && (c <= 'F')) {
						server->tmpArgInt = (server->tmpArgInt << 4) + (c-'A'+10);
					} else if (c == ',') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY;
						break;
					} else if (c == ' ') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_END;
						break;
					} else if (c == ']') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						SFPFunction_addArgument_barray(server->tmpFunction, server->tmpStringBuf, server->tmpStringBufPos);
						SFPServer_tmpBufferClear(server);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						SFPServer_tmpBufferClear(server);
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_ARRAY_INT_BINARY:
				while (available) {
					available--;
					char c = stream->readByte();

					if ((c == '0') || (c == '1')) {
						server->tmpArgInt = (server->tmpArgInt << 1) + (c-'0');
					} else if (c == ',') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY;
						break;
					} else if (c == ' ') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_END;
						break;
					} else if (c == ']') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						SFPFunction_addArgument_barray(server->tmpFunction, server->tmpStringBuf, server->tmpStringBufPos);
						SFPServer_tmpBufferClear(server);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						SFPServer_tmpBufferClear(server);
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_ARRAY_INT_OCTAL:
				while (available) {
					available--;
					char c = stream->readByte();

					if ((c >= '0') && (c <= '7')) {
						server->tmpArgInt = (server->tmpArgInt << 3) + (c-'0');
					} else if (c == ',') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY;
						break;
					} else if (c == ' ') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_END;
						break;
					} else if (c == ']') {
						SFPServer_tmpBufferAppendByte(server, server->tmpArgInt);
						SFPFunction_addArgument_barray(server->tmpFunction, server->tmpStringBuf, server->tmpStringBufPos);
						SFPServer_tmpBufferClear(server);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						SFPServer_tmpBufferClear(server);
						break;
					}
				}
				break;
			case SFP_STAGE_PARAM_PARSE_ARRAY_CHAR:
				if (available < 2) {
					return SFP_OK;
				}
				uint8_t b = stream->readByte();
				char c = stream->readByte();
				available -= 2;

				if (c != '\'' || b < ' ' || b > '~') { // Error
					server->stage = SFP_STAGE_FUNCTION_START;
					SFPServer_tmpBufferClear(server);
					return SFP_ERR_FORMAT;
				}
				SFPServer_tmpBufferAppendByte(server, b);
				server->stage = SFP_STAGE_PARAM_PARSE_ARRAY_END;

			case SFP_STAGE_PARAM_PARSE_ARRAY_END:
				while (available) {
					available--;
					char c = stream->readByte();

					if (c == ',') {
						server->stage = SFP_STAGE_PARAM_PARSE_ARRAY;
						break;
					} else if (c == ' ') {

					} else if (c == ']') {
						SFPFunction_addArgument_barray(server->tmpFunction, server->tmpStringBuf, server->tmpStringBufPos);
						SFPServer_tmpBufferClear(server);
						server->stage = SFP_STAGE_PARAM_END;
						break;
					} else { // Error
						server->stage = SFP_STAGE_FUNCTION_START;
						SFPServer_tmpBufferClear(server);
						return SFP_ERR_FORMAT;
					}
				}
				break;

			case SFP_STAGE_PARAM_END:
				while (available) {
					available--;
					char c = stream->readByte();

					if (c == ',') {
						server->stage = SFP_STAGE_PARAM_START;
						break;
					} else if (c == ' ') {
						// do nothing
					} else if (c == ')') {
						SFPServer_handleParsedFunction(server);
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					} else {
						server->stage = SFP_STAGE_FUNCTION_START;
						break;
					}
				}
				break;
		}
	}

	return SFP_OK;
}

void SFPServer_loop(SFPServer *sfpServer) {
	while (1)
		SFPServer_cycle(sfpServer);
}
