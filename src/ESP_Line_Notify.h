/**
 * LINE Notify Arduino Library for ESP8266 and ESP32 version 1.0.6
 * 
 * May 12, 2021
 *
 * This library provides ESP32 to perform REST API call to LINE Notify service to post the several message types.
 *
 * The library was test and work well with ESP32s based module.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
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

#include "ESP_Line_Notify_Common.h"
#include "utils/ESP_Line_Notify_Utils.h"


class ESP_Line_Notify
{

public:
    ESP_Line_Notify();
    ~ESP_Line_Notify();

    /** Send the Line Notify message.
     * 
     * @param client The LineNotiFyClient data which contains TCP client with message data.
     * @return LineNotifySendingResult type data to get the sending result.
    */
    LineNotifySendingResult send(LineNotiFyClient &client);

    /** SD card config with GPIO pins.
     * 
     * @param ss SPI Chip/Slave Select pin.
     * @param sck SPI Clock pin.
     * @param miso SPI MISO pin.
     * @param mosi SPI MOSI pin.
     * @return Boolean type status indicates the success of the operation.
    */
    bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);

private:
    ESP_Line_Notify_Utils *ut = nullptr;
    LineNotifySendingResult result;
    LineNotiFyClient *_client = nullptr;
    void setMultipartHeader(std::string &buf, std::string &boundary, esp_line_notify_multipart_header_type type, const char *imgFile);
    void getContentType(const std::string &filename, std::string &buf);
    void setMultipartBoundary(std::string &buf, std::string &boundary);
    void setHeader(LineNotiFyClient &client, std::string &buf, std::string &boundary, size_t contentLength);
    bool handleResponse(LineNotiFyClient &client);
    bool reconnect(LineNotiFyClient &client, unsigned long dataTime);
    void closeSession(LineNotiFyClient &client);
    void reportUpploadProgress(LineNotiFyClient &client, size_t total, size_t read);
};

extern ESP_Line_Notify LineNotify;

#endif