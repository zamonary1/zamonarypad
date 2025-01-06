
#define poll_data_rounding 1000.0
#define read_data_rounding 1000 // for cases when button noise is too high
//#define debug                    and buttons start doubleclicking
#define btn_1_pin T3
#define btn_2_pin T5
#define leds_pin 10

//Type of touch signal reading
#define analogRead

//#define plotter //uncomment if you want to know poll rate
                  //note: uncompatible with desktop app

//comment if you want to disable LED support.
#define aLED

#define eeprombtn1sens 10
#define eeprombtn2sens 18
#define eepromstartanim_colr 26 //color of boot animation (red)
#define eepromclick_colr 26+(8*4) //click indication color

#define eepromstartanim_colg eepromstartanim_colr+8 //(green)
#define eepromclick_colg eepromclick_colr+8

#define eepromstartanim_colb eepromstartanim_colr+16 //(blue)
#define eepromclick_colb eepromclick_colr+16
