/**
 * LINE Notify Arduino Library for Arduino version 2.1.1
 *
 * Created May 30, 2023
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

#ifndef ESP_LINE_NOTIFY_CPP
#define ESP_LINE_NOTIFY_CPP

#include "ESP_Line_Notify.h"

ESP_Line_Notify::ESP_Line_Notify()
{
    if (!mbfs)
        mbfs = new MB_FS();
}

ESP_Line_Notify::~ESP_Line_Notify()
{
    if (ut)
        delete ut;

    if (mbfs)
        delete mbfs;
}

LineNotifySendingResult ESP_Line_Notify::send(LineNotifyClient &client)
{
    result.status = LineNotify_Sending_None;
    result.error.code = ESP_LINE_NOTIFY_ERROR_HTTP_CODE_UNDEFINED;
    result.error.message.clear();
    result.quota.image.limit = 0;
    result.quota.image.remaining = 0;
    result.quota.text.limit = 0;
    result.quota.text.remaining = 0;
    result.quota.reset = 0;

    if (!client.tcpClient)
        client.tcpClient = new ESP_LINE_NOTIFY_TCP_CLIENT();

#if defined(ESP8266)
    client.tcpClient->setSPIEthernet(&spi_ethernet_module);
#endif

    if (!reconnect(client, 0))
    {
        result.status = LineNotify_Sending_Error;
        result.error.code = ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_LOST;
        result.error.message.clear();
        errorToString(result.error.code, result.error.message);

        if (client.sending_callback)
            client.sending_callback(result);

        closeSession(client);
        return result;
    }

    result.status = LineNotify_Sending_In_Progress;

    if (!ut)
        ut = new ESP_Line_Notify_Utils(&client, mbfs);

    client.tcpClient->setCACert(nullptr);

    MB_String host = esp_line_notify_str_1;
    int responseCode = 0;

    client.tcpClient->begin(host.c_str(), 443, &responseCode);
    uint32_t contentLength = 0;
    int ret = 0;

    MB_String textPayload;
    MB_String header;
    MB_String multipath;
    MB_String boundary = ut->getBoundary(10);

    result.status = LineNotify_Sending_Begin;
    if (client.sending_callback)
        client.sending_callback(result);

    if (client.message.length() > 0)
    {
        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_message, "");
        textPayload += client.message;

        if (client.gmap.center.length() > 0)
        {
            textPayload += esp_line_notify_str_68;
            textPayload += esp_line_notify_str_61;
            textPayload += client.gmap.center;
            textPayload += esp_line_notify_str_67;
            textPayload += client.gmap.map_type;
            textPayload += esp_line_notify_str_63;
            textPayload += client.gmap.zoom;
        }

        textPayload += esp_line_notify_str_4;
    }

    if (client.notification_disabled)
    {
        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_notification_disabled, "");
        textPayload += true;
        textPayload += esp_line_notify_str_4;
    }

    if (client.sticker.id > 0 && client.sticker.package_id > 0)
    {
        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_sticker_package_id, "");
        textPayload += client.sticker.package_id;
        textPayload += esp_line_notify_str_4;

        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_sticker_id, "");
        textPayload += client.sticker.id;
        textPayload += esp_line_notify_str_4;
    }

    if (client.image.url.length() > 0)
    {
        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_image_thumbnail, "");
        textPayload += client.image.url;
        textPayload += esp_line_notify_str_4;

        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_image_full_size, "");
        textPayload += client.image.url;
        textPayload += esp_line_notify_str_4;
    }

    if (client.gmap.google_api_key.length() > 0 && client.gmap.center.length() > 0)
    {
        MB_String url = esp_line_notify_str_60;
        url += client.gmap.google_api_key;
        url += esp_line_notify_str_62;
        url += client.gmap.center;
        url += esp_line_notify_str_63;
        url += client.gmap.zoom;
        url += esp_line_notify_str_64;
        url += client.gmap.map_type;
        url += esp_line_notify_str_65;
        url += client.gmap.size;

        MB_VECTOR<MB_String> mkrs = MB_VECTOR<MB_String>();
        ut->splitTk(client.gmap.markers, mkrs, " ");
        for (size_t i = 0; i < mkrs.size(); i++)
        {
            url += esp_line_notify_str_66;
            url += ut->url_encode(mkrs[i]);
        }

        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_image_thumbnail, "");
        textPayload += url;
        textPayload += esp_line_notify_str_4;

        setMultipartHeader(textPayload, boundary, esp_line_notify_multipart_header_type_image_full_size, "");
        textPayload += url;
        textPayload += esp_line_notify_str_4;
    }

    int sz = -1;

    if (client.image.file.path.length() > 0)
    {
        MB_String fpath;
        if (client.image.file.path[0] != '/')
            fpath = "/";
        fpath += client.image.file.path;

        sz = mbfs->open(fpath, mbfs_type client.image.file.storage_type, mb_fs_open_mode_read);

        if (sz < 0)
        {
            result.status = LineNotify_Sending_Error;
            result.error.code = sz;
            result.error.message.clear();
            errorToString(result.error.code, result.error.message);

            if (client.sending_callback)
                client.sending_callback(result);
            closeSession(client);
            return result;
        }

        result.file_name = client.image.file.name;
        client.internal.upload_len = sz;
        setMultipartHeader(multipath, boundary, esp_line_notify_multipart_header_type_undefined, client.image.file.name.c_str());
        contentLength = textPayload.length() + multipath.length() + sz + boundary.length() + 6;
    }

    if (client.image.data.blob && client.image.data.size > 0)
    {
        result.file_name = client.image.data.file_name;
        client.internal.upload_len = client.image.data.size;
        setMultipartHeader(multipath, boundary, esp_line_notify_multipart_header_type_undefined, client.image.data.file_name.c_str());
        contentLength = textPayload.length() + multipath.length() + client.image.data.size + boundary.length() + 6;
    }

    if (client.image.file.path.length() || client.image.data.size > 0)
    {
        setHeader(client, header, boundary, contentLength);

        ret = client.tcpClient->send(header.c_str());

        if (ret >= 0)
            ret = client.tcpClient->send(textPayload.c_str());

        if (ret >= 0)
            ret = client.tcpClient->send(multipath.c_str());

        header.clear();
        textPayload.clear();
        multipath.clear();

        if (ret < 0)
        {
            result.status = LineNotify_Sending_Error;
            result.error.code = responseCode;
            result.error.message.clear();
            errorToString(result.error.code, result.error.message);
            if (client.sending_callback)
                client.sending_callback(result);
            closeSession(client);
            return result;
        }

        size_t byteRead = 0;
        result.status = LineNotify_Sending_Upload;

        if (sz && client.image.file.path.length() > 0)
        {
            int available = mbfs->available(mbfs_type client.image.file.storage_type);
            int bufLen = 1024;
            uint8_t *buf = (uint8_t *)mbfs->newP(bufLen + 1);
            int read = 0;
            client.internal.progress = -1;
            while (available)
            {
                delay(0);
                if (available > bufLen)
                    available = bufLen;
                byteRead += available;
                read = mbfs->read(mbfs_type client.image.file.storage_type, buf, available);
                if (client.tcpClient->write(buf, read) != read)
                    break;
                reportUpploadProgress(client, client.internal.upload_len, byteRead);
                available = mbfs->available(mbfs_type client.image.file.storage_type);
            }
            mbfs->delP(&buf);
            mbfs->close(mbfs_type client.image.file.storage_type);
        }
        else if (client.image.data.size > 0)
        {
            int len = client.image.data.size;
            int available = len;
            int bufLen = 1024;
            uint8_t *buf = (uint8_t *)mbfs->newP(bufLen + 1);
            size_t pos = 0;
            while (available)
            {
                delay(0);
                if (available > bufLen)
                    available = bufLen;
                byteRead += available;
                memcpy_P(buf, client.image.data.blob + pos, available);

                if (client.tcpClient->write(buf, available) != available)
                    break;

                reportUpploadProgress(client, client.internal.upload_len, byteRead);
                pos += available;
                len -= available;
                available = len;
            }
            mbfs->delP(&buf);
        }

        textPayload.clear();

        textPayload += esp_line_notify_str_4;

        setMultipartBoundary(textPayload, boundary);

       
        ret = client.tcpClient->send(textPayload.c_str());
        textPayload.clear();
        if (ret < 0)
        {
            result.status = LineNotify_Sending_Error;
            result.error.code = responseCode;
            result.error.message.clear();
            errorToString(result.error.code, result.error.message);
            if (client.sending_callback)
                client.sending_callback(result);
            closeSession(client);
            return result;
        }
    }
    else
    {
        
        setMultipartBoundary(textPayload, boundary);

        contentLength = textPayload.length();

        setHeader(client, header, boundary, contentLength);

        ret = client.tcpClient->send(header.c_str());
        if (ret >= 0)
            ret = client.tcpClient->send(textPayload.c_str());
        textPayload.clear();
        header.clear();
    }

    if (ret >= 0)
    {
        result.status = LineNotify_Sending_Success;
        if (!handleResponse(client, result.error.code))
        {
            result.status = LineNotify_Sending_Error;
            errorToString(result.error.code, result.error.message);
        }
    }
    else
    {
        result.status = LineNotify_Sending_Error;
        result.error.code = responseCode;
        result.error.message.clear();
        errorToString(result.error.code, result.error.message);
    }

    closeSession(client);

    if (client.sending_callback)
        client.sending_callback(result);

    return result;
}

bool ESP_Line_Notify::handleResponse(LineNotifyClient &client, int &responseCode)
{

    if (!reconnect(client, 0))
        return false;

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
    int chunkBufSize = client.tcpClient->available();
    int hstate = 0;
    int chunkedDataState = 0;
    int chunkedDataSize = 0;
    int chunkedDataLen = 0;
    int defaultChunkSize = client.response_size;

    int payloadRead = 0;

    while (client.tcpClient->connected() && chunkBufSize <= 0)
    {
        if (!reconnect(client, dataTime))
        {
            responseCode = ESP_LINE_NOTIFY_ERROR_TCP_ERROR_NOT_CONNECTED;
            return false;
        }
        chunkBufSize = client.tcpClient->available();
        delay(0);
    }

    int availablePayload = chunkBufSize;
    MB_String payload;

    dataTime = millis();

    if (chunkBufSize > 1)
    {
        while (chunkBufSize > 0 || availablePayload > 0 || payloadRead < response.contentLen)
        {
            if (!reconnect(client, dataTime))
                return false;

            chunkBufSize = client.tcpClient->available();

            if (chunkBufSize <= 0 && availablePayload <= 0 && payloadRead >= response.contentLen && response.contentLen > 0)
                break;

            if (chunkBufSize > 0)
            {
                chunkBufSize = defaultChunkSize;

                if (chunkIdx == 0)
                {
                    // the first chunk can be http response header
                    header = (char *)mbfs->newP(chunkBufSize);
                    hstate = 1;
                    int readLen = client.tcpClient->readLine(header, chunkBufSize);
                    int pos = 0;

                    tmp = ut->getHeader(header, esp_line_notify_str_36, esp_line_notify_str_2, pos, 0);
                    delay(0);
                    dataTime = millis();
                    if (tmp)
                    {
                        // http response header with http response code
                        isHeader = true;
                        hBufPos = readLen;
                        response.httpCode = atoi(tmp);
                        responseCode = response.httpCode;
                        mbfs->delP(&tmp);
                    }
                }
                else
                {
                    delay(0);
                    dataTime = millis();
                    // the next chunk data can be the remaining http header
                    if (isHeader)
                    {
                        // read one line of next header field until the empty header has found
                        tmp = (char *)mbfs->newP(chunkBufSize);
                        int readLen = client.tcpClient->readLine(tmp, chunkBufSize);
                        bool headerEnded = false;

                        // check is it the end of http header (\n or \r\n)?
                        if (readLen == 1)
                            if (tmp[0] == '\r')
                                headerEnded = true;

                        if (readLen == 2)
                            if (tmp[0] == '\r' && tmp[1] == '\n')
                                headerEnded = true;

                        if (headerEnded)
                        {
                            // parse header string to get the header field
                            isHeader = false;
                            ut->parseRespHeader(header, response);

                            if (response.httpCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_OK)
                                result.status = LineNotify_Sending_Success;

                            result.quota = response.quota;

                            if (response.httpCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_NO_CONTENT)
                                result.error.code = 0;

                            if (hstate == 1)
                                mbfs->delP(&header);
                            hstate = 0;

                            if (response.contentLen == 0)
                            {
                                mbfs->delP(&tmp);
                                break;
                            }
                        }
                        else
                        {
                            // accumulate the remaining header field
                            memcpy(header + hBufPos, tmp, readLen);
                            hBufPos += readLen;
                        }
                        mbfs->delP(&tmp);
                    }
                    else
                    {
                        // the next chuunk data is the payload
                        if (!response.noContent)
                        {

                            pChunkIdx++;
                            pChunk = (char *)mbfs->newP(chunkBufSize + 1);

                            if (response.isChunkedEnc)
                                delay(10);
                            // read the avilable data
                            // chunk transfer encoding?
                            if (response.isChunkedEnc)
                                availablePayload = client.tcpClient->readChunkedData(pChunk, chunkedDataState, chunkedDataSize, chunkedDataLen, chunkBufSize);
                            else
                                availablePayload = client.tcpClient->readLine(pChunk, chunkBufSize);

                            if (availablePayload > 0)
                            {
                                payloadRead += availablePayload;

                                if (payloadRead < payloadLen)
                                    payload += pChunk;
                            }

                            mbfs->delP(&pChunk);

                            if (availablePayload < 0 || (payloadRead >= response.contentLen && !response.isChunkedEnc))
                            {
                                client.tcpClient->flush();
                                break;
                            }
                        }
                        else
                        {
                            // read all the rest data
                            client.tcpClient->flush();
                        }
                    }
                }

                chunkIdx++;
            }
        }

        if (hstate == 1)
            mbfs->delP(&header);

        // parse the payload
        if (payload.length() > 0)
        {
            FirebaseJson js;
            FirebaseJsonData data;
            js.setJsonData(payload.c_str());
            char *tmp = ut->strP(esp_line_notify_str_45);
            js.get(data, (const char *)tmp);
            mbfs->delP(&tmp);
            if (data.success)
                result.error.code = data.intValue;

            tmp = ut->strP(esp_line_notify_str_19);
            js.get(data, (const char *)tmp);
            mbfs->delP(&tmp);
            if (data.success)
                result.error.message = data.stringValue.c_str();
        }

        return result.error.code == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_OK;
    }
    else
    {

        client.tcpClient->flush();
    }

    return responseCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_OK;
}

bool ESP_Line_Notify::reconnect(LineNotifyClient &client, unsigned long dataTime)
{

    if (client.tcpClient->type() == esp_line_notify_tcp_client_type_external)
    {
#if !defined(ESP_LINE_NOTIFY_ENABLE_EXTERNAL_CLIENT)
        client.response.code = ESP_LINE_NOTIFY_ERROR_EXTERNAL_CLIENT_DISABLED;
        return false;
#endif
        if (!client.tcpClient->isInitialized())
        {
            client.response.code = ESP_LINE_NOTIFY_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED;
            return false;
        }
    }

    bool status = client.tcpClient->networkReady();

    if (dataTime > 0)
    {
        unsigned long tmo = 10 * 1000;

        if (millis() - dataTime > tmo)
        {
            client.response.code = ESP_LINE_NOTIFY_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT;

            closeSession(client);
            return false;
        }
    }

    if (!status)
    {
        closeSession(client);

        client.response.code = ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_LOST;

        if (client.reconnect_wifi &&  millis() - client.internal.esp_line_notify_last_reconnect_millis > 10 * 1000)
        {
#if defined(ESP32) || defined(ESP8266)
            WiFi.reconnect();
#else
            client.tcpClient->networkReconnect();
#endif
            client.internal.esp_line_notify_last_reconnect_millis = millis();
        }

        status = client.tcpClient->networkReady();
    }

    return status;
}

void ESP_Line_Notify::closeSession(LineNotifyClient &client)
{

    bool status = client.tcpClient->networkReady();

    if (status)
    {
        // close the socket and free the resources used by the BearSSL data
        if (client.tcpClient->connected())
        {

            client.internal.esp_line_notify_last_reconnect_millis = millis();

            if (client.tcpClient->connected())
                client.tcpClient->stop();
        }
    }
    if (client.tcpClient)
        delete client.tcpClient;
    client.tcpClient = nullptr;
}

void ESP_Line_Notify::setHeader(LineNotifyClient &client, MB_String &buf, MB_String &boundary, size_t contentLength)
{
    buf += esp_line_notify_str_5;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_6;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_7;
    buf += client.token;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_8;
    buf += boundary;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_9;
    buf += esp_line_notify_str_3;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_10;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_11;
    buf += esp_line_notify_str_1;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_12;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_13;
    buf += esp_line_notify_str_4;

    buf += esp_line_notify_str_14;
    buf += contentLength;
    buf += esp_line_notify_str_4;
    buf += esp_line_notify_str_4;
}

void ESP_Line_Notify::setMultipartHeader(MB_String &buf, MB_String &boundary, esp_line_notify_multipart_header_type type, const char *imgFile)
{
    buf += esp_line_notify_str_24;
    buf += boundary;
    buf += esp_line_notify_str_4;

    if (type == esp_line_notify_multipart_header_type_notification_disabled || type == esp_line_notify_multipart_header_type_message || type == esp_line_notify_multipart_header_type_sticker_id || type == esp_line_notify_multipart_header_type_sticker_package_id || type == esp_line_notify_multipart_header_type_image_thumbnail || type == esp_line_notify_multipart_header_type_image_full_size)
    {
        buf += esp_line_notify_str_18;
        if (type == esp_line_notify_multipart_header_type_message)
            buf += esp_line_notify_str_19;
        else if (type == esp_line_notify_multipart_header_type_sticker_id)
            buf += esp_line_notify_str_21;
        else if (type == esp_line_notify_multipart_header_type_sticker_package_id)
            buf += esp_line_notify_str_20;
        else if (type == esp_line_notify_multipart_header_type_image_thumbnail)
            buf += esp_line_notify_str_22;
        else if (type == esp_line_notify_multipart_header_type_image_full_size)
            buf += esp_line_notify_str_23;
        else if (type == esp_line_notify_multipart_header_type_notification_disabled)
            buf += esp_line_notify_str_57;

        buf += esp_line_notify_str_25;
        buf += esp_line_notify_str_4;
        buf += esp_line_notify_str_4;
    }
    else
    {
        buf += esp_line_notify_str_16;
        buf += imgFile;
        buf += esp_line_notify_str_25;
        buf += esp_line_notify_str_4;
        buf += esp_line_notify_str_17;
        getContentType(imgFile, buf);

        buf += esp_line_notify_str_4;
        buf += esp_line_notify_str_4;
    }
}

void ESP_Line_Notify::getContentType(const MB_String &filename, MB_String &buf)
{
    size_t p1 = filename.find_last_of(pgm2Str(esp_line_notify_str_35));
    if (p1 != MB_String::npos)
    {
        if (filename.find(pgm2Str(esp_line_notify_str_26), p1) != MB_String::npos || filename.find(pgm2Str(esp_line_notify_str_27), p1) != MB_String::npos)
            buf += esp_line_notify_str_28;
        else if (filename.find(pgm2Str(esp_line_notify_str_29), p1) != MB_String::npos)
            buf += esp_line_notify_str_30;
        else if (filename.find(pgm2Str(esp_line_notify_str_31), p1) != MB_String::npos)
            buf += esp_line_notify_str_32;
        else if (filename.find(pgm2Str(esp_line_notify_str_33), p1) != MB_String::npos)
            buf += esp_line_notify_str_34;
    }
}

void ESP_Line_Notify::setMultipartBoundary(MB_String &buf, MB_String &boundary)
{
    buf += esp_line_notify_str_24;
    buf += boundary;
    buf += esp_line_notify_str_24;
    buf += esp_line_notify_str_4;
}

void ESP_Line_Notify::reportUpploadProgress(LineNotifyClient &client, size_t total, size_t read)
{

    if (!client.sending_callback)
        return;

    int p = 100 * read / total;

    if (client.internal.progress != p && (p == 0 || p == 100 || client.internal.progress + 2 <= p))
    {
        client.internal.progress = p;
        result.progress = p;
        client.sending_callback(result);
    }
}

#if defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD)

bool ESP_Line_Notify::sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
{
    return mbfs->sdBegin(ss, sck, miso, mosi);
}

#if defined(ESP8266)
bool ESP_Line_Notify::sdBegin(SDFSConfig *sdFSConfig)
{
    return mbfs->sdFatBegin(sdFSConfig);
}
#endif

#if defined(ESP32)

bool ESP_Line_Notify::sdBegin(int8_t ss, SPIClass *spiConfig)
{
    return mbfs->sdSPIBegin(ss, spiConfig);
}
#endif

#if defined(MBFS_ESP32_SDFAT_ENABLED) || defined(MBFS_SDFAT_ENABLED)
bool ESP_Line_Notify::sdBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
{
    return mbfs->sdFatBegin(sdFatSPIConfig, ss, sck, miso, mosi);
}
#endif

#endif

#if defined(ESP8266) && defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD_MMC)

bool ESP_Line_Notify::sdMMCBegin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed)
{

    return mbfs->sdMMCBegin(mountpoint, mode1bit, format_if_mount_failed);
}

#endif

void ESP_Line_Notify::errorToString(int httpCode, MB_String &buff)
{
    buff.clear();

    switch (httpCode)
    {
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_UNDEFINED:
        buff += esp_line_notify_str_106;
        return;
    case ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED:
        buff += esp_line_notify_str_73;
        return;
    case ESP_LINE_NOTIFY_ERROR_TCP_ERROR_SEND_REQUEST_FAILED:
        buff += esp_line_notify_str_74;
        return;
    case ESP_LINE_NOTIFY_ERROR_TCP_ERROR_NOT_CONNECTED:
        buff += esp_line_notify_str_75;
        return;
    case ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_LOST:
        buff += esp_line_notify_str_76;
        return;
    case ESP_LINE_NOTIFY_ERROR_TCP_ERROR_NO_HTTP_SERVER:
        buff += esp_line_notify_str_77;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_BAD_REQUEST:
        buff += esp_line_notify_str_78;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_NON_AUTHORITATIVE_INFORMATION:
        buff += esp_line_notify_str_79;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_NO_CONTENT:
        buff += esp_line_notify_str_80;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_MOVED_PERMANENTLY:
        buff += esp_line_notify_str_81;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_USE_PROXY:
        buff += esp_line_notify_str_82;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_TEMPORARY_REDIRECT:
        buff += esp_line_notify_str_83;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_PERMANENT_REDIRECT:
        buff += esp_line_notify_str_84;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_UNAUTHORIZED:
        buff += esp_line_notify_str_85;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_FORBIDDEN:
        buff += esp_line_notify_str_86;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_NOT_FOUND:
        buff += esp_line_notify_str_87;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_METHOD_NOT_ALLOWED:
        buff += esp_line_notify_str_88;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_NOT_ACCEPTABLE:
        buff += esp_line_notify_str_89;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
        buff += esp_line_notify_str_90;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_REQUEST_TIMEOUT:
        buff += esp_line_notify_str_91;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_LENGTH_REQUIRED:
        buff += esp_line_notify_str_92;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_TOO_MANY_REQUESTS:
        buff += esp_line_notify_str_93;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        buff += esp_line_notify_str_94;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_INTERNAL_SERVER_ERROR:
        buff += esp_line_notify_str_95;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_BAD_GATEWAY:
        buff += esp_line_notify_str_96;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_SERVICE_UNAVAILABLE:
        buff += esp_line_notify_str_97;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_GATEWAY_TIMEOUT:
        buff += esp_line_notify_str_98;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED:
        buff += esp_line_notify_str_99;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
        buff += esp_line_notify_str_100;
        return;
    case ESP_LINE_NOTIFY_ERROR_HTTP_CODE_PRECONDITION_FAILED:
        buff += esp_line_notify_str_101;
        return;
    case ESP_LINE_NOTIFY_ERROR_TCP_RESPONSE_PAYLOAD_READ_TIMED_OUT:
        buff += esp_line_notify_str_102;
        return;
    case ESP_LINE_NOTIFY_ERROR_TCP_RESPONSE_READ_FAILED:
        buff += esp_line_notify_str_103;
        return;
    case MB_FS_ERROR_FILE_IO_ERROR:
        buff += esp_line_notify_str_55;
        return;

#if defined(MBFS_FLASH_FS) || defined(MBFS_SD_FS)

    case MB_FS_ERROR_FLASH_STORAGE_IS_NOT_READY:
        buff += esp_line_notify_str_69;
        return;

    case MB_FS_ERROR_SD_STORAGE_IS_NOT_READY:
        buff += esp_line_notify_str_70;
        return;

    case MB_FS_ERROR_FILE_STILL_OPENED:
        buff += esp_line_notify_str_71;
        return;

    case MB_FS_ERROR_FILE_NOT_FOUND:
        buff += esp_line_notify_str_72;
        return;
#endif

    case ESP_LINE_NOTIFY_ERROR_EXTERNAL_CLIENT_DISABLED:
        buff += esp_line_notify_str_104;
        return;
    case ESP_LINE_NOTIFY_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED:
        buff += esp_line_notify_str_105;
        return;

    default:
        return;
    }
}

ESP_Line_Notify LineNotify = ESP_Line_Notify();

#endif