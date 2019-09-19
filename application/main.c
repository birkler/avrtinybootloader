
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
 * (c) 2007 Jorgen Birkler (jorgen.birkler)a(gmail.com)
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

#include "usbdrv.h"
#include "oddebug.h"
#include "hw.h"
#include "timers.h"

/*
 * help macros
 */
#define STATIC_ASSERT(expr) extern char static_assert[ (!!(expr))*2 - 1]
#define elements_of(array) (sizeof(array) / sizeof(array[0]))




/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */


static report_id_t received_reportId;
static uchar reportBuffer[2]; /* buffer for HID reports */
static uchar idleRate; /* in 4 ms units */

ISR(TIMER0_OVF_vect)
{
}

STATIC_ASSERT(TIMER0_OVERFLOW_PERIOD_MS_X256 > 2);
STATIC_ASSERT(TIMER0_OVERFLOW_PERIOD_MS_X256 < 60000);


//FILE usb_stream;
static int usb_putchar(char c, FILE *stream);

typedef struct {
	unsigned int magic;
	unsigned long total;
	unsigned char pos;
	char data[128];
} usb_stream_buffer_t;

static usb_stream_buffer_t usb_stream_buffer;

static FILE usb_stream = FDEV_SETUP_STREAM2(usb_putchar, NULL, _FDEV_SETUP_WRITE,&usb_stream_buffer);


static int usb_putchar(char c, FILE *stream)
{
	usb_stream_buffer_t* s = fdev_get_udata(stream);
	if (c == '\n') c = '\r';
	s->data[s->pos++] = c;
	s->pos %= sizeof(s->data);
	return 0;
}


/**
 * The 2 byte report looks like this: No iit really doesn;t
 *
 * ------------------------------------------------------------------------------------------
 * | Shift 0/1 | Keyboard (0-127) | IR consumer code (0-0xFF)                             |
 * ------------------------------------------------------------------------------------------
 *
 */
#define ReportDescriptor usbHidReportDescriptor

PROGMEM
#include "application.h"

STATIC_ASSERT(sizeof(usbHidReportDescriptor) == USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);



uchar usbFunctionSetup(uchar data[8])
{
	usbRequest_t* rq = (void *)data;
	usbMsgPtr = reportBuffer;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* HID class request */
		/* class request type */
		if(rq->bRequest == USBRQ_HID_GET_REPORT)
		{
			/* wValue: ReportType (highbyte), ReportID (lowbyte) */
			received_reportId = rq->wValue.bytes[0];
			return USB_NO_MSG;
		}
		else if(rq->bRequest == USBRQ_HID_SET_REPORT)
		{
			/* wValue: ReportType (highbyte), ReportID (lowbyte) */
			received_reportId = rq->wValue.bytes[0];
			return USB_NO_MSG;
		}
		else if(rq->bRequest == USBRQ_HID_GET_IDLE)
		{
			usbMsgPtr = &idleRate;
			return 1;
		}
		else if(rq->bRequest == USBRQ_HID_SET_IDLE)
		{
			idleRate = rq->wValue.bytes[1];
		}
	}
	else
	{
		/* no vendor specific requests implemented */
	}
	return 0;
}

uchar usbFunctionRead(uchar *data, uchar len)
{
	switch (received_reportId) {
		case report_id_battery_voltage:
			break;
		case report_id_current_voltage:
			break;
		default:
			break;
	}
	return 1;
}


uchar usbFunctionWrite(uchar *data, uchar len)
{
	milliseconds_t delay = data[0] | data[1] << 8;
	switch (received_reportId) {
		case report_id_relay_battery_on:
			Timer_Set(TIMER_RELAY_BATTERY_ON,delay);
			break;
		case report_id_relay_battery_off:
			Timer_Set(TIMER_RELAY_BATTERY_OFF,delay);
			break;
		case report_id_relay_usb_on:
			Timer_Set(TIMER_RELAY_USB_ON,delay);
			break;
		case report_id_relay_usb_off:
			Timer_Set(TIMER_RELAY_USB_OFF,delay);
			break;
		default:
			break;
	}
	received_reportId = report_id_none;
	return 1;
}


/* ------------------------------------------------------------------------- */

typedef enum {
	self_test_init = 0,
	self_test_relay1_on,
	self_test_relay1_off,
	self_test_relay2_on,
	self_test_relay3_off,
	self_test_end
} self_test_state_t;

int main(void)
{
	self_test_state_t self_test_state = self_test_init;
	hardwareInit();
	stdout = &usb_stream;
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

	sei();
	usbDeviceConnect();
	LED_RED_ON();
	Timer_Set(TIMER_LED_BLINK,TIMER_LED_STARTUP_TIMEOUT);
	Timer_Set(TIMER_SELF_TEST,TIMER_SELF_TEST_TIMEOUT);
	//Timer_Reset(TIMER_SELF_TEST);
	Timer_Reset(TIMER_RELAY_USB_ON);
	Timer_Reset(TIMER_RELAY_USB_OFF);
	Timer_Reset(TIMER_RELAY_BATTERY_ON);
	Timer_Reset(TIMER_RELAY_BATTERY_OFF);
	printf_P(PSTR("Booted!\n"));
	// main event loop
	for (;;)
	{
		//Watchdog
		wdt_reset();


		//Timers
		/////////////////////////////////////////////////////
		if (TIFR & _BV(TOV0)) {
			TIFR |= _BV(TOV0);
			Timers_DecreaseAll(TIMER0_OVERFLOW_PERIOD_MS_X256);
		}

		//usb
		/////////////////////////////////////////////////////
		usbPoll();

		//USB interrupt
		/////////////////////////////////////////////////////
		if (Timer_HasExpired(TIMER_DATA_CHANGE) && usbInterruptIsReady())
		{
			Timer_Reset(TIMER_DATA_CHANGE);
			/* use last key and not current key status in order to avoid lost changes in key status. */
			//buildReport(0);
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		}

		//LED Timer
		/////////////////////////////////////////////////////
		if (Timer_HasExpired(TIMER_LED_BLINK)) {
			Timer_Set(TIMER_LED_BLINK,TIMER_LED_BLINK_TIMEOUT);
			LED_RED_CHANGE();
		}
		//Self test
		/////////////////////////////////////////////////////
		if (Timer_HasExpired(TIMER_SELF_TEST)) {
			Timer_Set(TIMER_SELF_TEST,TIMER_SELF_TEST_TIMEOUT);
			switch (self_test_state++) {
			case self_test_relay1_on:
				break;

			case self_test_relay1_off:
				break;
			case self_test_relay2_on:
				break;
			case self_test_relay3_off:
				break;
			case self_test_init:
				break;
			default:
				Timer_Reset(TIMER_SELF_TEST);
				break;
			}
		}

		//Timer
		/////////////////////////////////////////////////////
		for (int t = TIMER_RELAY_USB_ON ; t <= TIMER_RELAY_BATTERY_OFF ; t++)
		{
			if (Timer_HasExpired(t)) {
				Timer_Reset(t);
				switch (t) {
				case TIMER_RELAY_USB_ON:
					break;
				case TIMER_RELAY_USB_OFF:
					break;
				case TIMER_RELAY_BATTERY_ON:
					break;
				case TIMER_RELAY_BATTERY_OFF:
					break;
				}
			}
		}
	}
	return 0;
}

