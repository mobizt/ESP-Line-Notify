# LINE Notify Arduino Library for ESP8266 and ESP32 version 1.0.0

This Arduino library allows ESp8266 and ESP32 to send LINE Notify message, sticker and images from flash and SD memory.

The library was test and work well with ESP8266 and ESP32 based module.

Copyright (c) 2021 K. Suwatchai (Mobizt).


## Tested Devices

This following devices were tested and work well.

 * Wemos D1 Mini
 * NodeMCU-32
 * WEMOS LOLIN32


 
## Features

* Support sending text, sticker and images from flash and SD memory.

* Show upload progress via callback function.

* Flash and SD File systems can be configured to use LittleFS, FFat and SD_MMC



## Installation


Click on **Clone or download** dropdown at the top of repository, select **Download ZIP** and save file on your computer.

From Arduino IDE, goto menu **Sketch** -> **Include Library** -> **Add .ZIP Library...** and choose **ESP-Line-Notify-main.zip** that previously downloaded.

Go to menu **Files** -> **Examples** -> **ESP-Line-Notify-main** and choose one from examples



## Usages

```Cpp

  #include <ESP_Line_Notify.h>

  /* Define the LineNotifyClient object */
  LineNotiFyClient client;


  client.token = "Your Line Notify Access Token";
  client.message = "Hello world";

  LineNotiFySendingResult result = lineNotify.send(client);

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

```


See [full examples](/examples) for complete usages.



## All Supported Functions


#### Send the Line Notify message.

param **`client`** The LineNotiFyClient data which contains TCP client with message data

return **`LineNotiFySendingResult`** type data to get the sending result.

```C++
LineNotiFySendingResult send(LineNotiFyClient &client);
```




#### SD card config with GPIO pins.

param **`ss`** SPI Chip/Slave Select pin.

param **`sck`** SPI Clock pin.

param **`miso`** SPI MISO pin.

param **`mosi`** SPI MOSI pin.

return **`Boolean`** type status indicates the success of the operation.

```C++
bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);
```

   



## License

The MIT License (MIT)

Copyright (c) 2021 K. Suwatchai (Mobizt)


Permission is hereby granted, free of charge, to any person returning a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

