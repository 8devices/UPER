/**
 * @file	IAP.h
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

#ifndef IAP_H_
#define IAP_H_

#define IAP_ADDRESS 0x1FFF1FF1
unsigned param_table[5];
unsigned result_table[5];


void iap_entry(unsigned param_tab[], unsigned result_tab[]) {
	void (*iap)(unsigned[], unsigned[]);
	iap = (void (*)(unsigned[], unsigned[])) IAP_ADDRESS;
	iap(param_tab, result_tab);
}

void read_serial_number(void) {
	param_table[0] = 58; //IAP command
	iap_entry(param_table, result_table);
	if (result_table[0] == 0) {
		param_table[0] = 0;
	} else {
		param_table[0] = 0;
	}
}



#endif /* IAP_H_ */
