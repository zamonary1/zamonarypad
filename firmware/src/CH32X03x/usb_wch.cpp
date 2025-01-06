#include <Arduino.h>
#include "Adafruit_TinyUSB.h"

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_hid;

void hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) report_id;
  (void) bufsize;

  // LED indicator is output report with only 1 byte length
  if (report_type != HID_REPORT_TYPE_OUTPUT) return;
}

void usb_init() {
    if (!TinyUSBDevice.isInitialized()) { TinyUSBDevice.begin(0); }
    // Setup HID
    usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setStringDescriptor("Zamonarypad mini keyboard");

    // Set up output report (on control endpoint) for Capslock indicator
    usb_hid.setReportCallback(NULL, hid_report_callback);

    usb_hid.begin();

    // If already enumerated, additional class driver begin() e.g msc, hid, midi won't take effect until re-enumeration
    if (TinyUSBDevice.mounted()) {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
}

uint8_t ascii_to_hid_key(char ascii) {
    switch (ascii) {
        case 'a': return HID_KEY_A;
        case 'b': return HID_KEY_B;
        case 'c': return HID_KEY_C;
        case 'd': return HID_KEY_D;
        case 'e': return HID_KEY_E;
        case 'f': return HID_KEY_F;
        case 'g': return HID_KEY_G;
        case 'h': return HID_KEY_H;
        case 'i': return HID_KEY_I;
        case 'j': return HID_KEY_J;
        case 'k': return HID_KEY_K;
        case 'l': return HID_KEY_L;
        case 'm': return HID_KEY_M;
        case 'n': return HID_KEY_N;
        case 'o': return HID_KEY_O;
        case 'p': return HID_KEY_P;
        case 'q': return HID_KEY_Q;
        case 'r': return HID_KEY_R;
        case 's': return HID_KEY_S;
        case 't': return HID_KEY_T;
        case 'u': return HID_KEY_U;
        case 'v': return HID_KEY_V;
        case 'w': return HID_KEY_W;
        case 'x': return HID_KEY_X;
        case 'y': return HID_KEY_Y;
        case 'z': return HID_KEY_Z;
        case '0': return HID_KEY_0;
        case '1': return HID_KEY_1;
        case '2': return HID_KEY_2;
        case '3': return HID_KEY_3;
        case '4': return HID_KEY_4;
        case '5': return HID_KEY_5;
        case '6': return HID_KEY_6;
        case '7': return HID_KEY_7;
        case '8': return HID_KEY_8;
        case '9': return HID_KEY_9;
        // Add more mappings as needed
        default: return 0;
    }
}

void key_press(char key) {
    uint8_t hid_key = ascii_to_hid_key(key);
    if (hid_key != 0) {
        uint8_t keys[2] = {0}; // Array to hold up to 2 key presses
        keys[0] = hid_key;     // Set the first key in the array
        usb_hid.keyboardReport(0,0,keys);

    }
}

void key_release(char key) {
    uint8_t hid_key = ascii_to_hid_key(key);
    if (hid_key != 0) {
        uint8_t keys[2] = {0}; // Array to hold up to 2 key presses
        // No need to set the key in the array for release
        usb_hid.keyboardReport(0,0,keys);
    }
}