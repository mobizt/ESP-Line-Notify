
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2022 mobizt
 *
 */

/**
 * This example showed how to send the Line notify message via Ethernet.
 * This example is for ESP8266 and ENC28J60 Ethernet module.
 */

/**
 *
 * The ENC28J60 Ethernet module and ESP8266 board, SPI port wiring connection.
 *
 * ESP8266 (Wemos D1 Mini or NodeMCU)        ENC28J60
 *
 * GPIO12 (D6) - MISO                        SO
 * GPIO13 (D7) - MOSI                        SI
 * GPIO14 (D5) - SCK                         SCK
 * GPIO16 (D0) - CS                          CS
 * GND                                       GND
 * 3V3                                       VCC
 *
 */

#include <ENC28J60lwIP.h>
//#include <W5100lwIP.h>
//#include <W5500lwIP.h>

#include <ESP_Line_Notify.h>

/* Define the LineNotifyClient object */
LineNotifyClient line;

/* Function to print the sending result via Serial (optional) */
void printRessult(LineNotifySendingResult result);

/* The sending callback function (optional) */
void sendingCallback(LineNotifySendingResult result);

#define ETH_CS_PIN 16 // D0

ENC28J60lwIP eth(ETH_CS_PIN);
// Wiznet5100lwIP eth(ETH_CS_PIN);
// Wiznet5500lwIP eth(ETH_CS_PIN);

unsigned long sendMessagePrevMillis = 0;

void sendMessage()
{
    LineNotify.spi_ethernet_module.enc28j60 = &eth;

    line.reconnect_wifi = true;

    Serial.println("Sending Line Notify message...");

    line.token = "Your Line Notify Access Token";
    line.message = "Hello world";

    LineNotifySendingResult result = LineNotify.send(line);

    // Print the sending result
    printRessult(result);
}

void setup()
{

    Serial.begin(115200);
    Serial.println();
   
#if defined(ESP8266)
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz?
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    eth.setDefault(); // use ethernet for default route
    if (!eth.begin())
    {
        Serial.println("ethernet hardware not found ... sleeping");
        while (1)
        {
            delay(1000);
        }
    }
    else
    {
        Serial.print("connecting ethernet");
        while (!eth.connected())
        {
            Serial.print(".");
            delay(1000);
        }
    }
    Serial.println();
    Serial.print("ethernet IP address: ");
    Serial.println(eth.localIP());
#endif
}

void loop()
{
#if defined(ESP8266)
    if (millis() - sendMessagePrevMillis > 30000 || sendMessagePrevMillis == 0)
    {
        sendMessagePrevMillis = millis();
        sendMessage();
    }
#endif
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
