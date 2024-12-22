#if ARDUINO_USB_MODE
#warning This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

#include "USB.h"
#include "USBHIDKeyboard.h"
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#define NUM_LEDS 2
USBHIDKeyboard Keyboard;

// ESP32 Touch keyboard

#define poll_data_rounding 1000.0
#define read_data_rounding 1000 // for cases when button noise is too high
//#define debug                    and buttons start doubleclicking
#define btn_1_pin T3
#define btn_2_pin T5
#define leds_pin 10

//Type of touch signal reading

//#define interrupts //deprecated, do not use
#define analogRead


//#define plotter //uncomment if you want to know poll rate
                  //note: uncompatible with desktop app

unsigned int value1;
unsigned int value2;
unsigned int button1_sensitivity;
unsigned int button2_sensitivity;
unsigned long millis_sleep_timer = 0;
unsigned int i = 1;
const unsigned short sleep_timeout = 10000;  //10 seconds
bool touch1detected = false;
bool touch2detected = false;

bool button_1_pressed = false;
bool button_2_pressed = false;

long time_leds_updated_last;

CRGB leds[NUM_LEDS];

#ifdef plotter
  static long timer_a;
  static int timer_b;
#endif

void press_button_1(){ //using these functions is keeping the load off USB
  if (!button_1_pressed){    //less identical packets sent = less waiting for mcu = more poll rate
  Keyboard.press('z');
  button_1_pressed = true;
  }
}
void release_button_1(){ //using these functions is keeping the load off USB
  if (button_1_pressed){    //less identical packets sent = less waiting for mcu = more poll rate
  Keyboard.release('z');
  button_1_pressed = false;
  }
}

void press_button_2(){ //using these functions is keeping the load off USB
  if (!button_2_pressed){    //less identical packets sent = less waiting for mcu = more poll rate
  Keyboard.press('x');
  button_2_pressed = true;
  }
}
void release_button_2(){ //using these functions is keeping the load off USB
  if (button_2_pressed){    //less identical packets sent = less waiting for mcu = more poll rate
  Keyboard.release('x');
  button_2_pressed = false;
  }
}

void handle_led(int btn1val, int button1_sensitivity, int btn2val, int button2_sensitivity){ //this function gets called every loop() cycle and controls the behavior of LEDs.

  if (time_leds_updated_last<millis()-10){ //update every 10ms

    if (btn1val>button1_sensitivity){
      leds[0] = CRGB(0, 20, 15);
    } else if (btn1val<button1_sensitivity-read_data_rounding) {
      leds[0] = CRGB(0, 0, 0);
    }

    if (btn2val>button2_sensitivity){
      leds[1] = CRGB(0, 20, 15);
    } else if (btn2val<button2_sensitivity-read_data_rounding) {
      leds[1] = CRGB(0, 0, 0);
    }
  
    FastLED.show();

    time_leds_updated_last=millis();
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);  // give me some time to bring up serial monitor




  Keyboard.begin();
  USB.begin();
  //pinMode(LED_BUILTIN, OUTPUT); board does not contain a built-in led
  EEPROM.begin(30);
  FastLED.addLeds<WS2811, leds_pin, RGB>(leds, NUM_LEDS);



/*#ifdef debug
  delay(1000);
  Serial.println("Gently touch on button 1");

  unsigned int last_button_value = touchRead(btn_1_pin);

  while (touchRead(btn_1_pin) < last_button_value + 50) delay(20);

  timer_hz = millis();

  while (millis() < timer_hz + 1000) {
    if (touchRead(btn_1_pin) > last_button_value + 50) {
      last_button_value = touchRead(btn_1_pin);
      timer_hz = millis();
      Serial.println(last_button_value / 10);
    }
  }
  last_button_value /= 10;  //divide by 10 for correct interrupts usage
  Serial.print("Calibrated value of button 1 is ");
  Serial.println(last_button_value);
  EEPROM.put(10, last_button_value);

  delay(1000);
  Serial.println("Gently touch on button 2");

  last_button_value = touchRead(btn_2_pin);

  while (touchRead(btn_2_pin) < last_button_value + 50) delay(20);

  timer_hz = millis();

  while (millis() < timer_hz + 1000) {
    if (touchRead(btn_2_pin) > last_button_value + 50) {
      last_button_value = touchRead(btn_2_pin);
      timer_hz = millis();
      Serial.println(last_button_value / 10);
    }
  }
  last_button_value /= 10;  //divide by 10 for correct interrupts usage

  Serial.print("Calibrated value of button 2 is ");
  Serial.println(last_button_value);
  EEPROM.put(18, last_button_value);

  EEPROM.commit();
#endif*/

  EEPROM.get(10, button1_sensitivity);
  EEPROM.get(18, button2_sensitivity);


  Serial.println(button1_sensitivity);
  Serial.println(button2_sensitivity);
  for (int i; i<254; i+=3){
    leds[0] = CRGB(i, 0, 0);
    leds[1] = CRGB(i, 0, 0);
    FastLED.show();
    delay(5);
  }
  for (int i = 254; i>=0; i-=3){
    leds[0] = CRGB(i, 0, 0);
    leds[1] = CRGB(i, 0, 0);
    FastLED.show();
    delay(5);
  }
  
/*#ifdef interrupts
  touchAttachInterrupt(btn_1_pin, gotTouch1, button1_sensitivity);
  touchAttachInterrupt(btn_2_pin, gotTouch2, button2_sensitivity);
#endif*/
}



