/**
 * @file	CDC.c
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

#include "CDC/CDC.h"
#include "main.h"

#include "string.h"

/* Type declarations */
typedef enum {
	STOP_BIT_1 = 0,
	STOP_BIT_1_5 = 1,
	STOP_BIT_2 = 2,
} stop_bits_t;

typedef enum {
	PARITY_NONE = 0,
	PARITY_ODD  = 1,
	PARITY_EVEN = 2,
	PARITY_MARK = 3,
	PARITY_SPACE= 4,
} parity_t;

/* Private function declarations */
void UART_Init(uint32_t baudrate, uint8_t dataBits, parity_t parity, stop_bits_t stopBits);
void UART_Close(void);
void UART_Flush(void);

uint32_t CDC_Stream_available(void);
uint32_t CDC_Stream_read(uint8_t *buf, uint32_t len);
uint8_t  CDC_Stream_readByte(void);
void	 CDC_Stream_write(uint8_t *buf, uint32_t len);
void 	 CDC_Stream_flush(void);

void USB_pin_clk_init(void);

/* Private variables */
USBD_API_T   *pUsbApi;
USBD_HANDLE_T pUsbHandle;

uint8_t tmpRxBuf[USB_HS_MAX_BULK_PACKET];
uint8_t tmpTxBuf[USB_HS_MAX_BULK_PACKET];

/* UART Bridge variables */

volatile struct {
	uint32_t baudrate;
	stop_bits_t stopBits;
	parity_t parity;
	uint8_t dataBits;
} CDC_UART_Config;

volatile uint8_t CDC_UART_txBuffer[USB_HS_MAX_BULK_PACKET];
volatile uint32_t CDC_UART_txBufferSize;
volatile uint32_t CDC_UART_txBufferSent;

#define UART_RX_BUFFER_SIZE	(USB_HS_MAX_BULK_PACKET-1)	// -1 saves us from needing to send ZLP
volatile uint8_t CDC_UART_rxBuffer[UART_RX_BUFFER_SIZE];
volatile uint16_t CDC_UART_rxReceived;

volatile uint8_t CDC_UART_rxPending;
volatile uint8_t CDC_UART_txBusy;

/* SFP variables */
#define CDC_SFP_RX_BUFFER_SIZE_N	8
#define CDC_SFP_RX_BUFFER_MASK		((1 << CDC_SFP_RX_BUFFER_SIZE_N) - 1)
volatile uint8_t  CDC_SFP_rxBuffer[1 << CDC_SFP_RX_BUFFER_SIZE_N];
volatile uint32_t CDC_SFP_rxBufferWritePos;
volatile uint32_t CDC_SFP_rxBufferReadPos;

#define CDC_SFP_rxBufferAvailable()  ((CDC_SFP_rxBufferWritePos-CDC_SFP_rxBufferReadPos) & CDC_SFP_RX_BUFFER_MASK)
#define CDC_SFP_rxBufferFree()       ((CDC_SFP_rxBufferReadPos-1-CDC_SFP_rxBufferWritePos) & CDC_SFP_RX_BUFFER_MASK)

volatile uint8_t CDC_SFP_rxPending;
volatile uint8_t CDC_SFP_txReady;

#define CDC_SFP_TX_BUFFER_SIZE_N	7
#define CDC_SFP_TX_BUFFER_MASK		((1 << CDC_SFP_TX_BUFFER_SIZE_N) - 1)
volatile uint8_t  CDC_SFP_txBuffer[1 << CDC_SFP_TX_BUFFER_SIZE_N];
volatile uint32_t CDC_SFP_txBufferWritePos;
volatile uint32_t CDC_SFP_txBufferReadPos;

#define CDC_SFP_txBufferAvailable()  ((CDC_SFP_txBufferWritePos-CDC_SFP_txBufferReadPos) & CDC_SFP_TX_BUFFER_MASK)
#define CDC_SFP_txBufferFree()       ((CDC_SFP_txBufferReadPos-1-CDC_SFP_txBufferWritePos) & CDC_SFP_TX_BUFFER_MASK)

