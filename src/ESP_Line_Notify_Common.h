#ifndef ESP_LINE_NOTIFY_COMMON_H
#define ESP_LINE_NOTIFY_COMMON_H

#include <Arduino.h>
#include <SPI.h>
#include <time.h>
#include <vector>
#include <functional>
#if defined(ESP32)
#include <WiFi.h>
#include "wcs/esp32/LineNotify_HTTPClient32.h"
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include "wcs/esp8266/LineNotify_HTTPClient.h"
#define FS_NO_GLOBALS
#endif

#include <FS.h>
#include "json/FirebaseJson.h"

#define SD_CS_PIN 15

enum esp_line_notify_multipart_header_type
{
    esp_line_notify_multipart_header_type_undefined,
    esp_line_notify_multipart_header_type_message,
    esp_line_notify_multipart_header_type_sticker_package_id,
    esp_line_notify_multipart_header_type_sticker_id,
    esp_line_notify_multipart_header_type_image_thumbnail,
    esp_line_notify_multipart_header_type_image_full_size,
    esp_line_notify_multipart_header_type_notification_disabled

};

typedef enum esp_line_notify_sending_status
{
    LineNotify_Sending_None,
    LineNotify_Sending_Begin,
    LineNotify_Sending_In_Progress,
    LineNotify_Sending_Upload,
    LineNotify_Sending_Error,
    LineNotify_Sending_Success

} LineNotify_Sending_Status;

typedef enum esp_line_notify_storage_type
{
    LineNotify_Storage_Type_Undefined,
    LineNotify_Storage_Type_Flash,
    LineNotify_Storage_Type_SD

} LineNotify_Storage_Type;

struct esp_line_notify_sd_config_info_t
{
    int sck = -1;
    int miso = -1;
    int mosi = -1;
    int ss = -1;
};

struct esp_line_notify_file_info_t
{
    std::string path = "";
    std::string name = "";
    LineNotify_Storage_Type storage_type = LineNotify_Storage_Type_Undefined;
};

struct esp_line_notify_blob_data_info_t
{
    const uint8_t *blob = nullptr;
    size_t size = 0;
    std::string file_name = "";
};

struct esp_line_notify_gmap_info_t
{
    std::string google_api_key = "";
    std::string center = "";
    std::string size = "640x640";
    std::string map_type = "roadmap";
    size_t zoom = 13;
    std::string markers = "";
};

struct esp_line_notify_image_info_t
{
    struct esp_line_notify_blob_data_info_t data;
    struct esp_line_notify_file_info_t file;
    std::string url = "";
};

struct esp_line_notify_sticker_info_t
{
    int package_id = -1;
    int id = -1;
};

struct esp_line_notify_internal_info_t
{
    long default_ts = 1510644967;
    uint16_t ntpTimeout = 20;
    float esp_line_notify_gmt_offset = 0;
    fs::File esp_line_notify_file;
    bool esp_line_notify_sd_used = false;
    bool esp_line_notify_sd_rdy = false;
    bool esp_line_notify_clock_rdy = false;
    bool esp_line_notify_flash_rdy = false;
    int http_code = 0;
    bool http_connected = false;
    uint16_t esp_line_notify_reconnect_tmo = 10000;
    unsigned long esp_line_notify_last_reconnect_millis = 0;
    struct esp_line_notify_sd_config_info_t sd_config;
    int report_state = -1;
    size_t upload_len = 0;
};

struct esp_line_notify_sending_error_info_t
{
    int code = 0;
    std::string message = "";
};

struct esp_line_notify_sending_quota_info_t
{
    int limit = 0;
    int remaining = 0;
};

struct esp_line_notify_sending_quotas_info_t
{
    struct esp_line_notify_sending_quota_info_t text;
    struct esp_line_notify_sending_quota_info_t image;
    size_t reset = 0;
};

typedef struct esp_line_notify_sending_result_info_t
{
    LineNotify_Sending_Status status;
    struct esp_line_notify_sending_error_info_t error;
    struct esp_line_notify_sending_quotas_info_t quota;
    size_t progress = 0;
    std::string file_name = "";

} LineNotifySendingResult;

struct esp_line_notify_server_response_data_t
{
    int httpCode = -1;
    int payloadLen = -1;
    int contentLen = -1;
    int chunkRange = 0;
    int payloadOfs = 0;
    bool isChunkedEnc = false;
    bool noContent = false;
    std::string location = "";
    std::string contentType = "";
    std::string connection = "";
    std::string transferEnc = "";
    struct esp_line_notify_sending_quotas_info_t quota;
};

typedef void (*LineNotifyProgressCallback)(LineNotifySendingResult);

typedef struct esp_line_notify_client_info_t
{
    std::string token = "";
    std::string message = "";
    struct esp_line_notify_sticker_info_t sticker;
    struct esp_line_notify_image_info_t image;
    struct esp_line_notify_gmap_info_t gmap;
    struct esp_line_notify_internal_info_t _int;
    bool reconnect_wifi = false;
    bool notification_disabled = false;
    uint16_t response_size = 2048;
#if defined(ESP32)
    ESP_LN_HTTPClient32 *httpClient;
#elif defined(ESP8266)
    ESP_LN_HTTPClient *httpClient;
#endif
    LineNotifyProgressCallback sendingg_callback = NULL;
} LineNotiFyClient;

