/*
  AVRUSBBoot - USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>


  License:
  The project is built with AVR USB driver by Objective Development, which is
  published under a proprietary Open Source license. To conform with this
  license, USBasp is distributed under the same license conditions. See
  documentation.

  Target.........: ATMega8 at 12 MHz
  Creation Date..: 2006-03-18
  Last change....: 2006-06-25

  To adapt the bootloader to your hardware, you have to modify the following files:
  - bootloaderconfig.h:
    Define the condition when the bootloader should be started
  - usbconfig.h:
    Define the used data line pins. You have to adapt USB_CFG_IOPORT, USB_CFG_DMINUS_BIT and 
    USB_CFG_DPLUS_BIT to your hardware. The rest should be left unchanged.

  jorgen.birkler@gmail.com

  Moved some write functions to a STATE_FLUSHED to support AVR Tiny devices better:
  - the write/ will halt the CPU so we postpone it using STATE_FLUSHED so that the USB comm is not to interrupted
  - Host program is changed to support verification by implemting a read page command in the USB.
  - Writing to page 0 will write to a backup page, when teh bootloader is jump to the application code it will restore the
    vectors to the apps vectors (except the reset vector)



 */
 
#include <avr/io.h>
#include <avr/interrupt.h> 
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/fuse.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "usbdrv.h"
#include "bootloaderconfig.h"

typedef unsigned int avr_instr_t;

//Store the applications page 0
PROGMEM avr_instr_t vectors_backup[((_VECTORS_SIZE / SPM_PAGESIZE) * SPM_PAGESIZE + SPM_PAGESIZE) / sizeof(avr_instr_t)] __attribute__((__aligned__(SPM_PAGESIZE))) = {0};

/*
 * help macros
 */
///Assert statically (compiler evaluation), stop compilation if assert fails
#define STATIC_ASSERT(expr) extern char static_assert[ (!!(expr))*2 - 1]
///Return elements in an array
#define elements_of(array) (sizeof(array) / sizeof(array[0]))


STATIC_ASSERT(sizeof(vectors_backup) >= SPM_PAGESIZE);


extern PROGMEM void* __vectors[];
extern ISR(INT0_vect);
extern unsigned char space[];
extern unsigned char last_app_addr[];


FUSES =
{
	.low = (FUSE_CKDIV8 & FUSE_CKOUT & FUSE_SUT1 & FUSE_CKSEL0),
	.high = (FUSE_SPIEN & FUSE_EESAVE & FUSE_BODLEVEL2 & FUSE_BODLEVEL0),
	.extended = (EFUSE_DEFAULT & 0xFE)
};



#define USBBOOT_FUNC_LEAVE_BOOT 1
#define USBBOOT_FUNC_WRITE_PAGE 2
#define USBBOOT_FUNC_GET_PAGESIZE 3
#define USBBOOT_FUNC_READ_PAGE 4

#define STATE_IDLE 0
#define STATE_WRITE_PAGE 1 //writing pages
#define STATE_FLUSH_PAGE 2 //flush page to flash


uchar replyBuffer[8];
uchar state = STATE_IDLE;
unsigned int page_address;
unsigned int page_offset;
unsigned int flush_count_down;

static void pageEraseAndWrite(avr_instr_t* addr)
{
	cli();
	eeprom_busy_wait();
	boot_spm_busy_wait();
	//you will not feel a thing
	boot_page_erase(addr);
	boot_spm_busy_wait();
	//burn baby burn
	boot_page_write(addr);
	boot_spm_busy_wait();
}

/**
 * Rewrites the page0 and replaces int0 vector.
 * returns 1 if any changes needs to be written. 0 if not changes
 */
uchar setupRewritePage0(avr_instr_t interrupt0VectorInstr) {
	avr_instr_t* p = 0;
	avr_instr_t* s = &vectors_backup[0];
	uchar diff = 0;
	cli();
	while (p < (avr_instr_t*)SPM_PAGESIZE) {
		unsigned int toWrite;
		unsigned int current = pgm_read_word(p);
		if (p == (avr_instr_t*)&__vectors[0]) {
			//we never modify the reset vector
			toWrite = current;
		}
		else if (p == (avr_instr_t*)&__vectors[1]) {
			//this is what we want instead of current int0 vector
			toWrite = interrupt0VectorInstr;
		}
		else {
			toWrite = pgm_read_word(s);
		}
		boot_page_fill(p,toWrite);
		if (toWrite != current) {
			diff = 1;
		}
		p++;
		s++;
	}
	return diff;
}


/**
 * Rewrites the page0 and replaces int0 vector.
 * Checks if any diffs before erasing and writing the page
 */
static void rewritePage0(avr_instr_t interrupt0VectorInstr) {
	uchar diff = 0;
	cli();
	diff = setupRewritePage0(interrupt0VectorInstr);
	//you will not feel a thing
	if (diff) {
		pageEraseAndWrite(0);
	}
}


