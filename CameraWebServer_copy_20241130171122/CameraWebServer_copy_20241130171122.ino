#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "time.h"

// Camera model selection
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Wi-Fi credentials
const char* ssid = "Zub";
const char* password = "anzilabbas";

// Azure Computer Vision API details
const char* vision_api_endpoint = "https://maaasvision.cognitiveservices.azure.com/vision/v3.2/analyze?visualFeatures=Description";
const char* subscription_key = "613a136c3958454d92e13fee60e5dd0e";

// Firebase details
const char* api_key = "AIzaSyBRPrIoHgKPArmijt-LYebG-mkv3Yrhncs"; // Replace with your Firebase API Key
const char* project_id = "e5ue-f7841"; // Replace with your Firebase Project ID
String collection_name = "zed"; // Replace with your Firestore collection name

// NTP time settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // Adjust for your timezone
const int daylightOffset_sec = 0;

// Function prototypes
void startCameraServer();
void sendImageToAzure(uint8_t* image_data, size_t image_size);
void storeTextInFirestore(String text, String timestamp);
String getCurrentTimestamp();

// Function to send image to Azure Computer Vision API
void sendImageToAzure(uint8_t* image_data, size_t image_size) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(vision_api_endpoint);
    http.addHeader("Content-Type", "application/octet-stream");
    http.addHeader("Ocp-Apim-Subscription-Key", subscription_key);

    int httpResponseCode = http.POST(image_data, image_size);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);

      // Parse the JSON response
      StaticJsonDocument<1024> doc;
      deserializeJson(doc, response);

      // Extract the description text from the response
      String text = doc["description"]["captions"][0]["text"].as<String>();
      Serial.println("Extracted text: " + text);

      // Get the current timestamp
      String timestamp = getCurrentTimestamp();
      Serial.println("Timestamp: " + timestamp);

      // Store the extracted text and timestamp in Firestore
      storeTextInFirestore(text, timestamp);

    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}

// Function to get the current timestamp from NTP







String getCurrentTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "N/A";
  }

  char timestamp[50];
  strftime(timestamp, sizeof(timestamp), "%B %d, %Y at %I:%M:%S %p UTC%z", &timeinfo);

  // Insert colon in the UTC offset (e.g., "+0530" -> "+05:30")
  String formattedTime = timestamp;
  formattedTime = formattedTime.substring(0, formattedTime.length() - 2) + ":" +
                  formattedTime.substring(formattedTime.length() - 2);

  return formattedTime;
}

// Function to store extracted text and timestamp in Firestore
void storeTextInFirestore(String text, String timestamp) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "https://firestore.googleapis.com/v1/projects/" + String(project_id) + "/databases/(default)/documents/" + collection_name;

   String json_payload = "{ \"fields\": { " "\"name\": { \"stringValue\": \"" + text + "\" }, " "\"timestamp\": { \"stringValue\": \"" + timestamp + "\" } " "} }";


    http.begin(url.c_str());
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(json_payload);

    if (httpResponseCode > 0) {
      Serial.println("Data sent successfully to Firestore!");
      Serial.println(httpResponseCode);
      Serial.println(http.getString());
    } else {
      Serial.print("Error storing data in Firestore: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize NTP for timestamp retrieval
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Camera initialization code
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Init the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // Capture frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (fb) {
    sendImageToAzure(fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }
  delay(10000); // Adjust delay as needed
}
