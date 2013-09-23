/**
 * @file	SFPStream.h
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
 * SFPStream defines the interface which is used by the SFP library and
 * should be implemented by the library user to be able to send and receive
 * data.
 *
 */


#ifndef SFPSTREAM_H_
#define SFPSTREAM_H_

#include <stdlib.h>

typedef struct _SFPStream {
	uint32_t (*available)(void);					/**< Function that returns the ammount of available data */
	uint32_t (*read)(uint8_t *buf, uint32_t len);	/**< Function that reads the data into the buffer */
	uint8_t  (*readByte)(void);
	void 	 (*write)(uint8_t *buf, uint32_t len);	/**< Function that write the data into the stream */
} SFPStream;

#endif /* SFPPACKETSTREAM_H_ */