/* End of private variables */


/* USB and ISR handlers */
#define REQ_TYPE( direction, type, recipient )  ((direction<<7)|(type<<5)|recipient)

ErrorCode_t EP0_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event) {

	USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
	static USB_SETUP_PACKET packet;
	pUsbApi->hw->ReadSetupPkt( hUsb, USB_ENDPOINT_OUT(0), (uint32_t *)&packet );

	switch (event) {
		case USB_EVT_SETUP:

			if (packet.bmRequestType.B == 0x80 // Setup Device to Host
					&& packet.bRequest == 0x06 // Get descriptor
					&& packet.wValue.WB.H == 0x06 // Get Device Qualifier Descriptor
				) {
				pUsbApi->hw->WriteEP(pUsbHandle, USB_ENDPOINT_IN(0), UPER_DeviceQualifierDescriptor, USB_DEVICE_QUALI_SIZE);
				return LPC_OK;
			}

			if (packet.bmRequestType.B == 0x80 // Setup Device to Host
					&& packet.bRequest == 0x06 // Get descriptor
					&& packet.wValue.WB.H == 0x03 // Get String Descriptor
					&& packet.wValue.WB.L == 3 // Serial number string
				) {

				uint32_t descSize = UPER_USBSerialStringDescriptor[0];
				if (descSize > packet.wLength)
					descSize = packet.wLength;

				pUsbApi->hw->WriteEP(pUsbHandle, USB_ENDPOINT_IN(0), UPER_USBSerialStringDescriptor, descSize);
				return LPC_OK;
			}


			pCtrl->EP0Data.Count = packet.wLength;   // Number of bytes to transfer

			if ( (packet.bmRequestType.B == REQ_TYPE(REQUEST_HOST_TO_DEVICE,REQUEST_CLASS,REQUEST_TO_INTERFACE) )
				  && (packet.bRequest    == 0x20 ) // SetLineCoding
				  && (packet.wValue.W    == 0 )    // Zero
				  && ((packet.wIndex.W == USB_CDC_SFP_CIF_NUM) || (packet.wIndex.W == USB_CDC_UART_CIF_NUM)) // Interface number
				) {

				pCtrl->EP0Data.pData = pCtrl->EP0Buf;
				pCtrl->EP0Data.Count = 7;
				//pUsbApi->core->DataOutStage( hUsb );
				pUsbApi->core->StatusInStage(hUsb);
				return LPC_OK;
			}

			if ( (packet.bmRequestType.B == REQ_TYPE(REQUEST_DEVICE_TO_HOST,REQUEST_CLASS,REQUEST_TO_INTERFACE) )
				  && (packet.bRequest    == 0x21 ) // GetLineCoding
				  && (packet.wValue.W    == 0 )  // Zero
				  && ((packet.wIndex.W == USB_CDC_SFP_CIF_NUM) || (packet.wIndex.W == USB_CDC_UART_CIF_NUM)) // Interface number
				) {
				uint8_t lcs[] = { 0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08 }; // Default 9600 8n1

				if (packet.wIndex.W == USB_CDC_UART_CIF_NUM) {
					lcs[0] = CDC_UART_Config.baudrate;
					lcs[1] = CDC_UART_Config.baudrate >> 8;
					lcs[2] = CDC_UART_Config.baudrate >> 16;
					lcs[3] = CDC_UART_Config.baudrate >> 24;

					lcs[4] = CDC_UART_Config.stopBits;

					lcs[5] = CDC_UART_Config.parity;

					lcs[6] = CDC_UART_Config.dataBits;
				}

				pCtrl->EP0Data.Count = 7;
				pCtrl->EP0Data.pData = (uint8_t*)&lcs;
				pUsbApi->core->DataInStage(hUsb);

				return LPC_OK;
			}

			if ( (packet.bmRequestType.B == REQ_TYPE(REQUEST_HOST_TO_DEVICE,REQUEST_CLASS,REQUEST_TO_INTERFACE) )
				  && (packet.bRequest == 0x22 ) // SetControlLineState
				  && ((packet.wIndex.W == USB_CDC_SFP_CIF_NUM) || (packet.wIndex.W == USB_CDC_UART_CIF_NUM)) // Both interfaces
				) {
				pUsbApi->core->StatusInStage(hUsb);
				return LPC_OK;
			}

			break;
		case USB_EVT_OUT:
			if (pCtrl->EP0Data.Count > 0) {
				pUsbApi->core->DataOutStage(hUsb);
			} else {
				pUsbApi->core->StatusInStage(hUsb);

				if ( (packet.bmRequestType.B == REQ_TYPE(REQUEST_HOST_TO_DEVICE,REQUEST_CLASS,REQUEST_TO_INTERFACE) )
					  && (packet.bRequest        == 0x20 ) // SetLineCoding
					  && (packet.wValue.W        == (0<<8) )    // Zero
					  && ((packet.wIndex.W == USB_CDC_UART_CIF_NUM)) // Interface number
					) {
					uint8_t *ptr = pCtrl->EP0Buf;
					uint32_t baudrate = *ptr | (*(ptr+1) << 8) | (*(ptr+2) << 16) | (*(ptr+3) << 24);
					ptr += 4;
					uint8_t stopbits = *ptr++;
					uint8_t parity = *ptr++;
					uint8_t dataBits = *ptr++;

					UART_Init(baudrate, dataBits, parity, stopbits);
				}
			}

			return LPC_OK;
			break;

		case USB_EVT_IN:
			/*if (pCtrl->EP0Data.Count > 0) {
				pUsbApi->core->DataInStage( hUsb );
				return LPC_OK;
			} else {
				pUsbApi->core->StatusOutStage( hUsb );
				return LPC_OK;
			}*/

			break;

		default:
			break;
	}

	return ERR_USBD_UNHANDLED;
}

