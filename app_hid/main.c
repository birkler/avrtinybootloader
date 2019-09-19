
/*
 * jorgen.birkler)a(gmail.com
 */

/**
 * @mainpage
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
 * The USB device is configured as a Mouse HID device.
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
 *
 */
/**
 * \page hardware Hardware
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



typedef uint8_t rgb332_t;

/*
static inline
#define RGB888_RGB332(r,g,b)
*/

enum {
	RED,GREEN,BLUE,COLORS
};

typedef struct RGB888_ {
	uint8_t color_[COLORS];
} RGB888;

uint8_t unpack_color_red(rgb332_t rgb) {
	uint8_t base = rgb >> 5;
	base &= ~0b111;
	uint8_t res = base << 5;
	res |= base << 2;
	res |= base >> 1;
	return res;
}


uint8_t unpack_color_green(rgb332_t rgb) {
	uint8_t base = rgb >> 2;
	base &= ~0b111;
	uint8_t res = base << 5;
	res |= base << 2;
	res |= base >> 1;
	return res;
}

uint8_t unpack_color_blue(rgb332_t rgb) {
	uint8_t base = rgb;
	base &= ~0b11;
	uint8_t res = base << 6;
	res |= base << 4;
	res |= base << 2;
	res |= base << 0;
	return res;
}
#define nop() __builtin_avr_nop()  //asm volatile(" nop \n\t")


void send_strip_byte(uint8_t byteval) {
	uint8_t temp = byteval;
	for (uint8_t i=0;i<8;++i) {
		if (temp & 0b10000000) {
			STRIP_DATA_1();
		} else {
			STRIP_DATA_0();
		}
		STRIP_CLK_HIGH();
		temp <<=1;
		STRIP_CLK_LOW();
	}
}



void send_color(uint8_t r,uint8_t g,uint8_t b) {
	const uint8_t start_byte = 0xFF;
	send_strip_byte(start_byte);
	send_strip_byte(r);
	send_strip_byte(g);
	send_strip_byte(b);
}


/* ------------------------------------------------------------------------- */


RGB888 rainbow_colors[7] = {
		{{200,0,200}},
		{{200,0,0}},
		{{200,200,0}},
		{{0,200,0}},
		{{0,200,200}},
		{{0,0,200}},
		{{200,0,200}}
};


RGB888 getRainbowColor(int index) {
	//300/6 = 50 ~ 64
	const int steps = 32;


	int rainbow_index = index / steps;
	rainbow_index %= 6;
	int interpolate_index = index % steps;
	int weight_from = 64-interpolate_index;
	int weight_to = interpolate_index;

	RGB888 from = rainbow_colors[rainbow_index];
	RGB888 to = rainbow_colors[rainbow_index+1];

	RGB888 res;
	for (int c=0;c<COLORS;c++) {
		uint16_t temp = from.color_[c] * weight_from + to.color_[c] * weight_to;
		temp /= steps;
		temp /= 2;
		res.color_[c] = temp;
	}
	return res;
}





static void setColor(int numLedsOn, uint8_t r,uint8_t g,uint8_t b) {
	const int totalLeds = 400;
	send_strip_byte(0);
	send_strip_byte(0);
	send_strip_byte(0);
	send_strip_byte(0);
	send_strip_byte(0);

	send_strip_byte(0);


//	send_color(0,255,0);
//	send_color(0,0,255);
//	send_color(255,0,0);
//	send_color(0,255,255);

	for (int i=0;i<numLedsOn;i++) {
		send_color(b,g,r);
		send_color(0,0,0);
		send_color(0,0,0);
		send_color(0,0,0);
		send_color(0,0,0);
		send_color(0,0,0);
	}
	for (int i=0;i<totalLeds;i++) {
		send_color(0,0,0);
	}
	for (int i=0;i<totalLeds;i++) {
		send_strip_byte(0xFF);
	}
}


static void setColorOneLed(int numLedOn, uint8_t r,uint8_t g,uint8_t b) {
	const int totalLeds = 300;
	send_strip_byte(0);
	send_strip_byte(0);
	send_strip_byte(0);
	send_strip_byte(0);
	send_strip_byte(0);

//	send_color(0,255,0);
//	send_color(0,0,255);
//	send_color(255,0,0);
//	send_color(0,255,255);

	/*
	for (int i=0;i<numLedOn;i++) {
		send_color(b,g,r);
		send_color(0,0,0);
		send_color(0,0,0);
		send_color(0,0,0);
		send_color(0,0,0);
	}*/
	//send_color(b,g,r);
	//send_color(b,g,r);
	//send_color(b,g,r);
	//send_color(b,g,r);
	for (int i=0;i<totalLeds;i++) {
		RGB888 rgb;
		
		if (i < numLedOn) {
			rgb = getRainbowColor(i);
		} else {
			rgb.color_[0] = 0;
			rgb.color_[1] = 0;
			rgb.color_[2] = 0;
		}
		send_color(rgb.color_[RED],rgb.color_[GREEN],rgb.color_[BLUE]);
	}
	for (int i=0;i<totalLeds/2;i++) {
		send_strip_byte(0x00);
	}
}



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
	sei();
	usbDeviceConnect();
	LED_RED_ON();
	LED_SENSOR_INIT();
	LED_BUTTON_OFF();
	Timer_Set(TIMER_LED_BLINK,TIMER_LED_STARTUP_TIMEOUT);
	Timer_Set(TIMER_LED_SENSOR_BLINK,2);
	Timer_Set(TIMER_LED_BUTTON_BLINK,TIMER_LED_STARTUP_TIMEOUT);
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
				printf_P(PSTR("Button!\r\n"));
				//fprintf_P(&uartsw_stream,PSTR("stat diff=%u(%uus) max=%u(%uus) #=%u!\r\n"),capture_diff,us,capture_diff_max,us_max,captured);
			}
			buttonPressed = 1;
			buttonMask |= 0x1;
		}
		else {
			//LED_BUTTON_OFF();
			buttonMask &= ~0x1;
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