static const char esp_line_notify_str_1[] PROGMEM = "notify-api.line.me";
static const char esp_line_notify_str_2[] PROGMEM = " ";
static const char esp_line_notify_str_3[] PROGMEM = "ESP32";
static const char esp_line_notify_str_4[] PROGMEM = "\r\n";
static const char esp_line_notify_str_5[] PROGMEM = "POST /api/notify HTTP/1.1";
static const char esp_line_notify_str_6[] PROGMEM = "cache-control: no-cache";
static const char esp_line_notify_str_7[] PROGMEM = "Authorization: Bearer ";
static const char esp_line_notify_str_8[] PROGMEM = "Content-Type: multipart/form-data; boundary=";
static const char esp_line_notify_str_9[] PROGMEM = "User-Agent: ";
static const char esp_line_notify_str_10[] PROGMEM = "Accept: */*";
static const char esp_line_notify_str_11[] PROGMEM = "Host: ";
static const char esp_line_notify_str_12[] PROGMEM = "accept-encoding: gzip, deflate";
static const char esp_line_notify_str_13[] PROGMEM = "Connection: close";
static const char esp_line_notify_str_14[] PROGMEM = "content-length: ";
static const char esp_line_notify_str_15[] PROGMEM = "/api/notify";
static const char esp_line_notify_str_16[] PROGMEM = "Content-Disposition: form-data; name=\"imageFile\"; filename=\"";
static const char esp_line_notify_str_17[] PROGMEM = "Content-Type: ";
static const char esp_line_notify_str_18[] PROGMEM = "Content-Disposition: form-data; name=\"";
static const char esp_line_notify_str_19[] PROGMEM = "message";
static const char esp_line_notify_str_20[] PROGMEM = "stickerPackageId";
static const char esp_line_notify_str_21[] PROGMEM = "stickerId";
static const char esp_line_notify_str_22[] PROGMEM = "imageThumbnail";
static const char esp_line_notify_str_23[] PROGMEM = "imageFullsize";
static const char esp_line_notify_str_24[] PROGMEM = "--";
static const char esp_line_notify_str_25[] PROGMEM = "\"";
static const char esp_line_notify_str_26[] PROGMEM = ".jpg";
static const char esp_line_notify_str_27[] PROGMEM = ".jpeg";
static const char esp_line_notify_str_28[] PROGMEM = "image/jpeg";
static const char esp_line_notify_str_29[] PROGMEM = ".gif";
static const char esp_line_notify_str_30[] PROGMEM = "image/gif";
static const char esp_line_notify_str_31[] PROGMEM = ".png";
static const char esp_line_notify_str_32[] PROGMEM = "image/png";
static const char esp_line_notify_str_33[] PROGMEM = ".bmp";
static const char esp_line_notify_str_34[] PROGMEM = "image/bmp";
static const char esp_line_notify_str_35[] PROGMEM = ".";
static const char esp_line_notify_str_36[] PROGMEM = "HTTP/1.1 ";
static const char esp_line_notify_str_37[] PROGMEM = " ";
static const char esp_line_notify_str_38[] PROGMEM = "X-RateLimit-Limit: ";
static const char esp_line_notify_str_39[] PROGMEM = "X-RateLimit-ImageLimit: ";
static const char esp_line_notify_str_40[] PROGMEM = "X-RateLimit-Remaining: ";
static const char esp_line_notify_str_41[] PROGMEM = "X-RateLimit-ImageRemaining: ";
static const char esp_line_notify_str_42[] PROGMEM = "X-RateLimit-Reset: ";
static const char esp_line_notify_str_43[] PROGMEM = "/esp";
static const char esp_line_notify_str_44[] PROGMEM = "read timed out";
static const char esp_line_notify_str_45[] PROGMEM = "status";
static const char esp_line_notify_str_46[] PROGMEM = "Connection: ";
static const char esp_line_notify_str_47[] PROGMEM = "Content-Length: ";
static const char esp_line_notify_str_48[] PROGMEM = ";";
static const char esp_line_notify_str_49[] PROGMEM = "Location: ";
static const char esp_line_notify_str_50[] PROGMEM = "Transfer-Encoding: ";
static const char esp_line_notify_str_51[] PROGMEM = "chunked";
static const char esp_line_notify_str_52[] PROGMEM = "pool.ntp.org";
static const char esp_line_notify_str_53[] PROGMEM = "time.nist.gov";
static const char esp_line_notify_str_54[] PROGMEM = "=_abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char esp_line_notify_str_55[] PROGMEM = "File I/O error";
static const char esp_line_notify_str_56[] PROGMEM = "connection closed";
static const char esp_line_notify_str_57[] PROGMEM = "notificationDisabled";
static const char esp_line_notify_str_58[] PROGMEM = "true";
static const char esp_line_notify_str_59[] PROGMEM = "false";
static const char esp_line_notify_str_60[] PROGMEM = "https://maps.googleapis.com/maps/api/staticmap?key=";
static const char esp_line_notify_str_61[] PROGMEM = "https://www.google.com/maps/@?api=1&map_action=map&center=";
static const char esp_line_notify_str_62[] PROGMEM = "&center=";
static const char esp_line_notify_str_63[] PROGMEM = "&zoom=";
static const char esp_line_notify_str_64[] PROGMEM = "&maptype=";
static const char esp_line_notify_str_65[] PROGMEM = "&size=";
static const char esp_line_notify_str_66[] PROGMEM = "&markers=";
static const char esp_line_notify_str_67[] PROGMEM = "&basemap=";
static const char esp_line_notify_str_68[] PROGMEM = "\n\n";
#endif