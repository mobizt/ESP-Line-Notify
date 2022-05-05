/**
 * LINE Notify Arduino Library for Arduino version 2.0.0
 * 
 * Created May 5, 2022
 *
 * 
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ESP_LINE_NOTIFY_H
#define ESP_LINE_NOTIFY_H

#include "ESP_Line_Notify_Const.h"
#include "ESP_Line_Notify_Utils.h"


class ESP_Line_Notify
{

public:
    ESP_Line_Notify();
    ~ESP_Line_Notify();

    /** Send the Line Notify message.
     * 
     * @param client The LineNotifyClient data which contains TCP client with message data.
     * @return LineNotifySendingResult type data to get the sending result.
    */
    LineNotifySendingResult send(LineNotifyClient &client);

#if defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD)

    /** SD card config with GPIO pins.
     *
     * @param ss SPI Chip/Slave Select pin.
     * @param sck SPI Clock pin.
     * @param miso SPI MISO pin.
     * @param mosi SPI MOSI pin.
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);

#if defined(ESP8266)

    /** SD card config with SD FS configurations (ESP8266 only).
     *
     * @param ss SPI Chip/Slave Select pin.
     * @param sdFSConfig The pointer to SDFSConfig object (ESP8266 only).
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(SDFSConfig *sdFSConfig);

#endif

#if defined(ESP32)
    /** SD card config with chip select and SPI configuration (ESP32 only).
     *
     * @param ss SPI Chip/Slave Select pin.
     * @param spiConfig The pointer to SPIClass object for SPI configuartion (ESP32 only).
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(int8_t ss, SPIClass *spiConfig = nullptr);
#endif

#if defined(MBFS_ESP32_SDFAT_ENABLED) || defined(MBFS_SDFAT_ENABLED)
    /** SD card config with SdFat SPI and pins configurations (ESP32 with SdFat included only).
     *
     * @param sdFatSPIConfig The pointer to SdSpiConfig object for SdFat SPI configuration.
     * @param ss SPI Chip/Slave Select pin.
     * @param sck SPI Clock pin.
     * @param miso SPI MISO pin.
     * @param mosi SPI MOSI pin.
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);
#endif

#endif

#if defined(ESP32) && defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD_MMC)
    /** Initialize the SD_MMC card (ESP32 only).
     *
     * @param mountpoint The mounting point.
     * @param mode1bit Allow 1 bit data line (SPI mode).
     * @param format_if_mount_failed Format SD_MMC card if mount failed.
     * @return The boolean value indicates the success of operation.
     */
    bool sdMMCBegin(const char *mountpoint = "/sdcard", bool mode1bit = false, bool format_if_mount_failed = false);
#endif

    SPI_ETH_Module spi_ethernet_module;

private:
    ESP_Line_Notify_Utils *ut = nullptr;
    MB_FS *mbfs = nullptr;
    LineNotifySendingResult result;
    void setMultipartHeader(MB_String &buf, MB_String &boundary, esp_line_notify_multipart_header_type type, const char *imgFile);
    void getContentType(const MB_String &filename, MB_String &buf);
    void setMultipartBoundary(MB_String &buf, MB_String &boundary);
    void setHeader(LineNotifyClient &client, MB_String &buf, MB_String &boundary, size_t contentLength);
    bool handleResponse(LineNotifyClient &client, int &responseCode);
    bool reconnect(LineNotifyClient &client, unsigned long dataTime);
    void closeSession(LineNotifyClient &client);
    void reportUpploadProgress(LineNotifyClient &client, size_t total, size_t read);
    void errorToString(int httpCode, MB_String &buff);
};

extern ESP_Line_Notify LineNotify;

#endif