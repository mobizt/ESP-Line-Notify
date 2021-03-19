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
 * This example showed how to send the notified message via the Line Notify agent.
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
LineNotiFyClient client;

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
   
    client.notification_disabled = true;

  */

  /** To send sticker
   * See https://devdocs.line.me/files/sticker_list.pdf for STKPKGID (Sticker Package ID).
   * See https://devdocs.line.me/files/sticker_list.pdf for STKID (Sticker ID).
  
    client.sticker.package_id = 2;
    client.sticker.id = 157;

  */

  /** To send image from URL
   
    client.image.url = "https://cdn.pixabay.com/photo/2017/05/17/18/35/sewing-2321532_640.jpg";

  */

  /** To send image from flash or memory
   
    client.image.data.blob = (uint8_t *)dummyImageData;
    client.image.data.size = sizeof(dummyImageData);
    client.image.data.file_name = "image.jpg";

  */

  /** To send image from file systems (flash or SD)
   * 
   * The SPIFFS and SD file systems are enabled by default.
   * To use other File systems, edit the FS_Config.h to include the header files 
   * of File systems and defined the machro DEFAULT_FLASH_FS and DEFAULT_SD_FS with the class of File systems.

   
   client.image.file.path = "/test.jpg";
   client.image.file.storage_type = LineNotify_Storage_Type_Flash; // LineNotify_Storage_Type_Flash or LineNotify_Storage_Type_SD
   client.image.file.name = "test.jpg";

  */

  /** To assiggn the callback function
   
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