/*
 * TCP Client Base class, version 1.0.2
 *
 * Created February 28, 2022
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
 *
 *
 * Permission is hereby granted, free of charge, to any person returning a copy of
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

#ifndef ESP_Line_Notify_TCP_CLIENT_BASE_H
#define ESP_Line_Notify_TCP_CLIENT_BASE_H

#include <Arduino.h>
#include <IPAddress.h>
#include <Client.h>

#include "./mbfs/MB_FS.h"
#include "ESP_Line_Notify_Net.h"


typedef void (*ESP_Line_Notify_TCPConnectionRequestCallback)(const char *, int);
typedef void (*ESP_Line_Notify_NetworkConnectionRequestCallback)(void);
typedef void (*ESP_Line_Notify_NetworkStatusRequestCallback)(void);

#include "ESP_Line_Notify_Error.h"

typedef enum
{
    esp_line_notify_cert_type_undefined = -1,
    esp_line_notify_cert_type_none = 0,
    esp_line_notify_cert_type_data,
    esp_line_notify_cert_type_file

} esp_line_notify_cert_type;

typedef enum
{
    esp_line_notify_tcp_client_type_undefined,
    esp_line_notify_tcp_client_type_internal,
    esp_line_notify_tcp_client_type_external

} esp_line_notify_tcp_client_type;

class ESP_Line_Notify_TCP_Client_Base
{
    friend class ESP_Line_Notify;

public:
    ESP_Line_Notify_TCP_Client_Base()
    {
        certType = esp_line_notify_cert_type_undefined;
    };
    virtual ~ESP_Line_Notify_TCP_Client_Base(){};

    virtual void ethDNSWorkAround(){};

    virtual bool networkReady() { return false; }

    virtual void networkReconnect(){};

    virtual void disconnect(){};

    virtual esp_line_notify_tcp_client_type type() { return esp_line_notify_tcp_client_type_undefined; }

    virtual bool isInitialized() { return false; }

    virtual int hostByName(const char *name, IPAddress &ip) { return 0; }

    int virtual setError(int code)
    {
        if (!response_code)
            return -1000;

        *response_code = code;
        return *response_code;
    }

    virtual bool begin(const char *host, uint16_t port, int *response_code)
    {

        this->host = host;
        this->port = port;
        this->response_code = response_code;

        return true;
    }

    virtual bool connect()
    {

        if (!client)
            return false;

        if (connected())
        {
            flush();
            return true;
        }

        client->setTimeout(timeoutMs);

        if (!client->connect(host.c_str(), port))
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        return connected();
    }

    virtual void stop()
    {
        if (!client)
            return;

        if (connected())
            return client->stop();
    };

    virtual bool connected()
    {
        if (client)
        {
            bool ret = client->connected();

            if (!ret)
            {
                setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);
                client->stop();
                client->flush();
            }

            return ret;
        }

        return false;
    }

    virtual int write(uint8_t *data, int len)
    {
        if (!data || !client)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_SEND_REQUEST_FAILED);

        if (len == 0)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_SEND_REQUEST_FAILED);

        // call base or derived connect.
        if (!connect())
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int toSend = chunkSize;
        int sent = 0;
        while (sent < len)
        {
            if (sent + toSend > len)
                toSend = len - sent;

#if defined(ESP8266)
            delay(0);
#endif
            int res = client->write(data + sent, toSend);

         

            if (res != toSend)
                return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_SEND_REQUEST_FAILED);

            sent += toSend;
        }

        setError(ESP_LINE_NOTIFY_ERROR_HTTP_CODE_OK);

        return len;
    }

    virtual int send(const char *data, int len = 0)
    {
        if (len == 0)
            len = strlen(data);
        return write((uint8_t *)data, len);
    }

    virtual int print(const char *data)
    {
        return send(data);
    }

    virtual int print(int data)
    {
        char *buf = (char *)mbfs->newP(64);
        sprintf(buf, (const char *)MBSTRING_FLASH_MCR("%d"), data);
        int ret = send(buf);
        mbfs->delP(&buf);
        return ret;
    }

    virtual int println(const char *data)
    {
        int len = send(data);
        if (len < 0)
            return len;
        int sz = send((const char *)MBSTRING_FLASH_MCR("\r\n"));
        if (sz < 0)
            return sz;
        return len + sz;
    }

    virtual int println(int data)
    {
        char *buf = (char *)mbfs->newP(64);
        sprintf(buf, (const char *)MBSTRING_FLASH_MCR("%d\r\n"), data);
        int ret = send(buf);
        mbfs->delP(&buf);
        return ret;
    }

    virtual int available()
    {
        if (!client)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        return client->available();
    }

    virtual int read()
    {
        if (!client)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int r = client->read();

        if (r < 0)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_RESPONSE_READ_FAILED);

        return r;
    }

    virtual int readBytes(uint8_t *buf, int len)
    {
        if (!client)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int r = client->readBytes(buf, len);

        if (r != len)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_RESPONSE_READ_FAILED);

        setError(ESP_LINE_NOTIFY_ERROR_HTTP_CODE_OK);

        return r;
    }

    virtual int readBytes(char *buf, int len) { return readBytes((uint8_t *)buf, len); }

    void baseSetCertType(esp_line_notify_cert_type type) { certType = type; }

    void baseSetTimeout(uint32_t timeoutSec) { timeoutMs = timeoutSec * 1000; }

    int readLine(char *buf, int bufLen)
    {
        if (!client)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int res = -1;
        char c = 0;
        int idx = 0;
        if (!client)
            return idx;
        while (client->available() && idx <= bufLen)
        {
            if (!client)
                break;

#if defined(ESP8266)
            delay(0);
#endif
            res = client->read();
            if (res > -1)
            {
                c = (char)res;
                strcat_c(buf, c);
                idx++;
                if (c == '\n')
                    return idx;
            }
        }
        return idx;
    }

    int readLine(MB_String &buf)
    {
        if (!client)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        int res = -1;
        char c = 0;
        int idx = 0;
        if (!client)
            return idx;
        while (client->available())
        {
            if (!client)
                break;

#if defined(ESP8266)
            delay(0);
#endif
            res = client->read();
            if (res > -1)
            {
                c = (char)res;
                buf += c;
                idx++;
                if (c == '\n')
                    return idx;
            }
        }
        return idx;
    }

    int readChunkedData(char *out, int &chunkState, int &chunkedSize, int &dataLen, int bufLen)
    {
        if (!client)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        char *tmp = nullptr;
        char *buf = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            buf = (char *)mbfs->newP(bufLen);
            int readLen = readLine(buf, bufLen);
            if (readLen)
            {
                p1 = strpos(buf, (const char *)MBSTRING_FLASH_MCR(";"), 0);
                if (p1 == -1)
                {
                    p1 = strpos(buf, (const char *)MBSTRING_FLASH_MCR("\r\n"), 0);
                }

                if (p1 != -1)
                {
                    tmp = (char *)mbfs->newP(p1 + 1);
                    memcpy(tmp, buf, p1);
                    chunkedSize = hex2int(tmp);
                    mbfs->delP(&tmp);
                }

                // last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;

            mbfs->delP(&buf);
        }
        else
        {

            if (chunkedSize > -1)
            {
                buf = (char *)mbfs->newP(bufLen);
                int readLen = readLine(buf, bufLen);

                if (readLen > 0)
                {
                    // chunk may contain trailing
                    if (dataLen + readLen - 2 < chunkedSize)
                    {
                        dataLen += readLen;
                        memcpy(out, buf, readLen);
                        olen = readLen;
                    }
                    else
                    {
                        if (chunkedSize - dataLen > 0)
                            memcpy(out, buf, chunkedSize - dataLen);
                        dataLen = chunkedSize;
                        chunkState = 0;
                        olen = readLen;
                    }
                }
                else
                {
                    olen = -1;
                }

                mbfs->delP(&buf);
            }
        }

        return olen;
    }

    int readChunkedData(MB_String &out, int &chunkState, int &chunkedSize, int &dataLen)
    {
        if (!client)
            return setError(ESP_LINE_NOTIFY_ERROR_TCP_ERROR_CONNECTION_REFUSED);

        char *tmp = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            MB_String s;
            int readLen = readLine(s);
            if (readLen)
            {
                p1 = strpos(s.c_str(), (const char *)MBSTRING_FLASH_MCR(";"), 0);
                if (p1 == -1)
                {
                    p1 = strpos(s.c_str(), (const char *)MBSTRING_FLASH_MCR("\r\n"), 0);
                }

                if (p1 != -1)
                {
                    tmp = (char *)mbfs->newP(p1 + 1);
                    memcpy(tmp, s.c_str(), p1);
                    chunkedSize = hex2int(tmp);
                    mbfs->delP(&tmp);
                }

                // last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;
        }
        else
        {

            if (chunkedSize > -1)
            {
                MB_String s;
                int readLen = readLine(s);

                if (readLen > 0)
                {
                    // chunk may contain trailing
                    if (dataLen + readLen - 2 < chunkedSize)
                    {
                        dataLen += readLen;
                        out += s;
                        olen = readLen;
                    }
                    else
                    {
                        if (chunkedSize - dataLen > 0)
                            out += s;
                        dataLen = chunkedSize;
                        chunkState = 0;
                        olen = readLen;
                    }
                }
                else
                {
                    olen = -1;
                }
            }
        }

        return olen;
    }

    virtual void flush()
    {
        if (!client)
            return;

        while (client->available() > 0)
            client->read();
    }

    esp_line_notify_cert_type getCertType() { return certType; }

private:
    void strcat_c(char *str, char c)
    {
        for (; *str; str++)
            ;
        *str++ = c;
        *str++ = 0;
    }

    int strpos(const char *haystack, const char *needle, int offset)
    {
        if (!haystack || !needle)
            return -1;

        int hlen = strlen(haystack);
        int nlen = strlen(needle);

        if (hlen == 0 || nlen == 0)
            return -1;

        int hidx = offset, nidx = 0;
        while ((*(haystack + hidx) != '\0') && (*(needle + nidx) != '\0') && hidx < hlen)
        {
            if (*(needle + nidx) != *(haystack + hidx))
            {
                hidx++;
                nidx = 0;
            }
            else
            {
                nidx++;
                hidx++;
                if (nidx == nlen)
                    return hidx - nidx;
            }
        }

        return -1;
    }

    uint32_t hex2int(const char *hex)
    {
        uint32_t val = 0;
        while (*hex)
        {
            // get current character then increment
            uint8_t byte = *hex++;
            // transform hex character to the 4bit equivalent number, using the ascii table indexes
            if (byte >= '0' && byte <= '9')
                byte = byte - '0';
            else if (byte >= 'a' && byte <= 'f')
                byte = byte - 'a' + 10;
            else if (byte >= 'A' && byte <= 'F')
                byte = byte - 'A' + 10;
            // shift 4 to make space for new digit, and add the 4 bits of the new digit
            val = (val << 4) | (byte & 0xF);
        }
        return val;
    }

    int tcpTimeout()
    {
        return timeoutMs;
    }

    void setMBFS(MB_FS *mbfs) { this->mbfs = mbfs; }

    void setSPIEthernet(SPI_ETH_Module *eth) { this->eth = eth; }

    esp_line_notify_cert_type certType = esp_line_notify_cert_type_undefined;

protected:
    MB_String host;
    uint16_t port = 0;
    MB_FS *mbfs = nullptr;
    Client *client = nullptr;
    int chunkSize = 1024;

    // In esp8266, this is actually Arduino base Stream (char read) timeout.
    //  This will override internally by WiFiClientSecureCtx::_connectSSL
    //  to 5000 after SSL handshake was done with success.
    int timeoutMs = 40000; // 40 sec
    bool clockReady = false;
    time_t now = 0;
    int *response_code = nullptr;
    SPI_ETH_Module *eth = NULL;
};

#endif
