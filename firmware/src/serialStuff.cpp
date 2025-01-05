#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <projectDefs.h>

void responseSerial(String inputString, uint32_t btn1val, uint32_t btn2val, uint32_t button1_sensitivity, uint32_t button2_sensitivity){
    if (inputString == "read") {
      StaticJsonDocument<100> response;
      response["button1val"] = btn1val;
      response["button2val"] = btn2val;
      String stringResponse;
      serializeJson(response, stringResponse);
      Serial.println(stringResponse);
    }

    else if (inputString == "readmore") {
      StaticJsonDocument<100> response;
      response["button1val"] = btn1val;
      response["button2val"] = btn2val;
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
        Serial.print("unknown: \'");
        Serial.print(inputString);
        Serial.println("\"");
    }
}