
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
 * This example showed how to send the Line notify message via Ethernet.
 * This example is for ESP32 with LAN8720 Ethernet board.
 */

/**
 * There are may sources for LAN8720 and ESP32 interconnection on the internet which may
 * work for your LAN8720 board.
 *
 * Some methods worked unless no IP is available.
 *
 * This modification and interconnection provided in this example are mostly worked as
 * the 50 MHz clock was created internally in ESP32 which GPIO 17 is set to be output of this clock
 * and feeds to the LAN8720 chip XTAL input.
 *
 * The on-board LAN8720 50 MHz XTAL chip will be disabled by connect its enable pin or pin 1 to GND.
 *
 * Pleae see the images in the folder "modified_LAN8720_board_images" for how to modify the LAN8720 board.
 *
 * The LAN8720 Ethernet modified board and ESP32 board wiring connection.
 *
 * ESP32                        LAN8720
 *
 * GPIO17 - EMAC_CLK_OUT_180    nINT/REFCLK - LAN8720 XTAL1/CLKIN     4k7 Pulldown
 * GPIO22 - EMAC_TXD1           TX1
 * GPIO19 - EMAC_TXD0           TX0
 * GPIO21 - EMAC_TX_EN          TX_EN
 * GPIO26 - EMAC_RXD1           RX1
 * GPIO25 - EMAC_RXD0           RX0
 * GPIO27 - EMAC_RX_DV          CRS
 * GPIO23 - MDC                 MDC
 * GPIO18 - MDIO                MDIO
 * GND                          GND
 * 3V3                          VCC
 *
 */

#include <ESP_Line_Notify.h>

/* Define the LineNotifyClient object */
LineNotifyClient line;

/* Function to print the sending result via Serial (optional) */
void printRessult(LineNotifySendingResult result);

/* The sending callback function (optional) */
void sendingCallback(LineNotifySendingResult result);

#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT // RMII clock output from GPIO17

// Pin# of the enable signal for the external crystal oscillator (-1 to disable)
#define ETH_POWER_PIN -1

// Type of the Ethernet PHY (LAN8720 or TLK110)
#define ETH_TYPE ETH_PHY_LAN8720

// I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#define ETH_ADDR 1

// Pin# of the I²C clock signal for the Ethernet PHY
#define ETH_MDC_PIN 23

// Pin# of the I²C IO signal for the Ethernet PHY
#define ETH_MDIO_PIN 18

static bool eth_connected = false;

unsigned long sendMessagePrevMillis = 0;

#if defined(ESP32)

void WiFiEvent(WiFiEvent_t event)
{
    // Do not run any function here to prevent stack overflow or nested interrupt

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)

    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex())
        {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }

#else
    switch (event)
    {
    case SYSTEM_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex())
        {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case SYSTEM_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
#endif
}

#endif

void sendMessage()
{
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
#if defined(ESP32)
    WiFi.onEvent(WiFiEvent);
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
#endif
}

void loop()
{
#if defined(ESP32)
    if (eth_connected && (millis() - sendMessagePrevMillis > 30000 || sendMessagePrevMillis == 0))
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
