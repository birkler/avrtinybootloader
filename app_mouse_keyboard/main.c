
/*
 * Copyright 2007 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 * USB HID device for current monitoring and battery switch on-off
 * License: GNU GPL v2 (see License.txt) or proprietary (contact author)
 */

/**
 * @mainpage
 *
 *
 * \section implementation_guide Implementation
 * - \ref software
 * - \ref hardware
 * .
 *
 * (c) 2010 Jorgen Birkler (jorgen.birkler)a(gmail.com)
 *
 * USB driver
 *
 * (c) 2006 by OBJECTIVE DEVELOPMENT Software GmbH
 */
/**
 * \page software Software
 * Uses the firmware only USB low speed driver from http://obdev.at.
 * The USB device is configured as a Remote Control HID device.
 *
 * \section tips Tips about HID development
 * General tips about HID development:
 *
 * 1. HID device class is cached by Windows; change USB_CFG_DEVICE_ID if you change USAGE_PAGE
 *    class to another. It took me several weeks to find this info. I copied the use page for the
 *    remote but it never work until I changed the USB_CFG_DEVICE_ID to another number so that the
 *    device was rediscovered by Windows.
 *
 * 2. Added usbconfig.h manually to the dependencies in the make file to all .o files.
 *    WinAVR .d files doesn't seem to work for subdirs
 *
 * Ir is received by ICP interrupt:
 * \include irrx.h
 *
 * Main loop translated the ir codes received and handles the main USB look:
 * \include main.c
 */
/**
 * \page hardware Hardware
 *
 * Schematic:
 * \image html current_mon_switch.sch.png Schematic
 *
 * Partlist:
 * \verbinclude current_mon_switch.sch..parts.txt
 *
 * Board (for protoboards):
 * \image html current_mon_switch.brd.png
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define USB_GET_REPORT_IDS
#include "usbdrv.h"
#include "oddebug.h"
#include "hw.h"
#include "timers.h"
#include "usbstream.h"
/*
 * help macros
 */
#define STATIC_ASSERT(expr) extern char static_assert[ (!!(expr))*2 - 1]
#define elements_of(array) (sizeof(array) / sizeof(array[0]))




/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

/*
ISR(TIMER0_OVF_vect,ISR_NOBLOCK )
{
}
*/
ISR(PCINT_vect,ISR_NOBLOCK )
{
}

STATIC_ASSERT(TIMER0_OVERFLOW_PERIOD_MS_X256 > 2);
STATIC_ASSERT(TIMER0_OVERFLOW_PERIOD_MS_X256 < 60000);

static volatile uint16_t capture_diff;
static volatile uint16_t capture_diff_max;
static volatile uint16_t captured;


ISR(TIMER0_CAPT_vect,ISR_NOBLOCK )
{
	uint16_t capture_time;
	uint16_t service_time;
	service_time = TCNT0L;
	service_time |= ((unsigned int)TCNT0H << 8);
	capture_time = OCR0A;
	capture_time |= ((unsigned int)OCR0B << 8);
	capture_diff = service_time - capture_time;
	if (capture_diff > capture_diff_max) {
		capture_diff_max = capture_diff;
	}
	captured++;
}




#define ReportDescriptor usbHidReportDescriptor

PROGMEM
#include "mouse_keyboard_hid.h"

STATIC_ASSERT(sizeof(usbHidReportDescriptor) == USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);

/*
 *
 */
typedef struct{
	struct {
		unsigned char report_id;
		uchar   buttonMask;
		signed char    x;
		signed char    y;
	} mouse;
	struct {
		unsigned char report_id;
		unsigned char key;
	} keyboard;
}report_t;


static report_t reportBuffer;
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */



usbMsgLen_t usbFunctionSetup2(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;

	/* The following requests are never used. But since they are required by
	 * the specification, we implement them in this example.
	 *
	 */
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
		DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			switch (rq->wValue.bytes[0]) {
			case report_id_mouse:
				usbMsgPtr = (void *)&reportBuffer.mouse;
				return sizeof(reportBuffer.mouse);

			case report_id_keyboard:
				usbMsgPtr = (void *)&reportBuffer.keyboard;
				return sizeof(reportBuffer.keyboard);

			default:
				break;
			}

		}else if(rq->bRequest == USBRQ_HID_GET_IDLE){
			usbMsgPtr = &idleRate;
			//printf_P(PSTR("GET_IDLE =%d\n"),idleRate);
			return 1;
		}else if(rq->bRequest == USBRQ_HID_SET_IDLE){
			//printf_P(PSTR("SET_IDLE =%d\n"),idleRate);
			idleRate = rq->wValue.bytes[1];
		}
	}else{
		/* no vendor specific requests implemented */
	}
	return 0;   /* default for not implemented requests: return no data back to host */
}


typedef struct {
	char buffer[64];
	uchar bufLen;
} stdin_get_string_t;

/*
static const char* stdin_get_string(stdin_get_string_t* b)
{
	int c;
	if ((c = getchar()) >= 0) {
		if (c=='\b') {
			if (b->bufLen > 0) {
				b->bufLen--;
				putchar(c); //echo back
			}
		}
		else if (c=='\n') {
			b->buffer[b->bufLen] = '\0';
			putchar(c); //echo back
			b->bufLen = 0;
			return (const char*)b->buffer;
		}
		else if (c<20) {

		}
		else {
			if (b->bufLen+1 < sizeof(b->buffer)) {
				b->buffer[b->bufLen++] = (char)c;
				putchar(c); //echo back
			}
		}
	}
	return 0;
}
*/
/* ------------------------------------------------------------------------- */