ErrorCode_t UART_bulk_in_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event) {
	CDC_UART_txBusy = 0;
	return LPC_OK;
}

ErrorCode_t UART_bulk_out_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event) {
	switch (event) {
		case USB_EVT_OUT: {
			if (CDC_UART_txBufferSent != CDC_UART_txBufferSize) {
				CDC_UART_rxPending = 1;
				return LPC_OK;
			}

			CDC_UART_txBufferSize = pUsbApi->hw->ReadEP(hUsb, USB_CDC_UART_EP_BULK_OUT, (uint8_t*)CDC_UART_txBuffer);
			CDC_UART_txBufferSent = 0;
			CDC_UART_rxPending = 0;

			if (LPC_USART->LSR & BIT5) // THR register is empty
				LPC_USART->THR = CDC_UART_txBuffer[CDC_UART_txBufferSent++];

			return LPC_OK;
		}
		default:
			break;
	}
	return LPC_OK;
}

ErrorCode_t CDC_SFP_bulk_in_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event) {
	// TODO: add managing for 64b packets (send ZLP)
	CDC_SFP_txReady = 1;

	if (CDC_SFP_txBufferAvailable())
		CDC_Stream_flush();

	return LPC_OK;
}

ErrorCode_t CDC_SFP_bulk_out_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event) {
	switch (event) {
		case USB_EVT_OUT: {
			if (CDC_SFP_rxBufferFree() < sizeof(tmpRxBuf)) {
				CDC_SFP_rxPending = 1;
				return LPC_OK;
			}

			NVIC_DisableIRQ(USB_IRQn);

			uint32_t rxLen = pUsbApi->hw->ReadEP(pUsbHandle, USB_CDC_SFP_EP_BULK_OUT, (uint8_t*)tmpRxBuf);
			uint8_t *ptr = (uint8_t*)tmpRxBuf;
			while (rxLen--) {
				CDC_SFP_rxBuffer[CDC_SFP_rxBufferWritePos & CDC_SFP_RX_BUFFER_MASK] = *ptr++;
				CDC_SFP_rxBufferWritePos++;
			}
			CDC_SFP_rxPending = 0;

			NVIC_EnableIRQ(USB_IRQn); //  enable USB0 interrrupts

			return LPC_OK;

			break;
		}
		default:
			break;
	}
	return ERR_USBD_UNHANDLED;
}

