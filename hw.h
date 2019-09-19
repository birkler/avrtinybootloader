/**
 * Copyright 2007 Jorgen Birkler
 * jorgen.birkler)a(gmail.com
 * USB HID device for current monitoring and battery switch on-off
 * License: GNU GPL v2 (see License.txt) or proprietary (contact author)
 */

#ifndef __hw_h_included__
#define __hw_h_included__
#include <avr/io.h>

#define BOOTLOADER_CONDITION_INIT()   DDRA &= ~_BV(PA3);PORTA |= _BV(PA3)
#define BOOTLOADER_CONDITION (bit_is_clear(PINA,PA3))


#define LED_RED_CHANGE() PORTB ^= _BV(PB3)
#define LED_RED_ON() PORTB |= _BV(PB3)
#define LED_RED_OFF() PORTB &= ~_BV(PB3)
#define LED_RED_INIT() DDRB |= _BV(PB3);PORTB &= ~_BV(PB3);LED_RED_OFF()

#define LED_SENSOR_CHANGE() DDRA |= _BV(PA2);DDRB |= _BV(PB0);PORTB ^= _BV(PB0);PORTA &= ~_BV(PA2)
#define LED_SENSOR_ON() DDRA |= _BV(PA2);DDRB |= _BV(PB0);PORTB |= _BV(PB0); PORTA &= ~_BV(PA2)
#define LED_SENSOR_OFF() DDRA |= _BV(PA2);DDRB |= _BV(PB0);PORTB &= ~_BV(PB0); PORTA &= ~_BV(PA2)
#define LED_SENSOR_INIT() LED_SENSOR_OFF()
#define LED_SENSOR_REVERSE_BIAS()  PORTA |= _BV(PA2);DDRA |= _BV(PA2);DDRB |= _BV(PB0);PORTB &= ~_BV(PB0)
#define LED_SENSOR_INPUT() DDRA &= ~_BV(PA2);PORTA &= ~_BV(PA2)
#define LED_SENSOR_IS_HIGH() (PINA & _BV(PA2))

#define STRIP_DATA_1() PORTA |= _BV(PA6)
#define STRIP_DATA_0() PORTA &= ~_BV(PA6)
#define STRIP_CLK_HIGH() PORTA |= _BV(PA7)
#define STRIP_CLK_LOW() PORTA &= ~_BV(PA7)

//#define STRIP_CHANGE() PORTA ^= _BV(PA7) | _BV(PA6)

#define STRIP_INIT() DDRA |= _BV(PA6) | _BV(PA7);PORTB &= ~(_BV(PA6) | _BV(PA7))




//#define LED_SENSOR_CHANGE() DDRA |= _BV(PA2);DDRB |= _BV(PB0);PORTB ^= _BV(PB0);PORTA &= ~_BV(PA2)
//#define LED_SENSOR_ON() DDRA |= _BV(PA2);DDRB |= _BV(PB0);PORTB |= _BV(PB0); PORTA &= ~_BV(PA2)
//#define LED_SENSOR_OFF() DDRA |= _BV(PA2);DDRB |= _BV(PB0);PORTB &= ~_BV(PB0); PORTA &= ~_BV(PA2)
//#define LED_SENSOR_INIT() LED_SENSOR_OFF()
//#define LED_SENSOR_REVERSE_BIAS()  PORTA |= _BV(PA2);DDRA |= _BV(PA2);DDRB |= _BV(PB0);PORTB &= ~_BV(PB0)
//#define LED_SENSOR_INPUT() DDRB &= ~_BV(PB0)
//#define LED_SENSOR_IS_HIGH() (PINB & _BV(PB0))



#define LED_BUTTON_CHANGE() PORTB ^= _BV(PB1)
#define LED_BUTTON_ON() PORTB |= _BV(PB1)
#define LED_BUTTON_OFF() PORTB &= ~_BV(PB1)
#define LED_BUTTON_INIT() DDRB |= _BV(PB1);PORTB &= ~_BV(PB1);LED_BUTTON_OFF()

#define BUTTON_INIT() DDRA &= ~_BV(PA4);PORTA |= _BV(PA4)
#define BUTTON_IS_PRESSED() (!(PINA & _BV(PA4)))

#define POT_ON() PORTA |= _BV(PA0)
#define POT_OFF() PORTA &= ~_BV(PA0)
#define POT_INIT() DDRA |= _BV(PA0);DDRA &= ~_BV(PA1);PORTA &= ~_BV(PA1);POT_OFF()
#define POT_ADC_SETUP() ADCSRA = _BV(ADEN) | _BV(ADIF) | _BV(ADPS1) | _BV(ADPS0);ADCSRB = 0x00

