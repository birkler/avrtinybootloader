
/*
 * Copyright 2007 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 * USB HID device for current monitoring and battery switch on-off
 * License: GNU GPL v2 (see License.txt) or proprietary (contact author)
 */



#define TIMER_LED_BLINK_TIMEOUT      500
#define TIMER_LED_STARTUP_TIMEOUT 2000
#define TIMER_DATA_CHANGE_TIMEOUT      500
#define TIMER_SELF_TEST_TIMEOUT 300

enum 
{
  TIMER_RELAY_USB_ON,
  TIMER_RELAY_USB_OFF,
  TIMER_RELAY_BATTERY_ON,
  TIMER_RELAY_BATTERY_OFF,
  TIMER_LED_BLINK,
  TIMER_DATA_CHANGE,
  TIMER_SELF_TEST,
  TIMER_LAST
};
