/**
 * @file	SFP.h
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
 * SFP.h is the main include file for the SFP-Embedded library.
 * SFP-Embedded is Simple Function Protocol library written in C
 * for embedded systems, although it can be used in other systems
 * too.
 * It consists of three main parts:
 *
 * 1) SFPFunction - class-like SFPFunction structure and functions for creating,
 * deleting and manipulating the contents of the SFPFunction.
 *
 * 2) SFPStream - interface-like structure which should be implemented by
 * the SFP library user and passed to SFPServer for it to be able to read
 * and write SFPFunctions.
 *
 * 3) SFPServer - service-like structure and functions which initiate SFPServer,
 * read (and parse) SFPFunctions and calls appropriate handler functions.
 *
 */

#ifndef SFP_H_
#define SFP_H_

#include "SFPFunction.h"
#include "SFPStream.h"
#include "SFPServer.h"


#endif /* SFP_H_ */
