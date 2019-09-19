/*
 * Copyright 2010
 *
 * jorgen.birkler@gmail.com
 *
 * File reserves space in flash for application code
 *
 * See LICENSE for license
 */

#include <avr/pgmspace.h>

PROGMEM unsigned char space[4096+2048-515] = {0};
PROGMEM unsigned char last_app_addr[1] = {0};
//PROGMEM unsigned char space[1] = {0};
