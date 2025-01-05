
#if defined(ESP32)
    #include <aLED_esp32.h>
#elif defined(CH32V003)
    #error CH32 selected!
#else
    #error Used platform is not recognised! Report this error!
#endif