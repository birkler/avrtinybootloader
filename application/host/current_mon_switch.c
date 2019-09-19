/*
  Flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "libs-host/hiddata.h"
#define USB_HOST_ONLY_GET_IDS
#include "../usbconfig.h"


static char *usbErrorMessage(int errCode)
{
static char buffer[80];

    switch(errCode){
        case USBOPEN_ERR_ACCESS:      return "Access to device denied";
        case USBOPEN_ERR_NOTFOUND:    return "The specified device was not found";
        case USBOPEN_ERR_IO:          return "Communication error with device";
        default:
            sprintf(buffer, "Unknown USB error %d", errCode);
            return buffer;
    }
    return NULL;    /* not reached */
}

static void usage(int argc, char **argv) {
    printf("usage: %s [usb|power]:[on|off] <delay in seconds>\n",argv[0]);
    printf("example: %s usb:on 2\n",argv[0]);
    exit(1);
}

int main(int argc, char **argv) {
	  usbDevice_t* device = 0;
	  report_id_t reportId = 0;
	  unsigned int timeout = 0;

  if (argc < 2) {
	  usage(argc,argv);
  }

  if (strcmp(argv[1],"usb:on") == 0) {
	  reportId = report_id_relay_usb_on;
  }
  else if (strcmp(argv[1],"usb:off") == 0) {
	  reportId = report_id_relay_usb_off;
  }
  else if (strcmp(argv[1],"power:on") == 0) {
	  reportId = report_id_relay_battery_on;
  }
  else if (strcmp(argv[1],"power:off") == 0) {
	  reportId = report_id_relay_battery_off;
  }
  else {
	  usage(argc,argv);
  }
  
  if (argc>2) {
	  timeout = atoi(argv[2]);
  }


  unsigned char   rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
  char            vendorName[] = {USB_CFG_VENDOR_NAME, 0}, productName[] = {USB_CFG_DEVICE_NAME, 0};
  int             vid = rawVid[0] + 256 * rawVid[1];
  int             pid = rawPid[0] + 256 * rawPid[1];
  int             err;


  int res  = usbhidOpenDevice(&device, vid, vendorName, pid, productName, 1);

  if (res == USBOPEN_SUCCESS) {
	  printf("HID Device open.\n");
  }
  else if (res == USBOPEN_ERR_NOTFOUND) {
	  printf("Current Monitor Switch HID Device not found.\n");
  }
  else {
	  printf("Open HID Device failed. %d\n",res);
  }

  unsigned char buffer[3];

  buffer[0] = report_id_battery_voltage;
  int len = sizeof(buffer);

  if((err = usbhidGetReport(device,report_id_battery_voltage, buffer, &len)) != 0)
  {
      printf("error reading data: %s\n", usbErrorMessage(err));
  }
  printf("battery voltage:%.2f\n", (double)(buffer[0] | buffer[1]<<8));

  buffer[0] = reportId;
  buffer[1] = timeout >> 8;
  buffer[2] = timeout & 0xFF;

  if((err = usbhidSetReport(device, buffer, sizeof(buffer))) != 0)
  {
      printf("error writing data: %s\n", usbErrorMessage(err));
  }

  usbhidCloseDevice(device);
  device = 0;
  return 0;
}
