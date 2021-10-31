/**
 * LINE Notify Arduino Library for ESP8266 and ESP32 version 1.0.8
 * 
 * June 26, 2021
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

#ifndef ESP_LINE_NOTIFY_CPP
#define ESP_LINE_NOTIFY_CPP

#include "ESP_Line_Notify.h"

ESP_Line_Notify::ESP_Line_Notify()
{
}

ESP_Line_Notify::~ESP_Line_Notify()
{
    if (ut)
        delete ut;
}

bool ESP_Line_Notify::sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
{
    if (_client)
    {
        _client->_int.sd_config.sck = sck;
        _client->_int.sd_config.miso = miso;
        _client->_int.sd_config.mosi = mosi;
        _client->_int.sd_config.ss = ss;
    }
#if defined(ESP32)
    if (ss > -1)
    {
        SPI.begin(sck, miso, mosi, ss);
        return SD_FS.begin(ss, SPI);
    }
    else
        return SD_FS.begin();
#elif defined(ESP8266)
    if (ss > -1)
        return SD_FS.begin(ss);
    else
        return SD_FS.begin(SD_CS_PIN);
#endif
    return false;
}

LineNotifySendingResult ESP_Line_Notify::send(LineNotifyClient &client)
{
    _client = &client;
    result.status = LineNotify_Sending_None;
    result.error.code = 0;
    result.error.message.clear();
    result.quota.image.limit = 0;
    result.quota.image.remaining = 0;
    result.quota.text.limit = 0;
    result.quota.text.remaining = 0;
    result.quota.reset = 0;

    if (!reconnect(client, 0))
    {
        if (client.sendingg_callback)
            client.sendingg_callback(result);
        return result;
    }

    result.status = LineNotify_Sending_In_Progress;

    if (!ut)
        ut = new ESP_Line_Notify_Utils(client);

    if (!client.httpClient)
    {
#if defined(ESP32)
        client.httpClient = new ESP_LN_HTTPClient32();
#elif defined(ESP8266)
        client.httpClient = new ESP_LN_HTTPClient();
#endif
    }

    client.httpClient->setCACert(nullptr);

    MBSTRING host = "";
    ut->appendP(host, esp_line_notify_str_1);

    client.httpClient->begin(host.c_str(), 443);
    uint32_t contentLength = 0;
    int ret = 0;

    MBSTRING textPayload = "";
    MBSTRING header = "";
    MBSTRING multipath = "";
    MBSTRING boundary = ut->getBoundary(10);

    result.status = LineNotify_Sending_Begin;
    if (client.sendingg_callback)
        client.sendingg_callback(result);

    if (client.message.length() > 0)
    {
        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_message, "");
        textPayload += client.message;

        if (client.gmap.center.length() > 0)
        {
            ut->appendP(textPayload, esp_line_notify_str_68);
            ut->appendP(textPayload, esp_line_notify_str_61);
            textPayload += client.gmap.center;
            ut->appendP(textPayload, esp_line_notify_str_67);
            textPayload += client.gmap.map_type;
            char *tmp = ut->intStr(client.gmap.zoom);
            ut->appendP(textPayload, esp_line_notify_str_63);
            textPayload += tmp;
            ut->delP(&tmp);
        }

        ut->appendP(textPayload, esp_line_notify_str_4);
    }

    if (client.notification_disabled)
    {
        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_notification_disabled, "");
        char *tmp = ut->boolStr(true);
        textPayload += tmp;
        ut->delP(&tmp);
        ut->appendP(textPayload, esp_line_notify_str_4);
    }

    if (client.sticker.id > 0 && client.sticker.package_id > 0)
    {
        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_sticker_package_id, "");
        char *tmp = ut->intStr(client.sticker.package_id);
        textPayload += tmp;
        ut->delP(&tmp);
        ut->appendP(textPayload, esp_line_notify_str_4);

        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_sticker_id, "");
        tmp = ut->intStr(client.sticker.id);
        textPayload += tmp;
        ut->delP(&tmp);
        ut->appendP(textPayload, esp_line_notify_str_4);
    }

    if (client.image.url.length() > 0)
    {
        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_image_thumbnail, "");
        textPayload += client.image.url;
        ut->appendP(textPayload, esp_line_notify_str_4);

        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_image_full_size, "");
        textPayload += client.image.url;
        ut->appendP(textPayload, esp_line_notify_str_4);
    }

    if (client.gmap.google_api_key.length() > 0 && client.gmap.center.length() > 0)
    {
        MBSTRING url = "";
        ut->appendP(url, esp_line_notify_str_60);
        url += client.gmap.google_api_key;
        ut->appendP(url, esp_line_notify_str_62);
        url += client.gmap.center;
        char *tmp = ut->intStr(client.gmap.zoom);
        ut->appendP(url, esp_line_notify_str_63);
        url += tmp;
        ut->delP(&tmp);
        ut->appendP(url, esp_line_notify_str_64);
        url += client.gmap.map_type;
        ut->appendP(url, esp_line_notify_str_65);
        url += client.gmap.size;

        std::vector<MBSTRING> mkrs = std::vector<MBSTRING>();
        ut->splitTk(client.gmap.markers, mkrs, " ");
        for (size_t i = 0; i < mkrs.size(); i++)
        {
            ut->appendP(url, esp_line_notify_str_66);
            url += ut->url_encode(mkrs[i]);
        }

        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_image_thumbnail, "");
        textPayload += url;
        ut->appendP(textPayload, esp_line_notify_str_4);

        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_image_full_size, "");
        textPayload += url;
        ut->appendP(textPayload, esp_line_notify_str_4);
    }

    if (client.image.file.path.length() > 0)
    {
        MBSTRING fpath = "";
        if (client.image.file.path[0] != '/')
            fpath = "/";
        fpath += client.image.file.path;

        if (client.image.file.storage_type == LineNotify_Storage_Type_SD)
        {
            if (!ut->sdTest(client._int.esp_line_notify_file))
            {
                client._int.http_code = LINENOTIFY_ERROR_FILE_IO_ERROR;
                ut->appendP(result.error.message, esp_line_notify_str_55, true);

                if (client.sendingg_callback)
                    client.sendingg_callback(result);
                return result;
            }
            client._int.esp_line_notify_file = SD_FS.open(fpath.c_str(), FILE_READ);
        }
        else if (client.image.file.storage_type == LineNotify_Storage_Type_Flash)
        {
            if (!client._int.esp_line_notify_flash_rdy)
                ut->flashTest();
            client._int.esp_line_notify_file = FLASH_FS.open(fpath.c_str(), "r");
        }

        if (client._int.esp_line_notify_file)
        {
            result.file_name = client.image.file.name;
            client._int.upload_len = client._int.esp_line_notify_file.size();
            setMultipartHeader(multipath, boundary, esp_line_notify_multipart_header_type_undefined, client.image.file.name.c_str());
            contentLength = textPayload.length() + multipath.length() + client._int.esp_line_notify_file.size() + boundary.length() + 6;
        }
        else
        {
            ut->appendP(result.error.message, esp_line_notify_str_55, true);
            if (client.sendingg_callback)
                client.sendingg_callback(result);
            return result;
        }
    }

    if (client.image.data.blob && client.image.data.size > 0)
    {
        result.file_name = client.image.data.file_name;
        client._int.upload_len = client.image.data.size;
        setMultipartHeader(multipath, boundary, esp_line_notify_multipart_header_type_undefined, client.image.data.file_name.c_str());
        contentLength = textPayload.length() + multipath.length() + client.image.data.size + boundary.length() + 6;
    }

    if (client.image.file.path.length() || client.image.data.size > 0)
    {
        setHeader(client, header, boundary, contentLength);

        ret = client.httpClient->send(header.c_str(), textPayload.c_str());
        MBSTRING().swap(textPayload);
        MBSTRING().swap(header);
        if (ret < 0)
        {
            ut->appendP(result.error.message, esp_line_notify_str_56, true);
            if (client.sendingg_callback)
                client.sendingg_callback(result);
            return result;
        }

        ret = client.httpClient->send("", multipath.c_str());
        MBSTRING().swap(multipath);
        if (ret < 0)
        {
            ut->appendP(result.error.message, esp_line_notify_str_56, true);
            if (client.sendingg_callback)
                client.sendingg_callback(result);
            return result;
        }

        size_t byteRead = 0;
        result.status = LineNotify_Sending_Upload;

        if (client._int.esp_line_notify_file && client.image.file.path.length() > 0)
        {
            int available = client._int.esp_line_notify_file.available();
            int bufLen = 1024;
            uint8_t *buf = (uint8_t*)ut->newP(bufLen + 1);
            size_t read = 0;
            while (available)
            {
                delay(0);
                if (available > bufLen)
                    available = bufLen;
                byteRead += available;
                read = client._int.esp_line_notify_file.read(buf, available);
                if (client.httpClient->stream()->write(buf, read) != read)
                    break;
                reportUpploadProgress(client, client._int.upload_len, byteRead);
                available = client._int.esp_line_notify_file.available();
            }
            ut->delP(&buf);
            client._int.esp_line_notify_file.close();
        }
        else if (client.image.data.size > 0)
        {
            int len = client.image.data.size;
            int available = len;
            int bufLen = 1024;
            uint8_t *buf = (uint8_t*)ut->newP(bufLen + 1);
            size_t pos = 0;
            while (available)
            {
                delay(0);
                if (available > bufLen)
                    available = bufLen;
                byteRead += available;
                memcpy_P(buf, client.image.data.blob + pos, available);

                if (client.httpClient->stream()->write(buf, available) != (size_t)available)
                    break;
                reportUpploadProgress(client, client._int.upload_len, byteRead);
                pos += available;
                len -= available;
                available = len;
            }
            ut->delP(&buf);
        }

        textPayload.clear();

        ut->appendP(textPayload, esp_line_notify_str_4);

        setMultipartBoundary(textPayload, boundary);

        ret = client.httpClient->send("", textPayload.c_str());
        MBSTRING().swap(textPayload);
        if (ret < 0)
        {
            ut->appendP(result.error.message, esp_line_notify_str_56, true);
            if (client.sendingg_callback)
                client.sendingg_callback(result);
            return result;
        }

        ret = 0;
    }
    else
    {
        setMultipartBoundary(textPayload, boundary);

        contentLength = textPayload.length();

        setHeader(client, header, boundary, contentLength);

        ret = client.httpClient->send(header.c_str(), textPayload.c_str());
        MBSTRING().swap(textPayload);
        MBSTRING().swap(header);
    }

    if (ret == 0)
    {
        result.status = LineNotify_Sending_Success;
        client._int.http_connected = true;
        if (!handleResponse(client))
            result.status = LineNotify_Sending_Error;
    }
    else
        result.status = LineNotify_Sending_Error;

    closeSession(client);

    if (client.sendingg_callback)
        client.sendingg_callback(result);

    delete client.httpClient;
    client.httpClient = nullptr;
    return result;
}

bool ESP_Line_Notify::handleResponse(LineNotifyClient &client)
{

    if (!reconnect(client, 0))
        return false;

    WiFiClient *stream = client.httpClient->stream();

    unsigned long dataTime = millis();

    char *pChunk = nullptr;
    char *tmp = nullptr;
    char *header = nullptr;
    bool isHeader = false;

    struct esp_line_notify_server_response_data_t response;

    int chunkIdx = 0;
    int pChunkIdx = 0;
    int payloadLen = client.response_size;
    int hBufPos = 0;
    int chunkBufSize = stream->available();
    int hstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = client.response_size;

    int payloadRead = 0;

    client._int.http_code = LINENOTIFY_ERROR_HTTP_CODE_OK;

    while (client.httpClient->connected() && chunkBufSize <= 0)
    {
        if (!reconnect(client, dataTime) || stream == nullptr)
        {
            client._int.http_code = LINENOTIFY_ERROR_HTTPC_ERROR_NOT_CONNECTED;
            return false;
        }
        chunkBufSize = stream->available();
        delay(0);
    }

    int availablePayload = chunkBufSize;
    MBSTRING payload = "";

    dataTime = millis();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0 || availablePayload > 0 || payloadRead < response.contentLen)
        {
            if (!reconnect(client, dataTime))
                return false;

            chunkBufSize = stream->available();

            if (chunkBufSize <= 0 && availablePayload <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                break;

            if (chunkBufSize > 0)
            {
                chunkBufSize = defaultChunkSize;

                if (chunkIdx == 0)
                {
                    //the first chunk can be http response header
                    header = (char*)ut->newP(chunkBufSize);
                    hstate = 1;
                    int readLen = ut->readLine(stream, header, chunkBufSize);
                    int pos = 0;

                    tmp = ut->getHeader(header, esp_line_notify_str_36, esp_line_notify_str_2, pos, 0);
                    delay(0);
                    dataTime = millis();
                    if (tmp)
                    {
                        //http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(tmp);
                        client._int.http_code = response.httpCode;
                        ut->delP(&tmp);
                    }
                }
                else
                {
                    delay(0);
                    dataTime = millis();
                    //the next chunk data can be the remaining http header
                    if (isHeader)
                    {
                        //read one line of next header field until the empty header has found
                        tmp = (char*)ut->newP(chunkBufSize);
                        int readLen = ut->readLine(stream, tmp, chunkBufSize);
                        bool headerEnded = false;

                        //check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (tmp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (tmp[0] == '\r' && tmp[1] == '\n')
                                headerEnded = true;

                        if (headerEnded)
                        {
                            //parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);

                            if (response.httpCode == LINENOTIFY_ERROR_HTTP_CODE_OK)
                                result.status = LineNotify_Sending_Success;

                            result.quota = response.quota;

                            if (response.httpCode == LINENOTIFY_ERROR_HTTP_CODE_NO_CONTENT)
                                result.error.code = 0;

                            if (hstate == 1)
                                ut->delP(&header);
                            hstate = 0;

                            if (response.contentLen == 0)
                            {
                                ut->delP(&tmp);
                                break;
                            }
                        }
                        else
                        {
                            //accumulate the remaining header field
                            memcpy(header + hBufPos, tmp, readLen);
                            hBufPos += readLen;
                        }
                        ut->delP(&tmp);
                    }
                    else
                    {
                        //the next chuunk data is the payload
                        if (!response.noContent)
                        {

                            pChunkIdx++;
                            pChunk = (char*)ut->newP(chunkBufSize + 1);

                            if (response.isChunkedEnc)
                                delay(10);
                            //read the avilable data
                            //chunk transfer encoding?
                            if (response.isChunkedEnc)
                                availablePayload = ut->readChunkedData(stream, pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                            else
                                availablePayload = ut->readLine(stream, pChunk, chunkBufSize);

                            if (availablePayload > 0)
                            {
                                payloadRead += availablePayload;

                                if (payloadRead < payloadLen)
                                    payload += pChunk;
                            }

                            ut->delP(&pChunk);

                            if (availablePayload < 0 || (payloadRead >= response.contentLen && !response.isChunkedEnc))
                            {
                                while (stream->available() > 0)
                                    stream->read();
                                break;
                            }
                        }
                        else
                        {
                            //read all the rest data
                            while (stream->available() > 0)
                                stream->read();
                        }
                    }
                }

                chunkIdx++;
            }
        }

        if (hstate == 1)
            ut->delP(&header);

        //parse the payload
        if (payload.length() > 0)
        {
            FirebaseJson js;
            FirebaseJsonData data;
            js.setJsonData(payload.c_str());
            char *tmp = ut->strP(esp_line_notify_str_45);
            js.get(data, (const char *)tmp);
            ut->delP(&tmp);
            if (data.success)
                result.error.code = data.intValue;

            tmp = ut->strP(esp_line_notify_str_19);
            js.get(data, (const char *)tmp);
            ut->delP(&tmp);
            if (data.success)
                result.error.message = data.stringValue.c_str();
        }

        return result.error.code == LINENOTIFY_ERROR_HTTP_CODE_OK;
    }
    else
    {

        while (stream->available() > 0)
            stream->read();
    }

    return client._int.http_code == LINENOTIFY_ERROR_HTTP_CODE_OK;
}

bool ESP_Line_Notify::reconnect(LineNotifyClient &client, unsigned long dataTime)
{

    bool status = WiFi.status() == WL_CONNECTED;

    if (dataTime > 0)
    {
        if (millis() - dataTime > client.httpClient->timeout)
        {
            client._int.http_code = LINENOTIFY_ERROR_HTTPC_ERROR_READ_TIMEOUT;
            char *tmp = ut->strP(esp_line_notify_str_44);
            result.error.message = tmp;
            result.error.code = client._int.http_code;
            ut->delP(&tmp);
            closeSession(client);
            return false;
        }
    }

    if (!status)
    {
        if (client._int.http_connected)
            closeSession(client);

        client._int.http_code = LINENOTIFY_ERROR_HTTPC_ERROR_CONNECTION_LOST;

        if (client.reconnect_wifi)
        {
            if (millis() - client._int.esp_line_notify_last_reconnect_millis > client._int.esp_line_notify_reconnect_tmo && !client._int.http_connected)
            {
                WiFi.reconnect();
                client._int.esp_line_notify_last_reconnect_millis = millis();
            }
        }

        status = WiFi.status() == WL_CONNECTED;
    }

    return status;
}

void ESP_Line_Notify::closeSession(LineNotifyClient &client)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        //close the socket and free the resources used by the BearSSL data
        if (client._int.http_connected || client.httpClient->stream())
        {
            client._int.esp_line_notify_last_reconnect_millis = millis();
            if (client.httpClient->stream())
                if (client.httpClient->stream()->connected())
                    client.httpClient->stream()->stop();
        }
    }

    client._int.http_connected = false;
}

void ESP_Line_Notify::setHeader(LineNotifyClient &client, MBSTRING &buf, MBSTRING &boundary, size_t contentLength)
{
    ut->appendP(buf, esp_line_notify_str_5);
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_6);
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_7);
    buf += client.token;
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_8);
    buf += boundary;
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_9);
    ut->appendP(buf, esp_line_notify_str_3);
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_10);
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_11);
    ut->appendP(buf, esp_line_notify_str_1);
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_12);
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_13);
    ut->appendP(buf, esp_line_notify_str_4);

    ut->appendP(buf, esp_line_notify_str_14);
    char *tmp = ut->intStr(contentLength);
    buf += tmp;
    ut->delP(&tmp);
    ut->appendP(buf, esp_line_notify_str_4);
    ut->appendP(buf, esp_line_notify_str_4);
}

void ESP_Line_Notify::setMultipartHeader(MBSTRING &buf, MBSTRING &boundary, esp_line_notify_multipart_header_type type, const char *imgFile)
{
    ut->appendP(buf, esp_line_notify_str_24);
    buf += boundary;
    ut->appendP(buf, esp_line_notify_str_4);

    if (type == esp_line_notify_multipart_header_type_notification_disabled || type == esp_line_notify_multipart_header_type_message || type == esp_line_notify_multipart_header_type_sticker_id || type == esp_line_notify_multipart_header_type_sticker_package_id || type == esp_line_notify_multipart_header_type_image_thumbnail || type == esp_line_notify_multipart_header_type_image_full_size)
    {
        ut->appendP(buf, esp_line_notify_str_18);
        if (type == esp_line_notify_multipart_header_type_message)
            ut->appendP(buf, esp_line_notify_str_19);
        else if (type == esp_line_notify_multipart_header_type_sticker_id)
            ut->appendP(buf, esp_line_notify_str_21);
        else if (type == esp_line_notify_multipart_header_type_sticker_package_id)
            ut->appendP(buf, esp_line_notify_str_20);
        else if (type == esp_line_notify_multipart_header_type_image_thumbnail)
            ut->appendP(buf, esp_line_notify_str_22);
        else if (type == esp_line_notify_multipart_header_type_image_full_size)
            ut->appendP(buf, esp_line_notify_str_23);
        else if (type == esp_line_notify_multipart_header_type_notification_disabled)
            ut->appendP(buf, esp_line_notify_str_57);

        ut->appendP(buf, esp_line_notify_str_25);
        ut->appendP(buf, esp_line_notify_str_4);
        ut->appendP(buf, esp_line_notify_str_4);
    }
    else
    {
        ut->appendP(buf, esp_line_notify_str_16);
        buf += imgFile;
        ut->appendP(buf, esp_line_notify_str_25);
        ut->appendP(buf, esp_line_notify_str_4);
        ut->appendP(buf, esp_line_notify_str_17);
        getContentType(imgFile, buf);

        ut->appendP(buf, esp_line_notify_str_4);
        ut->appendP(buf, esp_line_notify_str_4);
    }
}

void ESP_Line_Notify::getContentType(const MBSTRING &filename, MBSTRING &buf)
{
    char *tmp = ut->strP(esp_line_notify_str_35);
    size_t p1 = filename.find_last_of(tmp);
    ut->delP(&tmp);
    if (p1 != MBSTRING::npos)
    {
        tmp = ut->strP(esp_line_notify_str_26);
        char *tmp2 = ut->strP(esp_line_notify_str_27);
        char *tmp3 = ut->strP(esp_line_notify_str_29);
        char *tmp4 = ut->strP(esp_line_notify_str_31);
        char *tmp5 = ut->strP(esp_line_notify_str_33);

        if (filename.find(tmp, p1) != MBSTRING::npos || filename.find(tmp2, p1) != MBSTRING::npos)
            ut->appendP(buf, esp_line_notify_str_28);
        else if (filename.find(tmp3, p1) != MBSTRING::npos)
            ut->appendP(buf, esp_line_notify_str_30);
        else if (filename.find(tmp4, p1) != MBSTRING::npos)
            ut->appendP(buf, esp_line_notify_str_32);
        else if (filename.find(tmp5, p1) != MBSTRING::npos)
            ut->appendP(buf, esp_line_notify_str_34);

        ut->delP(&tmp);
        ut->delP(&tmp2);
        ut->delP(&tmp3);
        ut->delP(&tmp4);
        ut->delP(&tmp5);
    }
}

void ESP_Line_Notify::setMultipartBoundary(MBSTRING &buf, MBSTRING &boundary)
{
    ut->appendP(buf, esp_line_notify_str_24);
    buf += boundary;
    ut->appendP(buf, esp_line_notify_str_24);
    ut->appendP(buf, esp_line_notify_str_4);
}

void ESP_Line_Notify::reportUpploadProgress(LineNotifyClient &client, size_t total, size_t read)
{

    if (!client.sendingg_callback)
        return;

    int p = 100 * read / total;

    if ((p % 2 == 0) && (p <= 100))
    {
        if (client._int.report_state == 0 || (p == 0 && client._int.report_state == -1))
        {
            result.progress = p;
            client.sendingg_callback(result);

            if (p == 0 && client._int.report_state == -1)
                client._int.report_state = 1;
            else if (client._int.report_state == 0)
                client._int.report_state = -1;
        }
    }
    else
        client._int.report_state = 0;
}

ESP_Line_Notify LineNotify = ESP_Line_Notify();

#endif