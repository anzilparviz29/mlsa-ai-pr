#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "time.h"
#include <Base64.h>    // Make sure a Base64 library is installed

// Camera model selection
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Wi-Fi credentials
const char* ssid = "Zub";
const char* password = "anzilabbas";

// Google Vertex AI API details – update these with your project details
const char* google_project_id   = "YOUR_PROJECT_ID";   // Replace with your Google Cloud project ID
const char* google_location     = "YOUR_LOCATION";       // e.g., "us-central1"
const char* google_access_token = "YOUR_ACCESS_TOKEN";   // Obtain via `gcloud auth print-access-token`

// Firebase details for Firestore (using REST API)
const char* firebase_api_key = "AIzaSyBRPrIoHgKPArmijt-LYebG-mkv3Yrhncs"; // Replace with your Firebase API Key
const char* firebase_project_id = "e5ue-f7841";  // Replace with your Firebase Project ID
String collection_name = "zed";                   // Replace with your Firestore collection name

// NTP time settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;  // Adjust for your timezone
const int daylightOffset_sec = 0;

// Function prototypes
void sendImageToGoogle(uint8_t* image_data, size_t image_size);
void storeTextInFirestore(String text, String timestamp);
String getCurrentTimestamp();

void setup() {
  Serial.begin(115200);
  
  // Initialize camera
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
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;
  
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    return;
  }
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Configure time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  // For example, trigger on a button press or any event.
  // Here we capture an image every 10 seconds.
  camera_fb_t* fb = esp_camera_fb_get();
  if (fb) {
    sendImageToGoogle(fb->buf, fb->len);
    esp_camera_fb_return(fb);
  } else {
    Serial.println("Failed to capture image.");
  }
  delay(10000);
}

// Function to send image to Google Vertex AI for captioning
void sendImageToGoogle(uint8_t* image_data, size_t image_size) {
  if (WiFi.status() == WL_CONNECTED) {
    // Encode image to Base64
    String encodedImage = base64::encode(image_data, image_size);
    
    // Build JSON payload according to the API spec
    StaticJsonDocument<1024> doc;
    JsonArray instances = doc.createNestedArray("instances");
    JsonObject instance = instances.createNestedObject();
    JsonObject imageObj = instance.createNestedObject("image");
    imageObj["bytesBase64Encoded"] = encodedImage;
    
    JsonObject parameters = doc.createNestedObject("parameters");
    parameters["sampleCount"] = 1;      // Request one caption (1-3 allowed)
    parameters["language"] = "en";      // Supported languages: en, fr, de, it, es
    
    String payload;
    serializeJson(doc, payload);
    
    // Construct the Vertex AI endpoint URL:
    // Format: https://LOCATION-aiplatform.googleapis.com/v1/projects/PROJECT_ID/locations/LOCATION/publishers/google/models/imagetext:predict
    String google_endpoint = "https://";
    google_endpoint += google_location;
    google_endpoint += "-aiplatform.googleapis.com/v1/projects/";
    google_endpoint += google_project_id;
    google_endpoint += "/locations/";
    google_endpoint += google_location;
    google_endpoint += "/publishers/google/models/imagetext:predict";
    
    HTTPClient http;
    http.begin(google_endpoint);
    http.addHeader("Content-Type", "application/json; charset=utf-8");
    http.addHeader("Authorization", "Bearer " + String(google_access_token));
    
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Google API Response Code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
      
      // Parse the JSON response
      StaticJsonDocument<2048> responseDoc;
      DeserializationError error = deserializeJson(responseDoc, response);
      if (error) {
        Serial.println("deserializeJson() failed");
        http.end();
        return;
      }
      
      // Extract the caption from the predictions array
      JsonArray predictions = responseDoc["predictions"].as<JsonArray>();
      if (predictions.size() > 0) {
        String caption = predictions[0].as<String>();
        Serial.println("Extracted Caption: " + caption);
        
        // Get the current timestamp from NTP
        String timestamp = getCurrentTimestamp();
        Serial.println("Timestamp: " + timestamp);
        
        // Store the caption and timestamp in Firestore
        storeTextInFirestore(caption, timestamp);
      } else {
        Serial.println("No predictions found in response.");
      }
    } else {
      Serial.print("HTTP Error on POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}

// Function to store text and timestamp in Firestore via REST API
void storeTextInFirestore(String text, String timestamp) {
  if (WiFi.status() == WL_CONNECTED) {
    // Construct the Firestore endpoint URL:
    // https://firestore.googleapis.com/v1/projects/PROJECT_ID/databases/(default)/documents/COLLECTION_NAME?key=API_KEY
    String firestoreEndpoint = "https://firestore.googleapis.com/v1/projects/";
    firestoreEndpoint += firebase_project_id;
    firestoreEndpoint += "/databases/(default)/documents/";
    firestoreEndpoint += collection_name;
    firestoreEndpoint += "?key=";
    firestoreEndpoint += firebase_api_key;
    
    // Build the JSON payload to create a document
    StaticJsonDocument<512> doc;
    JsonObject fields = doc.createNestedObject("fields");
    JsonObject textField = fields.createNestedObject("text");
    textField["stringValue"] = text;
    JsonObject timestampField = fields.createNestedObject("timestamp");
    timestampField["stringValue"] = timestamp;
    
    String payload;
    serializeJson(doc, payload);
    
    HTTPClient http;
    http.begin(firestoreEndpoint);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Firestore Response Code: " + String(httpResponseCode));
      Serial.println("Firestore Response: " + response);
    } else {
      Serial.print("Error storing data in Firestore. HTTP Response Code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}

// Function to get the current timestamp from NTP as a formatted string
String getCurrentTimestamp() {
  time_t now;
  time(&now);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char timeStringBuff[30];
  // Format: YYYY-MM-DD HH:MM:SS
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}
