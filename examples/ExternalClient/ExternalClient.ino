
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/ESP-Line-Notify
 *
 * Copyright (c) 2022 mobizt
 *
 */

/**
 * This example showed how to send the notified message via the Line Notify agent
 * with external Client.
 * This example used SAMD21 device and WiFiNINA as the client.
 *
 */

#if defined(ARDUINO_ARCH_SAMD)
#include <WiFiNINA.h>
#endif

#include <ESP_Line_Notify.h>

/* Set your WiFI AP credential */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* Define the LineNotifyClient object */
LineNotifyClient line;

/* Function to print the sending result via Serial (optional) */
void printRessult(LineNotifySendingResult result);

/* The sending callback function (optional) */
void sendingCallback(LineNotifySendingResult result);

WiFiSSLClient client;

void networkConnection()
{
    // Reset the network connection
    WiFi.disconnect();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
        if (millis() - ms >= 5000)
        {
            Serial.println(" failed!");
            return;
        }
    }
    Serial.println();
    Serial_Printf("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

// Define the callback function to handle server status acknowledgement
void networkStatusRequestCallback()
{
    // Set the network status
    line.setNetworkStatus(WiFi.status() == WL_CONNECTED);
}

// Define the callback function to handle server connection
void tcpConnectionRequestCallback(const char *host, int port)
{
    Serial.print("Connecting to server via external Client... ");
    if (!client.connect(host, port))
    {
        Serial.println("failed.");
        return;
    }
    Serial.println("success.");
}

void setup()
{

    Serial.begin(115200);

#if defined(ARDUINO_ARCH_SAMD)
    while (!Serial)
        ;
#endif

    networkConnection();

    /* line.setExternalClient and line.setExternalClientCallbacks must be called before Firebase.begin */

    /* Assign the pointer to global defined WiFiClient object */
    line.setExternalClient(&client);

    /* Assign the required callback functions */
    line.setExternalClientCallbacks(tcpConnectionRequestCallback, networkConnection, networkStatusRequestCallback);

    Serial.println("Sending Line Notify message...");

    line.token = "Your Line Notify Access Token";
    line.message = "Hello world";

    LineNotifySendingResult result = LineNotify.send(line);

    // Print the sending result
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