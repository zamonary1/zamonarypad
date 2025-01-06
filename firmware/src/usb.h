#if defined(ESP32)
    #include <esp32/usb_esp32.h>
#elif defined(CH32X03)
    #include <ch32x03x/usb_wch.h>
#else
    #error Used platform is not recognised! Report this error!
#endif