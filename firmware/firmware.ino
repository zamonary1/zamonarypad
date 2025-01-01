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

#define eeprombtn1sens 10
#define eeprombtn2sens 18
#define eepromstartanim_colr 26 //color of boot animation (red)
#define eepromclick_colr 26+(8*4) //click indication color

#define eepromstartanim_colg eepromstartanim_colr+8 //(green)
#define eepromclick_colg eepromclick_colr+8

#define eepromstartanim_colb eepromstartanim_colr+16 //(blue)
#define eepromclick_colb eepromclick_colr+16

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

int bootanim_col_r;
int bootanim_col_g;
int bootanim_col_b;
int click_col_r;
int click_col_g;
int click_col_b;

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
      leds[0] = CRGB(click_col_r, click_col_g, click_col_b);
    } else if (btn1val<button1_sensitivity-read_data_rounding) {
      leds[0] = CRGB(0, 0, 0);
    }

    if (btn2val>button2_sensitivity){
      leds[1] = CRGB(click_col_r, click_col_g, click_col_b);
    } else if (btn2val<button2_sensitivity-read_data_rounding) {
      leds[1] = CRGB(0, 0, 0);
    }
  
    FastLED.show();

    time_leds_updated_last=millis();
  }
}

void firstLaunchInit(){

  for(uint8_t i = 0; i<3; i++){
    leds[0] = CRGB(255, 255, 255);
    leds[1] = CRGB(255, 255, 255);
    FastLED.show();
    delay(100);

    leds[0] = CRGB(0, 0, 0);
    leds[1] = CRGB(0, 0, 0);
    FastLED.show();
    delay(100);
  }

  delay(3000);

  int btn1val = touchRead(btn_1_pin);
  int btn2val = touchRead(btn_2_pin);

  EEPROM.put(eeprombtn1sens, btn1val+10000);
  EEPROM.put(eeprombtn2sens, btn2val+10000);

  EEPROM.put(eepromstartanim_colr, 254 ); //color of boot animation
  EEPROM.put(eepromstartanim_colg, 0   );
  EEPROM.put(eepromstartanim_colb, 0   );

  EEPROM.put(eepromclick_colr,     0   ); //click indication color
  EEPROM.put(eepromclick_colg,     20  );
  EEPROM.put(eepromclick_colb,     15  );
  
  EEPROM.put(0, 100);//completed initial setup, skip on next boot
  EEPROM.commit();
  leds[0] = CRGB(0, 255, 0);
  leds[1] = CRGB(0, 255, 0);
  FastLED.show();
  delay(500);

  leds[0] = CRGB(0, 0, 0);
  leds[1] = CRGB(0, 0, 0);
  FastLED.show();

}

void getEEPROMvars(){
  if (EEPROM.read(0) != 100) firstLaunchInit();

  EEPROM.get(eeprombtn1sens, button1_sensitivity);
  EEPROM.get(eeprombtn2sens, button2_sensitivity);

  EEPROM.get(eepromstartanim_colr, bootanim_col_r);
  EEPROM.get(eepromstartanim_colg, bootanim_col_g);
  EEPROM.get(eepromstartanim_colb, bootanim_col_b);

  EEPROM.get(eepromclick_colr, click_col_r);
  EEPROM.get(eepromclick_colg, click_col_g);
  EEPROM.get(eepromclick_colb, click_col_b);
}

void printStartInfo(){
  Serial.println(button1_sensitivity);
  Serial.println(button2_sensitivity);
  Serial.print(bootanim_col_r);
  Serial.print(' ');
  Serial.print(bootanim_col_g);
  Serial.print(' ');
  Serial.println(bootanim_col_b);
  Serial.print(click_col_r);
  Serial.print(' ');
  Serial.print(click_col_g);
  Serial.print(' ');
  Serial.println(click_col_b);
}

void setup() {
  Serial.begin(115200);
  Keyboard.begin();
  USB.begin();
  delay(100);  // give me some time to bring up serial monitor

  pinMode(leds_pin, OUTPUT);

  //pinMode(LED_BUILTIN, OUTPUT); board does not contain a built-in led
  EEPROM.begin(100);
  FastLED.addLeds<WS2811, leds_pin, RGB>(leds, NUM_LEDS);

  getEEPROMvars();



  for (int i; i<=255; i+=1){
    leds[0] = CRGB(bootanim_col_r * i / 255, bootanim_col_g * i / 255, bootanim_col_b * i / 255);
    leds[1] = CRGB(bootanim_col_r * i / 255, bootanim_col_g * i / 256, bootanim_col_b * i / 255);
    FastLED.show();
    delay(2);
  }

  for (int i = 255; i>=0; i-=1){
    leds[0] = CRGB(bootanim_col_r * i / 255, bootanim_col_g * i / 255, bootanim_col_b * i / 255);
    leds[1] = CRGB(bootanim_col_r * i / 255, bootanim_col_g * i / 255, bootanim_col_b * i / 255);
    FastLED.show();
    delay(2);
  }

  printStartInfo();

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
      EEPROM.put(eeprombtn1sens, argument);
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
      EEPROM.put(eeprombtn2sens, argument);
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

    else if (inputString.startsWith("wrbanim")) {
      uint8_t argument_r = inputString.substring(8, 11).toInt();
      uint8_t argument_g = inputString.substring(12, 15).toInt();
      uint8_t argument_b = inputString.substring(16, 19).toInt();
      EEPROM.put(eepromstartanim_colr, argument_r);
      EEPROM.put(eepromstartanim_colg, argument_g);
      EEPROM.put(eepromstartanim_colb, argument_b);
      EEPROM.commit();

      StaticJsonDocument<100> response;
      response["button1val"] = touchRead(btn_1_pin);
      response["button2val"] = touchRead(btn_2_pin);
      response["status"] = "success";
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }

    else if (inputString.startsWith("wrcfbck")) {
      uint8_t argument_r = inputString.substring(8, 11).toInt();
      uint8_t argument_g = inputString.substring(12, 15).toInt();
      uint8_t argument_b = inputString.substring(16, 19).toInt();
      EEPROM.put(eepromclick_colr, argument_r);
      EEPROM.put(eepromclick_colg, argument_g);
      EEPROM.put(eepromclick_colb, argument_b);
      EEPROM.commit();

      StaticJsonDocument<100> response;
      response["button1val"] = touchRead(btn_1_pin);
      response["button2val"] = touchRead(btn_2_pin);
      response["status"] = "success";
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }

    else if (inputString.startsWith("setup")) {	
      EEPROM.put(0, 255); //make it seem like it's first boot
      EEPROM.commit();
      ESP.restart();
    }

    else if (inputString == "r") {	
      ESP.restart();
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