void USB_IRQHandler(void) {
	pUsbApi->hw->ISR(pUsbHandle);
}

void UART_IRQHandler() {
	uint32_t flags = (LPC_USART->IIR >> 1) & 0x7; // parse interrupt flags

	// Combined code for efficiency? and fault handling
	if (flags == 0x3) {	// Line error
		LPC_USART->LSR;
		LPC_USART->RBR;
	} else if (flags == 0x2) { // RDA - FIFO trigger level reached (8 bytes)
		uint32_t lsr;
		uint8_t i = 7;	// Read 7 bytes (leave at least 1 byte in FIFO to trigger CTI)
		while (i--) {
			if ((lsr = LPC_USART->LSR) & 0x9E) {	// if there's any error - drop the byte
				LPC_USART->RBR;
			} else {			// else - buffer it
				CDC_UART_rxBuffer[CDC_UART_rxReceived++] = LPC_USART->RBR;

				if (CDC_UART_rxReceived == UART_RX_BUFFER_SIZE) {	// if the buffer is full - send it out
					UART_Flush();
				}
			}
		}
	} else if (flags == 0x6) {	// CTI - no bytes received in a while
		uint32_t lsr;
		while ((lsr = LPC_USART->LSR) & 0x9F) {	// while data is available
			if (lsr & 0x9E) {	// if there's any error - drop the byte
				LPC_USART->RBR;
			} else {			// else - buffer it
				CDC_UART_rxBuffer[CDC_UART_rxReceived++] = LPC_USART->RBR;

				if (CDC_UART_rxReceived == UART_RX_BUFFER_SIZE) {	// if the buffer is full - send it out
					UART_Flush();
				}
			}
		}
		UART_Flush(); // force flush any remaining bytes
	} else if (flags == 0x01) {		// THRE interrupt
		if (CDC_UART_txBufferSent < CDC_UART_txBufferSize) {
			// assuming that THR is empty at interrupt
			LPC_USART->THR = CDC_UART_txBuffer[CDC_UART_txBufferSent++];
		} else if (CDC_UART_rxPending) {
			CDC_UART_txBufferSize = pUsbApi->hw->ReadEP(pUsbHandle, USB_CDC_UART_EP_BULK_OUT, (uint8_t*)CDC_UART_txBuffer);
			CDC_UART_txBufferSent = 0;
			CDC_UART_rxPending = 0;

			//if (LPC_USART->LSR & BIT5) // THR register is empty
				LPC_USART->THR = CDC_UART_txBuffer[CDC_UART_txBufferSent++];
		}
	}
}

char inline CDC_intToBase64(uint8_t i) {
	i &= 63;

	if (i < 10) {
		return '0' + i;
	} else if (i < (10+26)) {
		return ('A'-10) + i;
	} else if (i < (10+26+26)) {
		return ('a'-10-26) + i;
	} else if (i == 62) {
		return '@';
	} else {
		return '$';
	}
}

void inline CDC_GenerateSerialDescriptor(uint32_t guid[4]) {

	uint8_t *ptr = &UPER_USBSerialStringDescriptor[2];

	uint8_t i;
	for (i=0; i<4; i++) {
		uint32_t id = guid[i];

		*(ptr+0) = CDC_intToBase64(id >> 30);
		*(ptr+2) = CDC_intToBase64(id >> 24);
		*(ptr+4) = CDC_intToBase64(id >> 18);
		*(ptr+6) = CDC_intToBase64(id >> 12);
		*(ptr+8) = CDC_intToBase64(id >> 6);
		*(ptr+10) = CDC_intToBase64(id);

		ptr += 14;
	}
}

