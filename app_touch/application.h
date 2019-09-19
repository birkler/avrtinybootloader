
/* HID Touchscreen report descriptor */
char usbHidReportDescriptor[]=
{
0x05, 0x01, // USAGE_PAGE (Generic Desktop)
0x09, 0x02, // USAGE (Mouse)
0xa1, 0x01, // COLLECTION (Application)
0x09, 0x01, // USAGE (Pointer)
0xa1, 0x00, // COLLECTION (Physical)
0x05, 0x09, // USAGE_PAGE (Button)
0x19, 0x01, // USAGE_MINIMUM (Button 1)
0x29, 0x03, // USAGE_MAXIMUM (Button 3)
0x15, 0x00, // LOGICAL_MINIMUM (0)
0x25, 0x01, // LOGICAL_MAXIMUM (1)
0x95, 0x03, // REPORT_COUNT (3)
0x75, 0x01, // REPORT_SIZE (1)
0x81, 0x02, // INPUT (Data,Var,Abs)
0x95, 0x01, // REPORT_COUNT (1)
0x75, 0x05, // REPORT_SIZE (5)
0x81, 0x03, // INPUT (Cnst,Var,Abs)
0x05, 0x01, // USAGE_PAGE (Generic Desktop)
0x09, 0x30, // USAGE (X)
0x09, 0x31, // USAGE (Y)
0x09, 0x00, // USAGE (Undefined)
0x15, 0x00, // LOGICAL_MINIMUM (0)
0x26, 0xFF,0x03, // LOGICAL_MAXIMUM (1024-1)
0x35, 0x00, // PHYSICAL_MINIMUM(0)
0x47, 0xFF,0xFF,0x00,0x00, // PHYSICAL_MAXIMUM(65535)
0x75, 0x10, // REPORT_SIZE (16)
0x95, 0x03, // REPORT_COUNT (3)
0x81, 0x02, // INPUT (Data,Var,Abs)
0xc0, // END_COLLECTION
0xa1, 0x01, // COLLECTION (Application)
0x05, 0x09, // USAGE_PAGE (Button)
0x19, 0x01, // USAGE_MINIMUM (Button 1)
0x29, 0x03, // USAGE_MAXIMUM (Button 3)
0x15, 0x00, // LOGICAL_MINIMUM (0)
0x25, 0x01, // LOGICAL_MAXIMUM (1)
0x95, 0x03, // REPORT_COUNT (3)
0x75, 0x01, // REPORT_SIZE (1)
0xb1, 0x02, // FEATURE (Data,Var,Abs)
0x95, 0x01, // REPORT_COUNT (1)
0x75, 0x05, // REPORT_SIZE (5)
0xb1, 0x03, // FEATURE (Cnst,Var,Abs)
0xc0, // END_COLLECTION
0xc0 // END_COLLECTION
};
