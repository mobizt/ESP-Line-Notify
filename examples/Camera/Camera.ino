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
 * This example showed how to send image from camera as a message via the Line Notify agent.
 * The callback function and sending result can be assigned
 * 
*/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ESP_Line_Notify.h>

//The OV2640 library
#include "cam/OV2640.h"

/* Set your WiFI AP credential */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* Define the OV2640 object */
OV2640 cam;

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

  camera_config_t camera_config;

  /** For M5Stack M5Cam - ESP32 Camera (OV2640)
   * Change to match your pin configuration between OV2640 Camera and ESP32 connection
  */
  camera_config.ledc_channel = LEDC_CHANNEL_0;
  camera_config.ledc_timer = LEDC_TIMER_0;
  camera_config.pin_d0 = 17;
  camera_config.pin_d1 = 35;
  camera_config.pin_d2 = 34;
  camera_config.pin_d3 = 5;
  camera_config.pin_d4 = 39;
  camera_config.pin_d5 = 18;
  camera_config.pin_d6 = 36;
  camera_config.pin_d7 = 19;
  camera_config.pin_xclk = 27;
  camera_config.pin_pclk = 21;
  camera_config.pin_vsync = 22;
  camera_config.pin_href = 26;
  camera_config.pin_sscb_sda = 25;
  camera_config.pin_sscb_scl = 23;
  camera_config.pin_reset = 15;
  camera_config.xclk_freq_hz = 20000000;

  camera_config.pixel_format = CAMERA_PF_JPEG;
  camera_config.frame_size = CAMERA_FS_SVGA;

  cam.init(camera_config);

  delay(100);

  cam.run();

  client.reconnect_wifi = true;

  Serial.println("Sending Line Notify message...");

  client.token = "Your Line Notify Access Token";
  client.message = "This is camera image";

  client.image.data.blob = cam.getfb();
  client.image.data.size = cam.getSize();
  client.image.data.file_name = "camera.jpg";

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