ErrorCode_t CDC_Init(SFPStream *stream, uint32_t guid[4]) {
	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	USBD_HANDLE_T hUsb;
	ErrorCode_t ret = LPC_OK;
	uint32_t ep_indx;

	CDC_GenerateSerialDescriptor(guid);

	/* get USB API table pointer */
	pUsbApi = (USBD_API_T*) ((*(ROM **) (0x1FFF1FF8))->pUSBD);

	/* enable clocks and pinmux for usb0 */
	USB_pin_clk_init();

	/* initialize call back structures */
	memset((void*) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB_BASE;
	usb_param.mem_base = 0x10001000;
	usb_param.mem_size = 0x1000;
	usb_param.max_num_ep = 10;

	/* Initialize Descriptor pointers */
	memset((void*) &desc, 0, sizeof(USB_CORE_DESCS_T));
	desc.device_desc = (uint8_t *) &UPER_DeviceDescriptor[0];
	desc.string_desc = (uint8_t *) &UPER_StringDescriptor[0];
	desc.full_speed_desc = (uint8_t *) &UPER_ConfigDescriptor[0];
	desc.high_speed_desc = (uint8_t *) &UPER_ConfigDescriptor[0];
	//desc.device_qualifier = (uint8_t*) &VCOM_DeviceQualifier[0];

	/* USB Initialization */
	//uint32_t usbMemSize = pUsbApi->hw->GetMemSize(&usb_param);
	ret = pUsbApi->hw->Init(&hUsb, &desc, &usb_param);

	if (ret != LPC_OK)
		return ret;

	/* Initialize private data */
	pUsbHandle = hUsb;


	CDC_UART_txBufferSent = 0;
	CDC_UART_txBufferSize = 0;

	CDC_UART_txBusy = 0;
	CDC_UART_rxPending = 0;


	CDC_SFP_rxPending = 0;
	CDC_SFP_rxBufferReadPos = 0;
	CDC_SFP_rxBufferWritePos = 0;

	CDC_SFP_txReady = 1;
	CDC_SFP_txBufferReadPos = 0;
	CDC_SFP_txBufferWritePos = 0;

	/* register UART Bridge endpoint interrupt handler */
	ep_indx = (((USB_CDC_UART_EP_BULK_IN & 0x0F) << 1) + 1);
	ret = pUsbApi->core->RegisterEpHandler(hUsb, ep_indx, UART_bulk_in_hdlr, NULL);

	if (ret != LPC_OK)
		return ret;

	/* register UART Bridge endpoint interrupt handler */
	ep_indx = ((USB_CDC_UART_EP_BULK_OUT & 0x0F) << 1);
	ret = pUsbApi->core->RegisterEpHandler(hUsb, ep_indx, UART_bulk_out_hdlr, NULL);

	if (ret != LPC_OK)
		return ret;

	/* register SFP endpoint interrupt handler */
	ep_indx = (((USB_CDC_SFP_EP_BULK_IN & 0x0F) << 1) + 1);
	ret = pUsbApi->core->RegisterEpHandler(hUsb, ep_indx, CDC_SFP_bulk_in_hdlr, NULL);

	if (ret != LPC_OK)
		return ret;

	/* register SFP endpoint interrupt handler */
	ep_indx = ((USB_CDC_SFP_EP_BULK_OUT & 0x0F) << 1);
	ret = pUsbApi->core->RegisterEpHandler(hUsb, ep_indx, CDC_SFP_bulk_out_hdlr, NULL);

	if (ret != LPC_OK)
		return ret;

	/* register EP0 handler */
	ret = pUsbApi->core->RegisterClassHandler(hUsb, EP0_hdlr, NULL);
	if (ret != LPC_OK)
		return ret;

	/* enable IRQ */
	NVIC_SetPriority(USB_IRQn, 0); // give highest priority to USB
	NVIC_EnableIRQ(USB_IRQn); //  enable USB0 interrrupts

	/* USB Connect */
	pUsbApi->hw->Connect(hUsb, 1);

	//UART_Init(9600, 8, PARITY_NONE, STOP_BIT_1); // 9600 8n1

	stream->available = CDC_Stream_available;
	stream->read 	  = CDC_Stream_read;
	stream->readByte  = CDC_Stream_readByte;
	stream->write	  = CDC_Stream_write;

	return LPC_OK;
}

void USB_pin_clk_init(void) {
	/* Enable AHB clock to the GPIO domain. */
	LPC_SYSCON ->SYSAHBCLKCTRL |= (1 << 6);

	/* Enable AHB clock to the USB block and USB RAM. */
	LPC_SYSCON ->SYSAHBCLKCTRL |= ((0x1 << 14) | (0x1 << 27));

	/* Pull-down is needed, or internally, VBUS will be floating. This is to
	 address the wrong status in VBUSDebouncing bit in CmdStatus register. It
	 happens on the NXP Validation Board only that a wrong ESD protection chip is used. */
	LPC_IOCON ->PIO0_3 &= ~0x1F;
//  LPC_IOCON->PIO0_3   |= ((0x1<<3)|(0x01<<0));	/* Secondary function VBUS */
	LPC_IOCON ->PIO0_3 |= (0x01 << 0); /* Secondary function VBUS */
	LPC_IOCON ->PIO0_6 &= ~0x07;
	LPC_IOCON ->PIO0_6 |= (0x01 << 0); /* Secondary function SoftConn */

	LPC_IOCON->PIO0_18 &= ~0x07;
	LPC_IOCON->PIO0_18 |= 0x01;
	LPC_IOCON->PIO0_19 &= ~0x07;
	LPC_IOCON->PIO0_19 |= 0x01;

	return;
}

/* Part 1: Functions for UART Bridge */

void UART_Init(uint32_t baudrate, uint8_t dataBits, parity_t parity, stop_bits_t stopBits) {
	NVIC_DisableIRQ(UART_IRQn);

	if (baudrate < 46 || baudrate > 3000000)
		baudrate = CDC_UART_Config.baudrate;

	if ((stopBits != STOP_BIT_1) && (stopBits != STOP_BIT_2)) // only 1 and 2 stop bits supported
		stopBits = CDC_UART_Config.stopBits;

	if (dataBits > 8 || dataBits < 5)
		dataBits = CDC_UART_Config.dataBits;

	CDC_UART_Config.baudrate = baudrate;
	CDC_UART_Config.stopBits = stopBits;
	CDC_UART_Config.parity = parity;
	CDC_UART_Config.dataBits = dataBits;


	uint32_t roughDivider = (SystemCoreClock/16)/baudrate;

	uint32_t divider = roughDivider;
	uint8_t fracA = 0;
	uint8_t fracB = 1;

	uint32_t minFracClockDelta = (SystemCoreClock/16) - baudrate*divider;

	if ((roughDivider >> 1) < 15) { // divider/2
		uint32_t div;
		for (div=(roughDivider>>1); div<=roughDivider; div++) {
			uint32_t targetClock = div*baudrate;
			int32_t targetFracClock = (SystemCoreClock/16) - targetClock;
			if (targetFracClock < 0)
				targetFracClock = -targetFracClock;

			uint8_t tA;
			for (tA=0; tA<15; tA++) {
				uint32_t tmpFrackClock = targetClock*tA/15;
				int32_t fracClockDelta = targetFracClock - tmpFrackClock;
				if (fracClockDelta < 0)
					fracClockDelta = -fracClockDelta;

				if (fracClockDelta < minFracClockDelta) {
					divider = div;
					fracA = tA;
					fracB = 15;
					minFracClockDelta = fracClockDelta;
				}
			}
		}
	}

	int32_t tmpFracClockDelta = (SystemCoreClock/16) - baudrate*(divider+1);
	if (tmpFracClockDelta < minFracClockDelta) {
		divider = divider + 1;
		fracA = 0;
		fracB = 1;
	}


	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12); // enable AHB clock for UART
	LPC_SYSCON->UARTCLKDIV = 1; // 48MHz

	LPC_USART->LCR = (dataBits - 5) | ((stopBits == STOP_BIT_1 ? 0 : 1) << 2)
			| ((parity == PARITY_NONE ? 0 : 1) << 3) | (((parity - 1) & 0x3) << 4)
			| BIT7;

	LPC_USART->DLM = divider / 256;
	LPC_USART->DLL = divider % 256;
	LPC_USART->FDR = (fracB << 4) | (fracA);
	LPC_USART->LCR &= ~0x80;	// disable DLAB
	LPC_USART->FCR = (BIT0 | BIT1 | BIT2 | (2 << 6)); // enable and reset FIFO buffers, set RX FIFO triger level 2 (8 bytes)
	LPC_USART->IER = 0; 		// All USART interrupts disabled

	while ((LPC_USART->LSR & (BIT5 | BIT6)) != (BIT5 | BIT6)); //clear TX

	while (LPC_USART->LSR & 0x9F) { // clear RX buffer and line errors
		LPC_USART->RBR;
	}
	CDC_UART_rxReceived = 0;

	LPC_USART->IER = BIT0 | BIT1 | BIT2;	// Enable RDA(+CRT), THRE and RLS interrupts
	NVIC_SetPriority(UART_IRQn, 2);
	NVIC_EnableIRQ(UART_IRQn);
}

