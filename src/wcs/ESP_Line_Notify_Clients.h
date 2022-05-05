#ifndef ESP_Line_Notify_ClientS_H
#define ESP_Line_Notify_ClientS_H

#include "./mbfs/MB_FS.h"
#include "ESP_Line_Notify_Net.h"


#if defined(ESP_LINE_NOTIFY_ENABLE_EXTERNAL_CLIENT)
#include "./wcs/custom/ESP_Line_Notify_Custom_TCP_Client.h"
#define ESP_LINE_NOTIFY_TCP_CLIENT ESP_Line_Notify_Custom_TCP_Client
#elif defined(ESP32)
#include <WiFi.h>
#include "./wcs/esp32/ESP_Line_Notify_TCP_Client.h"
#define ESP_LINE_NOTIFY_TCP_CLIENT ESP_Line_Notify_TCP_Client
#elif defined(ESP8266)
#include <Schedule.h>
#include <ets_sys.h>
#include <ESP8266WiFi.h>
#include "./wcs/esp8266/ESP_Line_Notify_TCP_Client.h"
#define ESP_LINE_NOTIFY_TCP_CLIENT ESP_Line_Notify_TCP_Client
#define FS_NO_GLOBALS
#else
#include "./wcs/custom/ESP_Line_Notify_Custom_TCP_Client.h"
#define ESP_LINE_NOTIFY_TCP_CLIENT ESP_Line_Notify_Custom_TCP_Client
#endif

#endif