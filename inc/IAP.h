/**
 * @file	IAP.h
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

#ifndef IAP_H_
#define IAP_H_

#define IAP_ADDRESS 0x1FFF1FF1


void inline iap_entry(uint32_t param_tab[], uint32_t result_tab[]) {
	void (*iap)(uint32_t[], uint32_t[]);
	iap = (void (*)(uint32_t [], uint32_t[])) IAP_ADDRESS;
	iap(param_tab, result_tab);
}

void IAP_GetSerialNumber(uint32_t guid[4]) {
	uint32_t command = 58;
	uint32_t result[5];

	do {
		iap_entry(&command, result);
	} while (result[0] != 0);

	guid[0] = result[1];
	guid[1] = result[2];
	guid[2] = result[3];
	guid[3] = result[4];
}

uint32_t IAP_GetPartNumber(void) {
	uint32_t command = 54;
	uint32_t result[2];

	do {
		iap_entry(&command, result);
	} while (result[0] != 0);

	return result[1];
}

uint32_t IAP_GetBootCodeVersion(void) {
	uint32_t command = 55;
	uint32_t result[2];

	do {
		iap_entry(&command, result);
	} while (result[0] != 0);

	return result[1];
}



#endif /* IAP_H_ */