static void enterBootloader() {
	unsigned int expected_instr_vec1 = (0xC000) | (((unsigned int)&INT0_vect - (unsigned int)&__vectors[1]) & 0x0FFF);
	//We need to rewrite the vectors to get our int0
	cli();
	if (setupRewritePage0(expected_instr_vec1)) {
		//Not the correct vector 1; we need to erase page and write
		pageEraseAndWrite(0);
	}
}

static void leaveBootloader(void) {
	unsigned int app_reset_instruction = pgm_read_word(&vectors_backup[0]);
	unsigned int app_vec1_instr = pgm_read_word(&vectors_backup[1]);

	if (!BOOTLOADER_CONDITION && (app_reset_instruction & 0xF000) == 0xC000) {
		BOOT_STATUS_LED_OFF();
		rewritePage0(app_vec1_instr);
		void (*jump_to_app)(void) = (void*)((app_reset_instruction & 0x0FFF));
		jump_to_app();
	}
}

uchar usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;

    uchar len = 0;
    
    if (rq->bRequest == USBBOOT_FUNC_LEAVE_BOOT) {
      leaveBootloader();
    }
	else if (rq->bRequest == USBBOOT_FUNC_WRITE_PAGE ||
			rq->bRequest == USBBOOT_FUNC_READ_PAGE)
	{
		page_address = rq->wValue.word; /* page address */
		page_offset = 0;

		if (state != STATE_IDLE) return 1;

		if (rq->bRequest == USBBOOT_FUNC_WRITE_PAGE) {
			state = STATE_WRITE_PAGE;
		}

		if (page_address + SPM_PAGESIZE >= (unsigned int)&last_app_addr[0]) {
			//we're not going to read or write this, its out of bounds
			state = STATE_IDLE;
		}

		if (page_address == 0) {
			//write:nope, we will not write this right now, save it to backup,
			//      later leaveBootloader will write a modified version to page0
			//read: read from backup
			page_address = (unsigned int)&vectors_backup[0];
		}
		else if (page_address == 0xFF00 && rq->bRequest == USBBOOT_FUNC_READ_PAGE){
			//Special address to read page0
			page_address = 0;
		}
		len = 0xff; /* multiple out */
	}
    else if (rq->bRequest  == USBBOOT_FUNC_GET_PAGESIZE) {
		replyBuffer[0] = SPM_PAGESIZE >> 8;
		replyBuffer[1] = SPM_PAGESIZE & 0xff;
		replyBuffer[2] = (unsigned int)&last_app_addr[0] >> 8;
		replyBuffer[3] = (unsigned int)&last_app_addr[0] & 0xff;
		len = 4;
    }

    usbMsgPtr = replyBuffer;

    return len;
}

uchar usbFunctionRead(uchar *data, uchar len)
{
	uchar i;
	for (i = 0; i < len; i+=2) {
		unsigned int val = pgm_read_word(page_address + page_offset);
		data[i] = val & 0xFF;
		data[i+1] = val >> 8;
		page_offset += 2;
		if (page_offset >= SPM_PAGESIZE) {
			return i+2;
		}
	}
	return i;
}



uchar usbFunctionWrite(uchar *data, uchar len)
{
	uchar i;

	/* check if we are in correct state */
	if (state != STATE_WRITE_PAGE) return 0;

	for (i = 0; i < len; i+=2) {
	    cli();
	    boot_page_fill(page_address + page_offset, data[i] | (data[i + 1] << 8));
	    sei();
	    page_offset += 2;

	    /* check if we are at the end of a page */
	    if (page_offset >= SPM_PAGESIZE) {
	    	state = STATE_FLUSH_PAGE;
	    	flush_count_down = 10;
			return 1;
	    }
	}
	return 0;
}

int main(void)
{
	int i;
	int counter = 0;
	wdt_disable();
    /* initialize hardware */
    BOOTLOADER_INIT;
	BOOT_STATUS_LED_OFF();

	/* jump to application if jumper is set */
	leaveBootloader();

	enterBootloader();
    //GICR = (1 << IVCE);  /* enable change of interrupt vectors */
    //GICR = (1 << IVSEL); /* move interrupts to boot flash section */
	PORTB = 0x00;
	PORTA = 0x00;
	DDRB = 0x00;
	DDRA = 0x00;

    usbDeviceDisconnect();

    for(i=0;i<40;i++){  /* 600 ms disconnect */
        _delay_ms(15);
    }
    usbDeviceConnect();

    usbInit();

	leaveBootloader();

	enterBootloader();

    sei();
    for(;;){    /* main event loop */
        usbPoll();
        if (flush_count_down>0) flush_count_down--;
        if (state == STATE_FLUSH_PAGE && flush_count_down == 0) {
			BOOT_STATUS_LED_ON();
			cli();
    		pageEraseAndWrite((avr_instr_t*)page_address);
			sei();
        	state = STATE_IDLE;
        }
        if (counter == 0) {
			BOOT_STATUS_LED_CHANGE();
        	counter = 6000;
        }
        counter--;
    }
    return 0;
}

