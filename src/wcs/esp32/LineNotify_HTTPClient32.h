/**
 * HTTP Client wrapper v1.0.0 for ESP Line Notify
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 * 
 * HTTPClient Arduino library for ESP32
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the HTTPClient for Arduino.
 * Port to ESP32 by Evandro Luis Copercini (2017), 
 * changed fingerprints to CA verification. 	
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef LINENOTIFY_HTTPClient32_H
#define LINENOTIFY_HTTPClient32_H

#ifdef ESP32

#include <Arduino.h>
#include <WiFiClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>
#include "FS_Config.h"
#include <WiFiClientSecure.h>
#if __has_include(<WiFiEspAT.h>) || __has_include(<espduino.h>)
#error WiFi UART bridge was not supported.
#endif

#if __has_include(<esp_idf_version.h>)
#include <esp_idf_version.h>
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define FLASH_FS DEFAULT_FLASH_FS
#define SD_FS DEFAULT_SD_FS
#define FORMAT_FLASH FORMAT_FLASH_IF_MOUNT_FAILED

#include "wcs/LineNotify_HTTPCode.h"

struct esp_ln_sd_config_info_t
{
  int sck = -1;
  int miso = -1;
  int mosi = -1;
  int ss = -1;
};

class ESP_LN_HTTPClient32
{

  friend class ESP_Line_Notify;
  friend class ESP_Line_Notify_Utils;

public:
  ESP_LN_HTTPClient32();
  ~ESP_LN_HTTPClient32();

  /**
   * Initialization of new http connection.
   * \param host - Host name without protocols.
   * \param port - Server's port.
   * \return True as default.
   * If no certificate string provided, use (const char*)NULL to CAcert param 
  */
  bool begin(const char *host, uint16_t port);

  /**
   *  Check the http connection status.
   * \return True if connected.
  */
  bool connected();

  /**
   * Establish http connection if header provided and send it, send payload if provided.
   * \param header - The header string (constant chars array).
   * \param payload - The payload string (constant chars array), optional.
   * \return http status code, Return zero if new http connection and header and/or payload sent
   * with no error or no header and payload provided. If obly payload provided, no new http connection was established.
  */
  int send(const char *header, const char *payload);

  /**
   * Send extra header without making new http connection (if send with header has been called)
   * \param header - The header string (constant chars array).
   * \return True if header sending success.
   * Need to call send with header first. 
  */
  bool send(const char *header);

  /**
   * Get the WiFi client pointer.
   * \return WiFi client pointer.
  */
  WiFiClient *stream(void);

  /**
   * Set insecure mode
  */
  void setInsecure();

  void stop();

  bool connect(void);
  void setCACert(const char *caCert);
  void setCACertFile(const char *caCertFile, uint8_t storageType, struct esp_ln_sd_config_info_t sd_config);

protected:
  std::unique_ptr<WiFiClientSecure> _wcs = std::unique_ptr<WiFiClientSecure>(new WiFiClientSecure());
  std::string _host = "";
  uint16_t _port = 0;
  unsigned long timeout = LINENOTIFY_DEFAULT_TCP_TIMEOUT;

  std::string _CAFile = "";
  uint8_t _CAFileStoreageType = 0;
  int _certType = -1;
  bool _clockReady = false;
};

#endif /* ESP32 */

#endif /* LINENOTIFY_HTTPClient_H */