#define POT_ADC_START() ADMUX = 0x1;ADCSRA |= _BV(ADSC)
#define POT_ADC_DONE() ADCSRA |= _BV(ADIF)

#define POT_ADC_IS_POT_CHANNEL() ((ADMUX & 0x3F) == 0x1)

#define POT_ADC_IS_COMPLETED() (!(!(ADCSRA | _BV(ADIF))))

//Channel =ADC2
//#define LED_SENSOR_ADC_CH 0x02


// Channel= (ADC2-ADC1) x 20
#define LED_SENSOR_ADC_CH 0x0E


#define LED_SENSOR_ADC_SETUP() POT_OFF();ADCSRA = _BV(ADEN) | _BV(ADIF) | _BV(ADPS1) | _BV(ADPS0);ADCSRB = 0x00

#define LED_SENSOR_ADC_START() ADMUX = _BV(REFS1) | LED_SENSOR_ADC_CH;ADCSRA |= _BV(ADSC)
#define LED_SENSOR_ADC_DONE() ADCSRA |= _BV(ADIF)

#define LED_SENSOR_ADC_IS_LED_SENSOR_CHANNEL() ((ADMUX & 0x3F) == LED_SENSOR_ADC_CH)

#define LED_SENSOR_ADC_IS_COMPLETED() (!(!(ADCSRA & _BV(ADIF))))



//Timer0
////////////////////////////////////


//16bit mode. clk/1. Input capture on. noise canceler on. Negative edge (ICES0 cleared).
#define TIMER0_INIT() {TCCR0A = _BV(TCW0)|_BV(ICEN0)|_BV(ICNC0); TCCR0B = _BV(CS00);}
#define F_TIMER0 (F_CPU / 1)
#define TIMER0_ENABLE_OVERFLOW_INTERRUPT() TIMSK |= _BV(TOIE0);
#define TIMER0_DISABLE_OVERFLOW_INTERRUPT() TIMSK &= ~_BV(TOIE0);
#define TIMER0_ENABLE_INPUTCAPT_INTERRUPT() TIMSK |= _BV(TICIE0);
#define TIMER0_DISABLE_INPUTCAPT_INTERRUPT() TIMSK &= ~_BV(TICIE0);
#define TIMER0_OVERFLOW_PERIOD_MS ((256L * 256L * 1000L) / F_CPU)
#define TIMER0_OVERFLOW_PERIOD_MS_X256 ((256L * 256L *256L) / (F_CPU / 1000L))



//Normal mode. clk/128
#define TIMER1_INIT() {TCCR1A = 0; TCCR1B = _BV(CS13); TCCR1C = 0; TCCR1D = 0; TCCR1E = 0; PLLCSR = 0; }
#define F_TIMER1 (F_CPU/128)
#define TIMER1_ENABLE_OVERFLOW_INTERRUPT() TIMSK |= _BV(TOIE1);
#define TIMER1_DISABLE_OVERFLOW_INTERRUPT() TIMSK &= ~_BV(TOIE1);
#define TIMER1_OVERFLOW_PERIOD_MS ((256L * 1000L) / F_TIMER1)
#define TIMER1_OVERFLOW_PERIOD_MS_X256 ((256L *256L) / (F_TIMER1 / 1000L))

#define UARTSW_1_TX_ENABLE 1
#define UARTSW_1_RX_ENABLE 0
#define UARTSW_2_TX_ENABLE 0
#define UARTSW_2_RX_ENABLE 1
#define UARTSW_2_INVERT 1
#define UARTSW_1_INVERT 1

#define UARTSW_1_BAUDRATE 9600
#define UARTSW_2_BAUDRATE 9600
#define UARTSW_1_RXD_PINNAME
#define UARTSW_1_RXD_PINPORT
#define UARTSW_1_TXD_PINNAME A
#define UARTSW_1_TXD_PINPORT PA6
#define UARTSW_2_RXD_PINNAME A
#define UARTSW_2_RXD_PINPORT PA6
#define UARTSW_2_TXD_PINNAME
#define UARTSW_2_TXD_PINPORT


#ifndef __ASSEMBLER__

static inline void hardwareInit(void) {
	BOOTLOADER_CONDITION_INIT();
	LED_RED_INIT();
	LED_BUTTON_INIT();
	TIMER0_INIT();
	TIMER1_INIT();
	BUTTON_INIT();
	POT_INIT();
	STRIP_INIT();
}

#endif

#define FDEV_SETUP_STREAM2(p, g, f, u) \
	{ \
		.put = p, \
		.get = g, \
		.flags = f, \
		.udata = u, \
	}

#endif //__hw_h_included__
