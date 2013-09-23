/**
 * @file	SFPServer.h
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
 * SFPServer - service-like structure and functions which initiate SFPServer,
 * reads (and parses) SFPFunctions and calls appropriate handler functions.
 * SFPServer is created by calling SFPServer_new function. When the server is initiated,
 * function handler nodes can be added by calling SFPServer_addFunctionHandler. The given
 * handler will be automatically called when the SFPFuncion a with matching name or ID
 * is received. Handler can be added or removed (using removeFunctionHandler function)
 * at any time. SFPServer works in cycles, which should be initiated externally. Calling
 * SFPServer_cycle will perform one server cycle, while SFPServer_loop will continuously
 * perform server cycles. If at some point of program execution SFPServer is not needed
 * any more, it should be deleted using SFPServer_delete function, which will free all
 * of the allocated resources.
 *
 */

#ifndef SFPSERVER_H_
#define SFPSERVER_H_

#include "SFPFunction.h"
#include "SFPStream.h"

/**
 * \struct SFPServer is a structure which represents SFPServer instance and has all
 * private (hidden) members. SFPServer should only be referenced as a pointer.
 */
typedef struct _SFPServer SFPServer;

/**
 * \typedef SFPFunction describe the format of the SFPFunction handler function.
 */
typedef SFPResult (*SFPCallbackFunction)(SFPFunction* sfpFunction);


/**
 * Creates a new instance of SFPServer.
 *
 * @param sfpStream A pointer to initialized SFPStream structure, which will be called when reading and writing data.
 *
 * @return A pointer to a newly created SFPServer or NULL if error occurred.
 */
SFPServer*	SFPServer_new(SFPStream *sfpStream);

/**
 * Frees the resources allocated by the SFPServer.
 *
 * @param sfpServer A pointer to the SFPServer instance.
 */
void		SFPServer_delete(SFPServer *sfpServer);


/**
 * Adds function handler with specified name, ID and handler function.
 *
 * @param sfpServer A pointer to the SFPServer instance.
 *
 * @param fName A function name.
 *
 * @param fID A function ID number.
 *
 * @param func A callback handler function.
 *
 * @return SFP_OK if the node and handler were added successfully or error code.
 */
SFPResult SFPServer_addFunctionHandler(SFPServer *sfpServer, const char* fName, uint32_t fID, SFPCallbackFunction func);

/**
 * Removes function handler with specified name, ID and handler function.
 *
 * @param sfpServer A pointer to the SFPServer instance.
 *
 * @param address A function name.
 *
 * @param fID A function ID number.
 *
 * @param func A callback handler function.
 *
 * @return SFP_OK if the node and handler were removed or error code.
 */
SFPResult SFPServer_removeFunctionHandler(SFPServer *sfpServer, const char* fName, uint32_t fID, SFPCallbackFunction func);

/**
 * Sets the default function handler, which will be called when no function handler
 * is found for the received function.
 *
 * @param sfpServer A pointer to the SFPServer instance.
 *
 * @param func A callback handler function.
 *
 * @return SFP_OK.
 */
SFPResult SFPServer_setDefaultFunctionHandler(SFPServer *sfpServer, SFPCallbackFunction func);


/**
 * Performs one server cycle.
 *
 * @param sfpServer A pointer to the SFPServer instance.
 *
 * @return SFP_OK if everything was processed correctly or error code.
 *
 */
SFPResult	SFPServer_cycle(SFPServer *sfpServer);

/**
 * Performs server cycles forever.
 *
 * @param sfpServer A pointer to the SFPServer instance.
 *
 */
void		SFPServer_loop(SFPServer *sfpServer);

#endif /* SFPSERVER_H_ */