int main(void)
{
	unsigned char buttonPressed = 0;
	unsigned char buttonMask = 0;
	hardwareInit();
	stdout = &usb_out_stream;
	stdin = &usb_in_stream;
	stdin_get_string_t stdin_get_string_buffer;
	stdin_get_string_buffer.bufLen = 0;
	//const char* stdin_str;

	printf_P(PSTR("B!\n"));
	usbDeviceDisconnect();
	uchar i, j;
	j = 0;
	while (--j)
	{ /* USB Reset by device only required on Watchdog Reset */
		i = 0;
		while (--i)
			; /* delay >10ms for USB reset */
	}

	wdt_enable(WDTO_2S);
	odDebugInit();
	usbInit();
	 TIMER0_ENABLE_INPUTCAPT_INTERRUPT();
	sei();
	usbDeviceConnect();
	POT_ON();
	POT_ADC_SETUP();
	LED_RED_ON();
	LED_BUTTON_OFF();
	Timer_Set(TIMER_LED_BLINK,TIMER_LED_STARTUP_TIMEOUT);
	Timer_Set(TIMER_LED_BUTTON_BLINK,TIMER_LED_STARTUP_TIMEOUT);
	Timer_Set(TIMER_SELF_TEST,TIMER_SELF_TEST_TIMEOUT);
	Timer_Set(TIMER_MEASURE_POT,TIMER_MEASURE_POT_TIMEOUT);
	Timer_Set(TIMER_SEND_KEY_DOWN,5000);
	// main event loop
	for (;;)
	{
		//Watchdog
		wdt_reset();


		//Timers
		/////////////////////////////////////////////////////
		if (TIFR & _BV(TOV1)) {
			TIFR |= _BV(TOV1);
			Timers_DecreaseAll(TIMER0_OVERFLOW_PERIOD_MS_X256);
		}

		//usb
		/////////////////////////////////////////////////////
		usbPoll();

		//
		/////////////////////////////////////////////////////
		if (BUTTON_IS_PRESSED()) {
			LED_BUTTON_ON();
			if (!(buttonPressed)) {
				uint16_t us = capture_diff * (1000000L / F_TIMER0);
				uint16_t us_max = capture_diff_max * 1000000L / F_TIMER0;
				printf_P(PSTR("stat diff=%u(%uus) max=%u(%uus) #=%u!\n"),capture_diff,us,capture_diff_max,us_max,captured);
			}
			buttonPressed = 1;
		}
		else {
			//LED_BUTTON_OFF();
			//buttonMask &= ~0x1;
			buttonPressed = 0;
		}

		//USB interrupt
		/////////////////////////////////////////////////////
		if (usbInterruptIsReady())
		{
			if (buttonMask != reportBuffer.mouse.buttonMask) {
				reportBuffer.mouse.report_id = report_id_mouse;
				reportBuffer.mouse.buttonMask = buttonMask;
				usbSetInterrupt((uchar*)&reportBuffer.mouse, sizeof(reportBuffer.mouse));

			}
			else if (1) {

			}
			else if (Timer_HasExpired(TIMER_SEND_KEY_DOWN)) {
				Timer_Reset(TIMER_SEND_KEY_DOWN);
				Timer_Set(TIMER_SEND_KEY_UP,200);
				reportBuffer.keyboard.report_id = report_id_keyboard;
				reportBuffer.keyboard.key = 7;
				usbSetInterrupt((uchar*)&reportBuffer.keyboard, sizeof(reportBuffer.keyboard));
			}
			else if (Timer_HasExpired(TIMER_SEND_KEY_UP)) {
				Timer_Reset(TIMER_SEND_KEY_UP);
				Timer_Set(TIMER_SEND_KEY_DOWN,2000);
				reportBuffer.keyboard.report_id = report_id_keyboard;
				reportBuffer.keyboard.key = 0;
				usbSetInterrupt((uchar*)&reportBuffer.keyboard,sizeof(reportBuffer.keyboard));
			}

		}

		//stdin_str = stdin_get_string(&stdin_get_string_buffer);
		if (getchar() >= 0) {
			//if (strcmp_P(stdin_str,PSTR("stat")) == 0 )
			{

			}
		}

		//
		///////////////////////////////////////////////////////
		if (Timer_HasExpired(TIMER_MEASURE_POT)) {
			Timer_Set(TIMER_MEASURE_POT,TIMER_MEASURE_POT_TIMEOUT);
			POT_ADC_START();
		}

		if (POT_ADC_IS_COMPLETED()) {
			POT_ADC_DONE();
			//LED_BUTTON_CHANGE();
			if (POT_ADC_IS_POT_CHANNEL()) {
				reportBuffer.mouse.x = 0;
				reportBuffer.mouse.y = 0;
			}
		}

		//LED Timer
		/////////////////////////////////////////////////////
		if (Timer_HasExpired(TIMER_LED_BLINK)) {
			Timer_Set(TIMER_LED_BLINK,TIMER_LED_BLINK_TIMEOUT);
			LED_RED_CHANGE();
		}
		if (Timer_HasExpired(TIMER_LED_BUTTON_BLINK)) {
			Timer_Set(TIMER_LED_BUTTON_BLINK,TIMER_LED_BLINK_TIMEOUT/2);
			LED_BUTTON_CHANGE();
		}

		if (BOOTLOADER_CONDITION) {
			cli();
			DDRA = 0x00;
			DDRB = 0x00;
			PORTB = 0x00;
			PORTA = 0x00;
			void (*jump_to_bootloader)(void) = (void*)((0));
			jump_to_bootloader();
		}
	}
	return 0;
}

