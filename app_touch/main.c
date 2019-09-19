
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


#define ReportDescriptor usbHidReportDescriptor

PROGMEM
#include "application.h"

STATIC_ASSERT(sizeof(usbHidReportDescriptor) == USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);

/*
 *
 */
typedef struct{
    uchar   buttonMask;
    unsigned int    x;
    unsigned int    y;
    unsigned int    z;
}report_t;

static report_t reportBuffer;
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;

    /* The following requests are never used. But since they are required by
     * the specification, we implement them in this example.
     */
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            usbMsgPtr = (void *)&reportBuffer;
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}



/* ------------------------------------------------------------------------- */

int main(void)
{
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
	POT_ON();
	POT_ADC_SETUP();
	LED_RED_ON();
	LED_BUTTON_OFF();
	Timer_Set(TIMER_LED_BLINK,TIMER_LED_STARTUP_TIMEOUT);
	Timer_Set(TIMER_LED_BUTTON_BLINK,TIMER_LED_STARTUP_TIMEOUT);
	Timer_Set(TIMER_SELF_TEST,TIMER_SELF_TEST_TIMEOUT);
	Timer_Set(TIMER_MEASURE_POT,TIMER_MEASURE_POT_TIMEOUT);
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
		if (usbInterruptIsReady())
		{
			/* use last key and not current key status in order to avoid lost changes in key status. */
			//buildReport(0);
			usbSetInterrupt((uchar*)&reportBuffer, sizeof(reportBuffer));
		}

		//
		///////////////////////////////////////////////////////
		if (Timer_HasExpired(TIMER_MEASURE_POT)) {
			Timer_Set(TIMER_MEASURE_POT,TIMER_MEASURE_POT_TIMEOUT);
			POT_ADC_START();
		}

		if (POT_ADC_IS_COMPLETED()) {
			POT_ADC_DONE();
			LED_BUTTON_CHANGE();
			if (POT_ADC_IS_POT_CHANNEL()) {
				reportBuffer.x = ADC;
				reportBuffer.y = 512;
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
			//LED_BUTTON_CHANGE();
		}

		if (BUTTON_IS_PRESSED()) {
			LED_BUTTON_ON();
			reportBuffer.buttonMask |= 0x1;
		}
		else {
			reportBuffer.buttonMask &= ~0x1;
		}
	}
	return 0;
}

