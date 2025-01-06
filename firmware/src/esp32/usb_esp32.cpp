#include <Arduino.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

void usb_init(){
    Keyboard.begin();
    USB.begin();
}
void key_press(char key){
    Keyboard.press(key);
}
void key_release(char key){
    Keyboard.release(key);
}