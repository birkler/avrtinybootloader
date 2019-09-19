
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
#include <util/delay.h>
#include <util/atomic.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define USB_GET_REPORT_IDS
#include "usbdrv.h"
#include "oddebug.h"
#include "hw.h"
#include "timers.h"
#include "usbstream.h"
//#include "uartsw.h"
#include "lfsr8.h"
#include "iir_filter.h"

/*
 * help macros
 */
#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) extern char static_assert[ (!!(expr))*2 - 1]
#endif //#ifndef STATIC_ASSERT
#define elements_of(array) (sizeof(array) / sizeof(array[0]))

STATIC_ASSERT(((int8_t)(-8))>>1 < 0);
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ----------------------------- UART interface ----------------------------- */
/* ------------------------------------------------------------------------- */
/*
static int uartsw_stream_putchar(char c, FILE *stream)
{
	//uartsw1_putc(c);
	return c;
}
static int uartsw_stream_getchar(FILE *stream)
{
	return -1;//uartsw2_getc_nowait();
}
#define uartsw_init() (void)0

FILE uartsw_stream = FDEV_SETUP_STREAM(uartsw_stream_putchar,uartsw_stream_getchar, _FDEV_SETUP_READ|_FDEV_SETUP_WRITE);

*/

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

/*
ISR(TIMER0_OVF_vect,ISR_NOBLOCK )
{
}
 */

STATIC_ASSERT(TIMER0_OVERFLOW_PERIOD_MS_X256 > 2);
STATIC_ASSERT(TIMER0_OVERFLOW_PERIOD_MS_X256 < 60000);

#define us_to_ticks(_us_) (((_us_)*F_TIMER1) / 1000000L)

STATIC_ASSERT(us_to_ticks(100) > 0);


static volatile uint16_t capture_diff;
static volatile uint16_t capture_diff_max;
static volatile uint16_t captured;


ISR(TIMER0_CAPT_vect,ISR_NOBLOCK )
{
	/*
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
*/
}



/*
ISR(TIMER1_OVF_vect,ISR_NOBLOCK)
{
	timer1_ticks_major++;
}
static uint32_t gettimer1ticks(void)
{
	uint32_t res;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		res = TCNT1;
		res |= timer1_ticks_major<<8;
	}
	return res;
}
*/


#define ReportDescriptor usbHidReportDescriptor

PROGMEM
#include "hid.h"

STATIC_ASSERT(sizeof(usbHidReportDescriptor) == USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH);

typedef struct{
	struct {
		unsigned char report_id;
		uchar   buttonMask;
	} mouse;
}report_t;


