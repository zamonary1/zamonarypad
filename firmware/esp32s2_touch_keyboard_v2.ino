

#if ARDUINO_USB_MODE
#warning This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

#include "USB.h"
#include "USBHIDKeyboard.h"
#include <EEPROM.h>
#include <ArduinoJson.h>

USBHIDKeyboard Keyboard;

// ESP32 Touch keyboard

#define poll_data_rounding 5000.0
//#define debug
#define btn_1_pin T5
#define btn_2_pin T3



//#define plotter

unsigned int value1;
unsigned int value2;
unsigned int button1_sensitivity;
unsigned int button2_sensitivity;
unsigned long millis_sleep_timer = 0;
unsigned long millis_timeout_last_millis = 0;  //wtfs
//unsigned long timer_hz = millis();
unsigned int i = 1;
const unsigned short sleep_timeout = 10000;  //10 seconds
bool touch1detected = false;
bool touch2detected = false;



void setup() {
  Serial.begin(115200);
  //delay(1000);  // give me some time to bring up serial monitor

  Serial.println("ESP32 Touch Test");
  Keyboard.begin();
  USB.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  EEPROM.begin(30);



#ifdef debug
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
#endif

  EEPROM.get(10, button1_sensitivity);
  EEPROM.get(18, button2_sensitivity);

  Serial.println(button1_sensitivity);
  Serial.println(button2_sensitivity);

  touchAttachInterrupt(btn_1_pin, gotTouch1, button1_sensitivity);
  touchAttachInterrupt(btn_2_pin, gotTouch2, button2_sensitivity);
}

void gotTouch1() {
  touch1detected = true;
}

void gotTouch2() {
  touch2detected = true;
}



void loop() {
  /*if (i == poll_data_rounding) {
    Serial.print("Poll_rate(hz):");
    Serial.println(1000.0 / ((millis() - timer_hz) / poll_data_rounding));
    timer_hz = millis();
    i = 1;
  } else i++;*/

  for (short i; i < 10; i++) {  //we value button responsibility more than serial interface, so, we do it 10 times more

    if (touch1detected) {
      if (touchInterruptGetLastStatus(btn_1_pin)) {
        Keyboard.press('z');
      } else {
        Keyboard.release('z');
      }
      touch1detected = false;
    }
    if (touch2detected) {
      if (touchInterruptGetLastStatus(btn_2_pin)) {
        Keyboard.press('x');
      } else {
        Keyboard.release('x');
      }
      touch2detected = false;
    }

    if (touch1detected or touch2detected) digitalWrite(LED_BUILTIN, HIGH);
    else digitalWrite(LED_BUILTIN, LOW);
  }
  if (Serial.available() > 0) {

    String inputString = Serial.readStringUntil('\n');  // read untill the next string

    /*if (inputString == "readbtn1") {
      // return the value of button 1
      Serial.println(touchRead(btn_1_pin)/140);
    } 
    else if (inputString == "readbtn2") {
      Serial.println(touchRead(btn_2_pin)/140);
    } 
    else if (inputString == "readbtn1sens"){
      Serial.println(button1_sensitivity);
    }
    else if (inputString == "readbtn2sens"){
      Serial.println(button2_sensitivity);
    }*/
    if (inputString == "read") {
      StaticJsonDocument<100> response;
      response["button1val"] = touchRead(btn_1_pin) / 20.0;
      response["button2val"] = touchRead(btn_2_pin) / 20.0;
      response["button1sens"] = button1_sensitivity;
      response["button2sens"] = button2_sensitivity;
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }

    else if (inputString.startsWith("wrbtn1")) {
      int argument = inputString.substring(6).toInt();
      EEPROM.put(10, argument);
      EEPROM.commit();
      button1_sensitivity = argument;
      touchDetachInterrupt(btn_1_pin);
      touchAttachInterrupt(btn_1_pin, gotTouch1, argument);
      Serial.println("success");
    } else if (inputString.startsWith("wrbtn2")) {
      int argument = inputString.substring(6).toInt();
      EEPROM.put(18, argument);
      EEPROM.commit();
      button2_sensitivity = argument;
      touchDetachInterrupt(btn_2_pin);
      touchAttachInterrupt(btn_2_pin, gotTouch2, argument);
      Serial.println("success");
    }


    else {
      // unknown command
      Serial.print("unknown: ");
      Serial.println(inputString);
    }
  }
}


#endif
