#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"
#include "base64.h"
const int additionalLED = 13;
// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "DIGISOL"; // Enter SSID
const char* password = "qas1725utl1"; // Enter Password
const char* websockets_server_host = "3.111.108.14"; // Enter server address
const uint16_t websockets_server_port = 80; // Enter server port


//websocket
using namespace websockets;
WebsocketsClient websocketclient;

bool wifiConnected = false;
bool isFlashlightOn = false;
bool isBlinking = false;
void startCameraServer();
void reconnectWiFi();
void reconnectWebSocket();

void startCameraServer();
void setupLedFlash(int pin);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector   
  pinMode(LED_GPIO_NUM, OUTPUT); // Set the LED pin as an output
  digitalWrite(LED_GPIO_NUM, LOW); // Ensure the LED starts in the off state
// Ensure the flashlight LED starts in the off state

  pinMode(additionalLED, OUTPUT); // Set the additional LED pin as an output
  digitalWrite(additionalLED, LOW); 
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  config.frame_size = FRAMESIZE_QVGA; // Setting frame size to QVGA (320x240 pixels)

  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 1;
  config.fb_count = 1;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  wifiConnected = true;

   // Connect to WebSocket server
  websocketclient.addHeader("unique-identity", "chitta-admin@gmail.com-galaxy-society-sensor-pool"); // Add the custom header
  websocketclient.addHeader("client-ID", "radhey"); // Add the custom header
  bool connected = websocketclient.connect(websockets_server_host, websockets_server_port, "/ws/binaryHandler");

  reconnectWebSocket();
  websocketclient.onMessage([](WebsocketsMessage message) {
    handleMessage(message);
  });

}

void handleMessage(WebsocketsMessage message) {
  Serial.print("Received message: ");
  Serial.println(message.data());

  // Check if the message contains the "turnOnFlashlight" command
  // Parse the message to check for "turnOnFlashlight"
  if (message.data().indexOf("\"message\":\"turnOnFlashlight\"") != -1) {
    // Turn on the flashlight
    digitalWrite(LED_GPIO_NUM, HIGH); // Assuming LED_GPIO_NUM is the pin connected to the flashlight
    isFlashlightOn = true;
    Serial.println("Flashlight turned on");
  } else if (message.data().indexOf("\"message\":\"turnOffFlashlight\"") != -1) {
    // Turn off the flashlight
    digitalWrite(LED_GPIO_NUM, LOW); // Assuming LED_GPIO_NUM is the pin connected to the flashlight
    isFlashlightOn = false;
    Serial.println("Flashlight turned off");
  }
}

void loop() {
  static unsigned long previousMillis = 0;
  const unsigned long interval = 2000; // 2 seconds interval
  
  unsigned long currentMillis = millis();

   if (!wifiConnected) {
    reconnectWiFi();
  }

  if (!websocketclient.available()) {
    reconnectWebSocket();
  }

  if (currentMillis - previousMillis >= interval) {
    // Capture image here


  }
  
  camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      
       // Send captured image over WebSocket
      esp_camera_fb_return(fb);
      return;
    }
sendImageOverWebSocket(fb->buf, fb->len);
esp_camera_fb_return(fb);
 
  websocketclient.poll();

    previousMillis = currentMillis;
  
  
}



void sendImageOverWebSocket(uint8_t* imgData, size_t imgSize) {
  // Check if the WebSocket client is connected
  delay(3000);
  if (websocketclient.available()) {
    // Cast the uint8_t* to const char* before sending

    websocketclient.sendBinary(reinterpret_cast<const char*>(imgData),imgSize);
    
    Serial.println("Image data sent over WebSocket");
  } else {
    Serial.println("WebSocket not connected. Image data not sent.");
  }
}


void reconnectWiFi() {
  Serial.println("Attempting to reconnect to WiFi...");
  WiFi.reconnect();

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - startAttemptTime > 10000) { // Try for 10 seconds
      Serial.println("WiFi connection failed. Trying again...");
      WiFi.begin(ssid, password);
      startAttemptTime = millis();
    }
  }

  Serial.println("WiFi reconnected!");
  wifiConnected = true;
}

void reconnectWebSocket() {
  Serial.println("Attempting to reconnect to WebSocket...");
  unsigned long startAttemptTime = millis();

  while (!websocketclient.connect(websockets_server_host, websockets_server_port, "/ws/binaryHandler")) {
    delay(500);
    if (millis() - startAttemptTime > 10000) { // Try for 10 seconds
      Serial.println("WebSocket connection failed. Trying again...");
      startAttemptTime = millis();
    }
  }

  Serial.println("WebSocket reconnected!");
}