void UART_Close() {
	NVIC_DisableIRQ(UART_IRQn);
	LPC_USART->IER = 0;

	LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 12); // disable AHB clock for UART
}

void UART_Flush() {
	NVIC_DisableIRQ(UART_IRQn);

	if (CDC_UART_rxReceived != 0) {	// if the buffer is not empty
		while (CDC_UART_txBusy);
		CDC_UART_txBusy = 1;
		pUsbApi->hw->WriteEP(pUsbHandle, USB_CDC_UART_EP_BULK_IN, (uint8_t*)CDC_UART_rxBuffer, CDC_UART_rxReceived);
		CDC_UART_rxReceived = 0;
	}

	NVIC_EnableIRQ(UART_IRQn);
}


/* Part 2: Functions for SFP CDC port */

uint32_t CDC_Stream_available(void) {
	if (CDC_SFP_rxPending && (CDC_SFP_rxBufferFree() >= sizeof(tmpRxBuf))) {
		CDC_SFP_bulk_out_hdlr(pUsbHandle, NULL, USB_EVT_OUT);
	}

	return CDC_SFP_rxBufferAvailable();
}

uint32_t CDC_Stream_read(uint8_t *buf, uint32_t len) {
	if (len > CDC_SFP_rxBufferAvailable())
		len = CDC_SFP_rxBufferAvailable();

	uint32_t i;
	for (i=0; i<len; i++)
		buf[i] = CDC_SFP_rxBuffer[CDC_SFP_rxBufferReadPos++ & CDC_SFP_RX_BUFFER_MASK];

	return len;
}

