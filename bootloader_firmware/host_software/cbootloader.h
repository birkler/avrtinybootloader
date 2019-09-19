/*
  cbootloader.cpp - part of flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <usb.h>
#include "cpage.h"

#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC  /* Obdev's free shared PID */

class CBootloader {
 public:
  CBootloader();
  ~CBootloader();
  unsigned int getPagesize();
  unsigned int getLastAddr();

  bool writePage(CPage* page);
  bool verifyPage(CPage* page, bool errorOutput = true);
  void startApplication();

 protected:
  void findBootloaderDevice();
  usb_dev_handle *usbhandle;
};

