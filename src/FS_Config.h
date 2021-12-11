
/**
 * To use other flash file systems
 * 
 * LittleFS File system
 * 
 * #include <LittleFS.h>
 * #define DEFAULT_FLASH_FS LittleFS //For LitteFS
 * 
 * 
 * FFat File system
 * 
 * #include <FFat.h>
 * #define DEFAULT_FLASH_FS FFat  //For ESP32 FFat
 * 
*/
#if defined(ESP32)
#include <SPIFFS.h>
#endif
#define DEFAULT_FLASH_FS SPIFFS

/**
 * To use SD card file systems with different hardware interface
 * e.g. SDMMC hardware bus on the ESP32
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD#faq
 * 
 * #include <SD_MMC.h>
 * #define DEFAULT_SD_FS SD_MMC //For ESP32 SDMMC
 * 
*/
#include <SD.h>
#define DEFAULT_SD_FS SD

//For ESP32, format SPIFFS or FFat if mounting failed
#define FORMAT_FLASH_IF_MOUNT_FAILED 1

#if defined(ESP32) || defined(ESP8266)
#define ESP_LINE_NOTIFY_USE_PSRAM
#endif