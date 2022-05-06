/**
 * ESP Line Notify TCP Client v1.0.0
 *
 * Created May 6, 2022
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
 *
 * TCPClient Arduino library for ESP32
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the TCPClient for Arduino.
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

#ifndef ESP_Line_Notify_CUSTOM_TCP_Client_H
#define ESP_Line_Notify_CUSTOM_TCP_Client_H

// This file was included in wcs/clients.h

#include <Arduino.h>
#include <SPI.h>
#include "./wcs/base/ESP_Line_Notify_TCP_Client_Base.h"

class ESP_Line_Notify_Custom_TCP_Client : public ESP_Line_Notify_TCP_Client_Base
{
    friend class ESP_Line_Notify;

public:
    ESP_Line_Notify_Custom_TCP_Client(){};
    ~ESP_Line_Notify_Custom_TCP_Client(){};

    void setCACert(const char *caCert) {}

    bool setCertFile(const char *certFile, mb_fs_mem_storage_type storageType) { return false; }

    void setTimeout(uint32_t timeoutmSec)
    {
        baseSetTimeout(timeoutmSec);
    }

    void ethDNSWorkAround()
    {
    }

    bool networkReady()
    {
        if (network_status_cb)
            network_status_cb();

        return networkStatus;
    }

    void networkReconnect()
    {
        if (network_connection_cb)
            network_connection_cb();
    }

    void networkDisconnect()
    {
    }

    esp_line_notify_tcp_client_type type()
    {
        return esp_line_notify_tcp_client_type_external;
    }

    bool isInitialized()
    {
        return this->client != nullptr && tcp_connection_cb != NULL && network_connection_cb != NULL;
    }

    int hostByName(const char *name, IPAddress &ip)
    {
        // return WiFi.hostByName(name, ip);
        return 1;
    }

    bool connect()
    {
       
        if (!client)
            return false;

        if (connected())
        {
            flush();
            return true;
        }

#if !defined(ESP_LINE_NOTIFY_ENABLE_EXTERNAL_CLIENT)
        return setError(ESP_LINE_NOTIFY_ERROR_EXTERNAL_CLIENT_DISABLED);
#endif
        if (!isInitialized())
            return setError(ESP_LINE_NOTIFY_ERROR_EXTERNAL_CLIENT_NOT_INITIALIZED);

        networkReady();

        if (this->tcp_connection_cb)
            this->tcp_connection_cb(host.c_str(), port);

        return connected();
    }

    void setClient(Client *client)
    {
        this->client = client;
    }

    void tcpConnectionRequestCallback(ESP_Line_Notify_TCPConnectionRequestCallback tcpConnectionCB)
    {
        this->tcp_connection_cb = tcpConnectionCB;
    }

    void networkConnectionRequestCallback(ESP_Line_Notify_NetworkConnectionRequestCallback networkConnectionCB)
    {
        this->network_connection_cb = networkConnectionCB;
    }

    void networkStatusRequestCallback(ESP_Line_Notify_NetworkStatusRequestCallback networkStatusCB)
    {
        this->network_status_cb = networkStatusCB;
    }

    void setNetworkStatus(bool status)
    {
        networkStatus = status;
    }

private:
    ESP_Line_Notify_TCPConnectionRequestCallback tcp_connection_cb = NULL;
    ESP_Line_Notify_NetworkConnectionRequestCallback network_connection_cb = NULL;
    ESP_Line_Notify_NetworkStatusRequestCallback network_status_cb = NULL;
    volatile bool networkStatus = false;
};

#endif