static report_t reportBuffer;


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
			default:
				break;
			}

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
	unsigned char led_sensor_blink = 0;
	filter_iir2_t filter_iir2_data = {0,0,0,0};
	q7_8_t filtered_val = 0;
	uint8_t led_sensor_last_random = 0;
	static uint16_t led_sensor_delta_time = 0;
	uint16_t led_sensor_trig_time = 0;
	uint8_t led_sensor_prev_time=0;
	uint16_t led_sensor_values[5];
	uint8_t led_sensor_nvalues = 0;
	static uint8_t led_sensor_nsample = 5;
	static uint16_t led_sensor_weigthed_sum_data[20];
	uint16_t led_sensor_weigthed_sum = 0;
	uint8_t led_sensor_weigthed_sum_idx = 0;

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
	//uartsw_init();
	sei();
	usbDeviceConnect();
	LED_RED_ON();
	LED_SENSOR_INIT();
	LED_BUTTON_OFF();
	Timer_Set(TIMER_LED_BLINK,TIMER_LED_STARTUP_TIMEOUT);
	Timer_Set(TIMER_LED_SENSOR_BLINK,2);
	Timer_Set(TIMER_LED_BUTTON_BLINK,TIMER_LED_STARTUP_TIMEOUT);
	//fprintf_P(&uartsw_stream,PSTR("B!\x10\n"));
	/*char buffer[10];
	if (strcmp_P(fgets(buffer,sizeof(buffer),&uartsw_stream),PSTR("B!\x10\n")) != 0) {
		fprintf_P(&uartsw_stream,PSTR("Fail"));
	}*/
	filtered_val = filter_iir2(&filter_iir2_data,1000);
	// main event loop
	for (;;)
	{
		//update delta time
		{
			uint8_t temp = TCNT1;
			led_sensor_delta_time += (temp - led_sensor_prev_time) & 0xFF;
			led_sensor_prev_time = temp;
		}

		//Watchdog
		wdt_reset();

		//Timers
		/////////////////////////////////////////////////////
		if (TIFR & _BV(TOV1)) {
			TIFR |= _BV(TOV1);
			Timers_DecreaseAll(TIMER1_OVERFLOW_PERIOD_MS_X256);
		}

		//usb
		/////////////////////////////////////////////////////
		usbPoll();

		//
		/////////////////////////////////////////////////////
		if (BUTTON_IS_PRESSED()) {
			LED_BUTTON_ON();
			if (!(buttonPressed)) {
				//uint16_t us = capture_diff * 1000000L / F_TIMER0;
				//uint16_t us_max = capture_diff_max * 1000000L / F_TIMER0;
				//printf_P(PSTR("stat diff=%u(%uus) max=%u(%uus) #=%u!\r\n"),capture_diff,us,capture_diff_max,us_max,captured);
				//fprintf_P(&uartsw_stream,PSTR("stat diff=%u(%uus) max=%u(%uus) #=%u!\r\n"),capture_diff,us,capture_diff_max,us_max,captured);
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

		}
		/*
		int c;
		if (0 && (c = fgetc(&uartsw_stream)) != -1) {
			printf_P(PSTR("%c"),c);
			//fprintf_P(&uartsw_stream,PSTR("%c"),c);
		}
		*/

		//stdin_str = stdin_get_string(&stdin_get_string_buffer);
		if (0 && getchar() >= 0) { //offending line...?
			//if (strcmp_P(stdin_str,PSTR("stat")) == 0 )
			{

			}
		}

		//
		///////////////////////////////////////////////////////


		//LED Timer
		/////////////////////////////////////////////////////
		if (Timer_HasExpired(TIMER_LED_BLINK)) {
			Timer_Set(TIMER_LED_BLINK,TIMER_LED_BLINK_TIMEOUT);
			//LED_RED_CHANGE();
		}
		if (Timer_HasExpired(TIMER_LED_BUTTON_BLINK)) {
			Timer_Set(TIMER_LED_BUTTON_BLINK,TIMER_LED_BLINK_TIMEOUT/2);
			//LED_BUTTON_CHANGE();
		}
		if (Timer_HasExpired(TIMER_LED_SENSOR_BLINK))
		{
			led_sensor_last_random = prng_lfsr1_1();
			if (led_sensor_last_random & 0x1) {
				LED_RED_ON();
			}
			else {
				LED_RED_OFF();
			}
			Timer_Set(TIMER_LED_SENSOR_BLINK,100);
			LED_SENSOR_ADC_SETUP();
			//LED_SENSOR_ON();
			//_delay_us(200);
			//LED_SENSOR_REVERSE_BIAS();
			//_delay_us(200);
			LED_SENSOR_INPUT();

			led_sensor_blink++;
			if (led_sensor_nvalues > 0) {
				led_sensor_weigthed_sum -= led_sensor_weigthed_sum_data[led_sensor_weigthed_sum_idx];
				led_sensor_weigthed_sum_data[led_sensor_weigthed_sum_idx] = led_sensor_values[4];
				led_sensor_weigthed_sum += led_sensor_weigthed_sum_data[led_sensor_weigthed_sum_idx];

				led_sensor_weigthed_sum_idx++;
				if (led_sensor_weigthed_sum_idx >= elements_of(led_sensor_weigthed_sum_data) ) {
					led_sensor_weigthed_sum_idx = 0;
				}


				if ((led_sensor_blink & 0xF) == 0) {
					printf("led adc[%d]:\t%05u: \t%05u,\t%05u\t%05u ticks:\t%06u\tfilt:%06d   \r",
							led_sensor_nvalues,
							led_sensor_weigthed_sum / elements_of(led_sensor_weigthed_sum_data) ,
							led_sensor_values[2],
							led_sensor_values[3],
							led_sensor_values[4],
							led_sensor_trig_time,
							filtered_val);
				}
			}

			led_sensor_delta_time = 0;
			led_sensor_trig_time = 0;
			led_sensor_nvalues = 0;
			led_sensor_nsample = 2;
		}

		//Time measurement of LED sensor discharge
		if (!LED_SENSOR_IS_HIGH()) {
			LED_BUTTON_OFF();
			if (led_sensor_trig_time == 0) {
				led_sensor_trig_time = led_sensor_delta_time;
			}
		}
		else {
			LED_BUTTON_ON();
		}


		//ADC measurement of led sensor
		{
			if (led_sensor_delta_time >= us_to_ticks(4000) * led_sensor_nsample)
			{
				LED_SENSOR_ADC_START();
				led_sensor_nsample++;
			}
			if (LED_SENSOR_ADC_IS_LED_SENSOR_CHANNEL() && LED_SENSOR_ADC_IS_COMPLETED()) {
				if (led_sensor_nvalues < elements_of(led_sensor_values)) {
					led_sensor_values[led_sensor_nvalues] = ADC;
					if (led_sensor_nvalues == 4) {
						int16_t val = led_sensor_values[led_sensor_nvalues];
						if ((led_sensor_last_random & 0x1)) {
							val = -val;
						}
						filtered_val = filter_iir2(&filter_iir2_data,val);
					}
				}
				led_sensor_nvalues++;
				LED_SENSOR_ADC_DONE();
			}
		}


		if (BOOTLOADER_CONDITION) {
			cli();
			BOOTLOADER_CONDITION_INIT();
			void (*reset_vec)(void) = 0;
			reset_vec();
		}
	}
	return 0;
}

