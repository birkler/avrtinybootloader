

const char ReportDescriptor[80] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x85, 0x01,                    //     REPORT_ID (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION
    0x0b, 0x00, 0x00, 0x01, 0x00,  // USAGE (Generic Desktop:Undefined)
    0xa1, 0x00,                    // COLLECTION (Physical)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x85, 0x7c,                    //   REPORT_ID (124)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x85, 0x7d,                    //   REPORT_ID (125)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x44,                    //   REPORT_COUNT (68)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x85, 0x7e,                    //   REPORT_ID (126)
    0xb1, 0x00,                    //   FEATURE (Data,Ary,Abs)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x95, 0x09,                    //   REPORT_COUNT (9)
    0x85, 0x7f,                    //   REPORT_ID (127)
    0xb1, 0x00,                    //   FEATURE (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};

