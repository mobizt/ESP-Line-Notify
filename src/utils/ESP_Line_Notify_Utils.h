/**
 * Util class, Utils.h version 1.0.1
 * 
 * This library supports Espressif ESP8266 and ESP32
 * 
 * Created March 13, 2021
 * 
 * This work is a part of ESP Line Notify library
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
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

#ifndef FB_UTILS_H
#define FB_UTILS_H

#include <Arduino.h>
#include "ESP_Line_Notify_Common.h"

class ESP_Line_Notify_Utils
{
    friend class ESP_Line_Notify;

public:
    
    LineNotiFyClient *client = nullptr;

    ESP_Line_Notify_Utils(LineNotiFyClient c)
    {
        client = &c;
    };

    ~ESP_Line_Notify_Utils(){};

    char *strP(PGM_P pgm)
    {
        size_t len = strlen_P(pgm) + 5;
        char *buf = newS(len);
        memset(buf, 0, len);
        strcpy_P(buf, pgm);
        return buf;
    }

    int strposP(const char *buf, PGM_P beginH, int ofs)
    {
        char *tmp = strP(beginH);
        int p = strpos(buf, tmp, ofs);
        delS(tmp);
        return p;
    }

    bool strcmpP(const char *buf, int ofs, PGM_P beginH)
    {
        char *tmp = nullptr;
        if (ofs < 0)
        {
            int p = strposP(buf, beginH, 0);
            if (p == -1)
                return false;
            ofs = p;
        }
        tmp = strP(beginH);
        char *tmp2 = newS(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;
        bool ret = (strcasecmp(tmp, tmp2) == 0);
        delS(tmp);
        delS(tmp2);
        return ret;
    }

    char *subStr(const char *buf, PGM_P beginH, PGM_P endH, int beginPos, int endPos)
    {

        char *tmp = nullptr;
        int p1 = strposP(buf, beginH, beginPos);
        if (p1 != -1)
        {
            int p2 = -1;
            if (endPos == 0)
                p2 = strposP(buf, endH, p1 + strlen_P(beginH));

            if (p2 == -1)
                p2 = strlen(buf);

            int len = p2 - p1 - strlen_P(beginH);
            tmp = newS(len + 1);
            memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
            return tmp;
        }

        return nullptr;
    }

    void appendP(std::string &buf, PGM_P p, bool empty = false)
    {
        if (empty)
            buf.clear();
        char *b = strP(p);
        buf += b;
        delS(b);
    }

    void trimDigits(char *buf)
    {
        size_t i = strlen(buf) - 1;
        while (buf[i] == '0' && i > 0)
        {
            if (buf[i - 1] == '.')
            {
                i--;
                break;
            }
            if (buf[i - 1] != '0')
                break;
            i--;
        }
        if (i < strlen(buf) - 1)
            buf[i] = '\0';
    }

    void strcat_c(char *str, char c)
    {
        for (; *str; str++)
            ;
        *str++ = c;
        *str++ = 0;
    }

    int strpos(const char *haystack, const char *needle, int offset)
    {
        size_t len = strlen(haystack);
        size_t len2 = strlen(needle);
        if (len == 0 || len < len2 || len2 == 0 || offset >= (int)len)
            return -1;
        char *_haystack = newS(len - offset + 1);
        _haystack[len - offset] = 0;
        strncpy(_haystack, haystack + offset, len - offset);
        char *p = strstr(_haystack, needle);
        int r = -1;
        if (p)
            r = p - _haystack + offset;
        delS(_haystack);
        return r;
    }

    char *rstrstr(const char *haystack, const char *needle)
    {
        size_t needle_length = strlen(needle);
        const char *haystack_end = haystack + strlen(haystack) - needle_length;
        const char *p;
        size_t i;
        for (p = haystack_end; p >= haystack; --p)
        {
            for (i = 0; i < needle_length; ++i)
            {
                if (p[i] != needle[i])
                    goto next;
            }
            return (char *)p;
        next:;
        }
        return 0;
    }

    int rstrpos(const char *haystack, const char *needle, int offset)
    {
        size_t len = strlen(haystack);
        size_t len2 = strlen(needle);
        if (len == 0 || len < len2 || len2 == 0 || offset >= (int)len)
            return -1;
        char *_haystack = newS(len - offset + 1);
        _haystack[len - offset] = 0;
        strncpy(_haystack, haystack + offset, len - offset);
        char *p = rstrstr(_haystack, needle);
        int r = -1;
        if (p)
            r = p - _haystack + offset;
        delS(_haystack);
        return r;
    }

    inline std::string trim(const std::string &s)
    {
        auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
        return std::string(wsfront, std::find_if_not(s.rbegin(), std::string::const_reverse_iterator(wsfront), [](int c) { return std::isspace(c); }).base());
    }

    void delS(char *p)
    {
        if (p != nullptr)
            delete[] p;
    }

    char *newS(size_t len)
    {
        char *p = new char[len];
        memset(p, 0, len);
        return p;
    }

    char *newS(char *p, size_t len)
    {
        delS(p);
        p = newS(len);
        return p;
    }

    char *newS(char *p, size_t len, char *d)
    {
        delS(p);
        p = newS(len);
        strcpy(p, d);
        return p;
    }

    inline int ishex(int x)
    {
        return (x >= '0' && x <= '9') ||
               (x >= 'a' && x <= 'f') ||
               (x >= 'A' && x <= 'F');
    }

    void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
    {
        hex1 = c / 16;
        hex2 = c % 16;
        hex1 += hex1 <= 9 ? '0' : 'a' - 10;
        hex2 += hex2 <= 9 ? '0' : 'a' - 10;
    }

    char from_hex(char ch)
    {
        return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
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

    void parseRespHeader(const char *buf, struct esp_line_notify_server_response_data_t &response)
    {
        int beginPos = 0, pmax = 0, payloadPos = 0;

        char *tmp = nullptr;

        if (response.httpCode != -1)
        {
            payloadPos = beginPos;
            pmax = beginPos;
            tmp = getHeader(buf, esp_line_notify_str_46, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.connection = tmp;
                delS(tmp);
            }
            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_17, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.contentType = tmp;
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_47, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.contentLen = atoi(tmp);
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_50, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.transferEnc = tmp;
                if (stringCompare(tmp, 0, esp_line_notify_str_51))
                    response.isChunkedEnc = true;
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_46, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.connection = tmp;
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_38, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.text.limit = atoi(tmp);
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_39, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.image.limit = atoi(tmp);
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_40, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.text.remaining = atoi(tmp);
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_41, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.image.remaining = atoi(tmp);
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_42, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.reset = atoi(tmp);
                delS(tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_47, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {

                response.payloadLen = atoi(tmp);
                delS(tmp);
            }

            if (response.httpCode == LINENOTIFY_ERROR_HTTP_CODE_OK || response.httpCode == LINENOTIFY_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || response.httpCode == LINENOTIFY_ERROR_HTTP_CODE_PERMANENT_REDIRECT || response.httpCode == LINENOTIFY_ERROR_HTTP_CODE_MOVED_PERMANENTLY || response.httpCode == LINENOTIFY_ERROR_HTTP_CODE_FOUND)
            {
                if (pmax < beginPos)
                    pmax = beginPos;
                beginPos = payloadPos;
                tmp = getHeader(buf, esp_line_notify_str_49, esp_line_notify_str_4, beginPos, 0);
                if (tmp)
                {
                    response.location = tmp;
                    delS(tmp);
                }
            }

            if (response.httpCode == LINENOTIFY_ERROR_HTTP_CODE_NO_CONTENT)
                response.noContent = true;
        }
    }

    int readLine(WiFiClient *stream, char *buf, int bufLen)
    {
        int res = -1;
        char c = 0;
        int idx = 0;
        while (stream->available() && idx <= bufLen)
        {
            res = stream->read();
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

    int readChunkedData(WiFiClient *stream, char *out, int &chunkState, int &chunkedSize, int &dataLen, int bufLen)
    {

        char *tmp = nullptr;
        char *buf = nullptr;
        int p1 = 0;
        int olen = 0;

        if (chunkState == 0)
        {
            chunkState = 1;
            chunkedSize = -1;
            dataLen = 0;
            buf = newS(bufLen);
            int readLen = readLine(stream, buf, bufLen);
            if (readLen)
            {
                tmp = strP(esp_line_notify_str_48);
                p1 = strpos(buf, tmp, 0);
                delS(tmp);
                if (p1 == -1)
                {
                    tmp = strP(esp_line_notify_str_4);
                    p1 = strpos(buf, tmp, 0);
                    delS(tmp);
                }

                if (p1 != -1)
                {
                    tmp = newS(p1 + 1);
                    memcpy(tmp, buf, p1);
                    chunkedSize = hex2int(tmp);
                    delS(tmp);
                }

                //last chunk
                if (chunkedSize < 1)
                    olen = -1;
            }
            else
                chunkState = 0;

            delS(buf);
        }
        else
        {

            if (chunkedSize > -1)
            {
                buf = newS(bufLen);
                int readLen = readLine(stream, buf, bufLen);

                if (readLen > 0)
                {
                    //chunk may contain trailing
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

                delS(buf);
            }
        }

        return olen;
    }

    char *getHeader(const char *buf, PGM_P beginH, PGM_P endH, int &beginPos, int endPos)
    {

        char *tmp = strP(beginH);
        int p1 = strpos(buf, tmp, beginPos);
        int ofs = 0;
        delS(tmp);
        if (p1 != -1)
        {
            tmp = strP(endH);
            int p2 = -1;
            if (endPos > 0)
                p2 = endPos;
            else if (endPos == 0)
            {
                ofs = strlen_P(endH);
                p2 = strpos(buf, tmp, p1 + strlen_P(beginH) + 1);
            }
            else if (endPos == -1)
            {
                beginPos = p1 + strlen_P(beginH);
            }

            if (p2 == -1)
                p2 = strlen(buf);

            delS(tmp);

            if (p2 != -1)
            {
                beginPos = p2 + ofs;
                int len = p2 - p1 - strlen_P(beginH);
                tmp = newS(len + 1);
                memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
                return tmp;
            }
        }

        return nullptr;
    }

    void closeFileHandle(bool sd)
    {
        if (client->_int.esp_line_notify_file)
            client->_int.esp_line_notify_file.close();
        if (sd)
        {
            client->_int.esp_line_notify_sd_used = false;
            client->_int.esp_line_notify_sd_rdy = false;
            SD_FS.end();
        }
    }

    bool stringCompare(const char *buf, int ofs, PGM_P beginH)
    {
        char *tmp = strP(beginH);
        char *tmp2 = newS(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;
        bool ret = (strcmp(tmp, tmp2) == 0);
        delS(tmp);
        delS(tmp2);
        return ret;
    }

    bool setClock(float gmtOffset)
    {
        if (time(nullptr) > client->_int.default_ts && gmtOffset == client->_int.esp_line_notify_gmt_offset)
            return true;

        if (WiFi.status() != WL_CONNECTED)
            WiFi.reconnect();

        time_t now = time(nullptr);

        client->_int.esp_line_notify_clock_rdy = now > client->_int.default_ts;

        if (!client->_int.esp_line_notify_clock_rdy || gmtOffset != client->_int.esp_line_notify_gmt_offset)
        {
            char *server1 = strP(esp_line_notify_str_52);
            char *server2 = strP(esp_line_notify_str_53);

           configTime(gmtOffset * 3600, 0, server1, server2);

            now = time(nullptr);
            unsigned long timeout = millis();
            while (now < client->_int.default_ts)
            {
                now = time(nullptr);
                if (now > client->_int.default_ts || millis() - timeout > client->_int.ntpTimeout)
                    break;
                delay(10);
            }

            delS(server1);
            delS(server2);
        }

        client->_int.esp_line_notify_clock_rdy = now > client->_int.default_ts;
        if (client->_int.esp_line_notify_clock_rdy)
            client->_int.esp_line_notify_gmt_offset = gmtOffset;

        return client->_int.esp_line_notify_clock_rdy;
    }

    bool sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
    {
        if (client)
        {
            client->_int.sd_config.sck = sck;
            client->_int.sd_config.miso = miso;
            client->_int.sd_config.mosi = mosi;
            client->_int.sd_config.ss = ss;
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
    }

    bool flashTest()
    {
#if defined(ESP32)
        if (FORMAT_FLASH == 1)
            client->_int.esp_line_notify_flash_rdy = FLASH_FS.begin(true);
        else
            client->_int.esp_line_notify_flash_rdy = FLASH_FS.begin();
#elif defined(ESP8266)
        client->_int.esp_line_notify_flash_rdy = FLASH_FS.begin();
#endif
        return client->_int.esp_line_notify_flash_rdy;
    }

    bool sdTest(fs::File file)
    {
        std::string filepath = "/sdtest01.txt";

        if (!sdBegin(client->_int.sd_config.ss, client->_int.sd_config.sck, client->_int.sd_config.miso, client->_int.sd_config.mosi))
            return false;

        file = SD_FS.open(filepath.c_str(), FILE_WRITE);
        if (!file)
            return false;

        if (!file.write(32))
        {
            file.close();
            return false;
        }

        file.close();

        file = SD_FS.open(filepath.c_str());
        if (!file)
            return false;

        while (file.available())
        {
            if (file.read() != 32)
            {
                file.close();
                return false;
            }
        }
        file.close();

        SD_FS.remove(filepath.c_str());

        std::string().swap(filepath);

        return true;
    }

    std::string getBoundary(size_t len)
    {
        char *tmp = strP(esp_line_notify_str_54);
        char *buf = newS(len);
        if (len)
        {
            --len;
            buf[0] = tmp[0];
            buf[1] = tmp[1];
            for (size_t n = 2; n < len; n++)
            {
                int key = rand() % (int)(strlen(tmp) - 1);
                buf[n] = tmp[key];
            }
            buf[len] = '\0';
        }
        std::string s = buf;
        delS(buf);
        delS(tmp);
        return s;
    }

    char *intStr(int value)
    {
        char *buf = newS(36);
        memset(buf, 0, 36);
        itoa(value, buf, 10);
        return buf;
    }

    char *boolStr(bool value)
    {
        char *buf = nullptr;
        if (value)
            buf = strP(esp_line_notify_str_58);
        else
            buf = strP(esp_line_notify_str_59);
        return buf;
    }

    std::string url_encode(std::string s)
    {
        const char *str = s.c_str();
        std::vector<char> v(s.size());
        v.clear();
        for (size_t i = 0, l = s.size(); i < l; i++)
        {
            char c = str[i];
            if ((c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
                c == '*' || c == '\'' || c == '(' || c == ')')
            {
                v.push_back(c);
            }
            else if (c == ' ')
            {
                v.push_back('+');
            }
            else
            {
                v.push_back('%');
                unsigned char d1, d2;
                hexchar(c, d1, d2);
                v.push_back(d1);
                v.push_back(d2);
            }
        }

        return std::string(v.cbegin(), v.cend());
    }

    void splitTk(const std::string &str, std::vector<std::string> &tk, const char *delim)
    {
        std::size_t current, previous = 0;
        current = str.find(delim, previous);
        std::string s;
        while (current != std::string::npos)
        {
            s = str.substr(previous, current - previous);
            tk.push_back(s);
            previous = current + strlen(delim);
            current = str.find(delim, previous);
        }
        s = str.substr(previous, current - previous);
        tk.push_back(s);
        std::string().swap(s);
    }

private:
};

#endif