/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

/**
 * This example showed how to send the location and google map image message via the Line Notify agent.
 * The callback function and sending result can be assigned
 * 
*/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ESP_Line_Notify.h>

//Demo image data
#include "image.h"

/* Set your WiFI AP credential */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* Define the LineNotifyClient object */
LineNotifyClient client;

/* Function to print the sending result via Serial (optional) */
void printRessult(LineNotifySendingResult result);

/* The sending callback function (optional) */
void sendingCallback(LineNotifySendingResult result);

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(200);
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    client.reconnect_wifi = true;

    Serial.println("Sending Line Notify message...");

    client.token = "Your Line Notify Access Token";
    client.message = "Hello world";

    /** To send message without user notification
     * 
     client.notification_disabled = true;

    */

    client.message = "Location";

    client.gmap.zoom = 18;
    client.gmap.map_type = "satellite";          //roadmap or satellite
    client.gmap.center = "40.718217,-73.998284"; //Places or Latitude, Longitude

    /** To send the map image, Google Map Static API must be enable
     * 
     * To enable Map Static API
     * https://console.cloud.google.com/apis/library/static-maps-backend.googleapis.com
     * 
     * To create the API key
     * https://developers.google.com/maps/documentation/javascript/get-api-key
     

     client.gmap.google_api_key = "Your API Key";
     client.gmap.size = "640x6400";//size of map image in pixels (widthxheight), 640x640 is maximum
     client.gmap.markers = "color:green|label:P|40.718217,-73.998284"; //the marker in static map image: color, label, //Latitude, Longitude

    */

    /** To assiggn the callback function
     * 
     client.sendingg_callback = sendingCallback;

    */

    LineNotifySendingResult result = LineNotify.send(client);

    //Print the sending result
    printRessult(result);
}

void loop()
{
}

/* Function to print the sending result via Serial */
void printRessult(LineNotifySendingResult result)
{
    if (result.status == LineNotify_Sending_Success)
    {
        Serial.printf("Status: %s\n", "success");
        Serial.printf("Text limit: %d\n", result.quota.text.limit);
        Serial.printf("Text remaining: %d\n", result.quota.text.remaining);
        Serial.printf("Image limit: %d\n", result.quota.image.limit);
        Serial.printf("Image remaining: %d\n", result.quota.image.remaining);
        Serial.printf("Reset: %d\n", result.quota.reset);
    }
    else if (result.status == LineNotify_Sending_Error)
    {
        Serial.printf("Status: %s\n", "error");
        Serial.printf("error code: %d\n", result.error.code);
        Serial.printf("error msg: %s\n", result.error.message.c_str());
    }
}

/* The sending callback function (optional) */
void sendingCallback(LineNotifySendingResult result)
{
    if (result.status == LineNotify_Sending_Begin)
    {
        Serial.println("Sending begin");
    }
    else if (result.status == LineNotify_Sending_Upload)
    {
        Serial.printf("Uploaded %s, %d%s\n", result.file_name.c_str(), (int)result.progress, "%");
    }
    else if (result.status == LineNotify_Sending_Success)
    {
        Serial.println("Sending success\n\n");
        Serial.printf("Text limit: %d\n", result.quota.text.limit);
        Serial.printf("Text remaining: %d\n", result.quota.text.remaining);
        Serial.printf("Image limit: %d\n", result.quota.image.limit);
        Serial.printf("Image remaining: %d\n", result.quota.image.remaining);
        Serial.printf("Reset: %d\n", result.quota.reset);
    }
    else if (result.status == LineNotify_Sending_Error)
    {
        Serial.println("Sending failed\n\n");
        Serial.printf("error code: %d\n", result.error.code);
        Serial.printf("error msg: %s\n", result.error.message.c_str());
    }
}