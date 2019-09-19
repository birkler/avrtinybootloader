
/*
 * Copyright 2007 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 * USB HID device for current monitoring and battery switch on-off
 * License: GNU GPL v2 (see License.txt) or proprietary (contact author)
 */



#define TIMER_LED_BLINK_TIMEOUT      500
#define TIMER_LED_STARTUP_TIMEOUT 2000
#define TIMER_DATA_CHANGE_TIMEOUT      500
#define TIMER_MEASURE_POT_TIMEOUT 500
#define TIMER_SELF_TEST_TIMEOUT 300

enum 
{
  TIMER_LED_BLINK,
  TIMER_LED_SENSOR_BLINK,
  TIMER_LED_BUTTON_BLINK,
  TIMER_SEND_KEY_UP,
  TIMER_SEND_KEY_DOWN,
  TIMER_MEASURE_POT,
  TIMER_DATA_CHANGE,
  TIMER_SELF_TEST,
  TIMER_LAST
};
