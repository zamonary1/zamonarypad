
#if defined(ESP32)
    #ifdef aLED
        #include <esp32/aLED_esp32.h>
    #endif
#elif defined(CH32X035)
    #warning LEDs are not yet supported on this platform!
    #ifdef aLED
        #undef aLED
    #endif
#else
    #error Used platform is not recognised! Report this error!
#endif

#if not defined(aLED)
    #define led_init()    void()
    #define led1_rgb()    void()
    #define led2_rgb()    void()
    #define led_update()  void()
#endif