void gotTouch1() {
  touch1detected = true;
}

void gotTouch2() {
  touch2detected = true;
}



void loop() {

  int btn1val = touchRead(btn_1_pin);
  int btn2val = touchRead(btn_2_pin);

  if (btn1val > button1_sensitivity) press_button_1();
  else if (btn1val < button1_sensitivity - read_data_rounding) release_button_1();

  if (btn2val > button2_sensitivity) press_button_2();
  else if (btn2val < button2_sensitivity - read_data_rounding) release_button_2();

  handle_led(btn1val, button1_sensitivity, btn2val, button2_sensitivity);


  if (Serial.available() > 0) {

    String inputString = Serial.readStringUntil('\n');  // read untill the next string

    if (inputString == "read") {
      StaticJsonDocument<100> response;
      response["button1val"] = touchRead(btn_1_pin);
      response["button2val"] = touchRead(btn_2_pin);
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }

    else if (inputString == "readmore") {
      StaticJsonDocument<100> response;
      response["button1val"] = touchRead(btn_1_pin);
      response["button2val"] = touchRead(btn_2_pin);
      response["button1sens"] = button1_sensitivity;
      response["button2sens"] = button2_sensitivity;
      response["millis"] = millis();
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }

    else if (inputString.startsWith("wrbtn1")) {
      int argument = inputString.substring(6).toInt();
      EEPROM.put(10, argument);
      EEPROM.commit();
      button1_sensitivity = argument;
/*      #ifdef interrupts
        touchDetachInterrupt(btn_1_pin);
        touchAttachInterrupt(btn_1_pin, gotTouch1, argument);
      #endif */
      StaticJsonDocument<100> response;
      response["button1val"] = touchRead(btn_1_pin);
      response["button2val"] = touchRead(btn_2_pin);
      response["status"] = "success";
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }

    else if (inputString.startsWith("wrbtn2")) {
      int argument = inputString.substring(6).toInt();
      EEPROM.put(18, argument);
      EEPROM.commit();
      button2_sensitivity = argument;
/*      #ifdef interrupts
        touchDetachInterrupt(btn_2_pin);
        touchAttachInterrupt(btn_2_pin, gotTouch2, argument);
      #endif */
      StaticJsonDocument<100> response;
      response["button1val"] = touchRead(btn_1_pin);
      response["button2val"] = touchRead(btn_2_pin);
      response["status"] = "success";
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }


    else if (inputString == "hello") {
      StaticJsonDocument<100> response;
      response["response"] = "Hello!";
      response["millis"] = millis();
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }

    else {
      // unknown command
      Serial.print("unknown: ");
      Serial.println(inputString);
    }
  }
  #ifdef plotter
    timer_b++;
    if (timer_b==200){ //data rounding
      Serial.println(String(1000.0 / (millis() - timer_a) * 200) + "hz rate");
      timer_a = millis();
      timer_b = 0;
    }
  #endif

  delayMicroseconds(500);
}


#endif
