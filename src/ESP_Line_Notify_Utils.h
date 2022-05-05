/**
 * ESP_Line_Notify_Utils class, ESP_Line_Notify_Utils.h version 1.0.2
 *
 *
 * Created May 5, 2022
 *
 * This work is a part of ESP Line Notify library
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

#ifndef ESP_Line_Notify_Utils_H
#define ESP_Line_Notify_Utils_H

#include <Arduino.h>
#include "ESP_Line_Notify_Const.h"

class ESP_Line_Notify_Utils
{
    friend class ESP_Line_Notify;

public:
    LineNotifyClient *client = nullptr;
    MB_FS *mbfs = nullptr;

    ESP_Line_Notify_Utils(LineNotifyClient *c, MB_FS *m)
    {
        client = c;
        mbfs = m;
    };

    ~ESP_Line_Notify_Utils(){};

    char *strP(PGM_P pgm)
    {
        size_t len = strlen_P(pgm) + 5;
        char *buf = (char *)mbfs->newP(len);
        memset(buf, 0, len);
        strcpy_P(buf, pgm);
        return buf;
    }

    int strposP(const char *buf, PGM_P beginH, int ofs)
    {
        char *tmp = strP(beginH);
        int p = strpos(buf, tmp, ofs);
        mbfs->delP(&tmp);
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
        char *tmp2 = (char *)mbfs->newP(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;
        bool ret = (strcasecmp(tmp, tmp2) == 0);
        mbfs->delP(&tmp);
        mbfs->delP(&tmp2);
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
            tmp = (char *)mbfs->newP(len + 1);
            memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
            return tmp;
        }

        return nullptr;
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
        char *_haystack = (char *)mbfs->newP(len - offset + 1);
        _haystack[len - offset] = 0;
        strncpy(_haystack, haystack + offset, len - offset);
        char *p = strstr(_haystack, needle);
        int r = -1;
        if (p)
            r = p - _haystack + offset;
        mbfs->delP(&_haystack);
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
        char *_haystack = (char *)mbfs->newP(len - offset + 1);
        _haystack[len - offset] = 0;
        strncpy(_haystack, haystack + offset, len - offset);
        char *p = rstrstr(_haystack, needle);
        int r = -1;
        if (p)
            r = p - _haystack + offset;
        mbfs->delP(&_haystack);
        return r;
    }

    void ltrim(MB_String &str, const MB_String &chars = " ")
    {
        size_t pos = str.find_first_not_of(chars);
        if (pos != MB_String::npos)
            str.erase(0, pos);
    }

    void rtrim(MB_String &str, const MB_String &chars = " ")
    {
        size_t pos = str.find_last_not_of(chars);
        if (pos != MB_String::npos)
            str.erase(pos + 1);
    }

    inline MB_String trim(const MB_String &s)
    {
        MB_String chars = " ";
        MB_String str = s;
        ltrim(str, chars);
        rtrim(str, chars);
        return str;
    }

    void substr(MB_String &str, const char *s, int offset, size_t len)
    {
        if (!s)
            return;

        int slen = strlen(s);

        if (slen == 0)
            return;

        int last = offset + len;

        if (offset >= slen || len == 0 || last > slen)
            return;

        for (int i = offset; i < last; i++)
            str += s[i];
    }

    char *newS(char *p, size_t len)
    {
        mbfs->delP(&p);
        p = (char *)mbfs->newP(len);
        return p;
    }

    char *newS(char *p, size_t len, char *d)
    {
        mbfs->delP(&p);
        p = (char *)mbfs->newP(len);
        strcpy(p, d);
        return p;
    }

    inline int ishex(int x)
    {
        return (x >= '0' && x <= '9') ||
               (x >= 'a' && x <= 'f') ||
               (x >= 'A' && x <= 'F');
    }

    void hexchar(char c, char &hex1, char &hex2)
    {
        hex1 = c / 16;
        hex2 = c % 16;
        hex1 += hex1 < 10 ? '0' : 'A' - 10;
        hex2 += hex2 < 10 ? '0' : 'A' - 10;
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
                mbfs->delP(&tmp);
            }
            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_17, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.contentType = tmp;
                mbfs->delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_47, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.contentLen = atoi(tmp);
                mbfs->delP(&tmp);
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
                mbfs->delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_46, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.connection = tmp;
                mbfs->delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_38, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.text.limit = atoi(tmp);
                mbfs->delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_39, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.image.limit = atoi(tmp);
                mbfs->delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_40, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.text.remaining = atoi(tmp);
                mbfs->delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_41, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.image.remaining = atoi(tmp);
                mbfs->delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_42, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {
                response.quota.reset = atoi(tmp);
                mbfs->delP(&tmp);
            }

            if (pmax < beginPos)
                pmax = beginPos;
            beginPos = payloadPos;
            tmp = getHeader(buf, esp_line_notify_str_47, esp_line_notify_str_4, beginPos, 0);
            if (tmp)
            {

                response.payloadLen = atoi(tmp);
                mbfs->delP(&tmp);
            }

            if (response.httpCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_OK || response.httpCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_TEMPORARY_REDIRECT || response.httpCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_PERMANENT_REDIRECT || response.httpCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_MOVED_PERMANENTLY || response.httpCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_FOUND)
            {
                if (pmax < beginPos)
                    pmax = beginPos;
                beginPos = payloadPos;
                tmp = getHeader(buf, esp_line_notify_str_49, esp_line_notify_str_4, beginPos, 0);
                if (tmp)
                {
                    response.location = tmp;
                    mbfs->delP(&tmp);
                }
            }

            if (response.httpCode == ESP_LINE_NOTIFY_ERROR_HTTP_CODE_NO_CONTENT)
                response.noContent = true;
        }
    }

    char *getHeader(const char *buf, PGM_P beginH, PGM_P endH, int &beginPos, int endPos)
    {

        char *tmp = strP(beginH);
        int p1 = strpos(buf, tmp, beginPos);
        int ofs = 0;
        mbfs->delP(&tmp);
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

            mbfs->delP(&tmp);

            if (p2 != -1)
            {
                beginPos = p2 + ofs;
                int len = p2 - p1 - strlen_P(beginH);
                tmp = (char *)mbfs->newP(len + 1);
                memcpy(tmp, &buf[p1 + strlen_P(beginH)], len);
                return tmp;
            }
        }

        return nullptr;
    }

    bool stringCompare(const char *buf, int ofs, PGM_P beginH)
    {
        char *tmp = strP(beginH);
        char *tmp2 = (char *)mbfs->newP(strlen_P(beginH) + 1);
        memcpy(tmp2, &buf[ofs], strlen_P(beginH));
        tmp2[strlen_P(beginH)] = 0;
        bool ret = (strcmp(tmp, tmp2) == 0);
        mbfs->delP(&tmp);
        mbfs->delP(&tmp2);
        return ret;
    }

    bool setClock(float gmtOffset)
    {
        uint32_t now = 0;

#if defined(ESP32) || defined(ESP8266)
            if (time(nullptr) > client->internal.default_ts && gmtOffset == client->internal.esp_line_notify_gmt_offset) return true;

        if (WiFi.status() != WL_CONNECTED)
            WiFi.reconnect();

         now = time(nullptr);

        client->internal.esp_line_notify_clock_rdy = now > client->internal.default_ts;

        if (!client->internal.esp_line_notify_clock_rdy || gmtOffset != client->internal.esp_line_notify_gmt_offset)
        {

            configTime(gmtOffset * 3600, 0, "pool.ntp.org", "time.nist.gov");

            now = time(nullptr);
            unsigned long timeout = millis();
            while (now < client->internal.default_ts)
            {
                now = time(nullptr);
                if (now > client->internal.default_ts || millis() - timeout > client->internal.ntpTimeout)
                    break;
                delay(0);
            }
        }

#endif

        client->internal.esp_line_notify_clock_rdy = now > client->internal.default_ts;
        if (client->internal.esp_line_notify_clock_rdy)
            client->internal.esp_line_notify_gmt_offset = gmtOffset;

        return client->internal.esp_line_notify_clock_rdy;
    }

    MB_String getBoundary(size_t len)
    {
        char *tmp = strP(esp_line_notify_str_54);
        char *buf = (char *)mbfs->newP(len);
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
        MB_String s = buf;
        mbfs->delP(&buf);
        mbfs->delP(&tmp);
        return s;
    }

    MB_String url_encode(const MB_String &s)
    {
        MB_String ret;
        ret.reserve(s.length() * 3 + 1);
        for (size_t i = 0, l = s.size(); i < l; i++)
        {
            char c = s[i];
            if ((c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
                c == '*' || c == '\'' || c == '(' || c == ')')
            {
                ret += c;
            }
            else
            {
                ret += '%';
                char d1, d2;
                hexchar(c, d1, d2);
                ret += d1;
                ret += d2;
            }
        }
        ret.shrink_to_fit();
        return ret;
    }

    void splitTk(const MB_String &str, MB_VECTOR<MB_String> &tk, const char *delim)
    {
        size_t current, previous = 0;
        current = str.find(delim, previous);
        MB_String s;
        MB_String _str = str;
        while (current != MB_String::npos)
        {
            s = _str.substr(previous, current - previous);
            tk.push_back(s);
            previous = current + strlen(delim);
            current = str.find(delim, previous);
        }
        s = _str.substr(previous, current - previous);
        tk.push_back(s);
        s.clear();
    }

private:
};

#endif