uint8_t  CDC_Stream_readByte(void) {
	return CDC_SFP_rxBuffer[CDC_SFP_rxBufferReadPos++ & CDC_SFP_RX_BUFFER_MASK];
}

void CDC_Stream_write(uint8_t *buf, uint32_t len) {
	while (len) {
		uint32_t nWrite = CDC_SFP_txBufferFree();

		if (nWrite == 0) {
			CDC_Stream_flush();
			while ((nWrite = CDC_SFP_txBufferFree()) == 0);
		}

		if (nWrite > len)
			nWrite = len;

		len -= nWrite;

		while (nWrite--) {
			CDC_SFP_txBuffer[CDC_SFP_txBufferWritePos++ & CDC_SFP_TX_BUFFER_MASK] = *buf++;
		}
	}
	CDC_Stream_flush();
}

void CDC_Stream_flush(void) {
	if (CDC_SFP_txReady) {
		uint32_t len = CDC_SFP_txBufferAvailable();

		if (len == 0)
			return;

		if (len > 64)
			len = 64;

		CDC_SFP_txReady = 0;

		uint8_t i;
		for (i=0; i<len; i++)
			tmpTxBuf[i] = CDC_SFP_txBuffer[CDC_SFP_txBufferReadPos++ & CDC_SFP_TX_BUFFER_MASK];

		pUsbApi->hw->WriteEP(pUsbHandle, USB_CDC_SFP_EP_BULK_IN, tmpTxBuf, len);
	}
}

