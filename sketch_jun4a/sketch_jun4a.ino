#include <NewPing.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const int TRIGGER_PIN = 5;   // GPIO5 (D1) - Trigger pin of HC-SR04 connected to NodeMCU
const int ECHO_PIN = 4;      // GPIO4 (D2) - Echo pin of HC-SR04 connected to NodeMCU
const int LED_PIN = 0;       // GPIO0 (D3) - LED pin connected to NodeMCU

const int MAX_DISTANCE = 20000; // Maximum distance to measure in centimeters

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
HTTPClient http;

const char* ssid = "DIGISOL";
const char* password = "qas1725utl1";
const String serverName = "http://3.111.108.14:4000/api/sensor/addSensorData";

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  pinMode(LED_PIN, OUTPUT); // Set LED pin as an output

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  delay(3000); // Wait for 15 seconds (15,000 milliseconds)

  unsigned int distance = sonar.ping_cm(); // Send a ping and get the distance in centimeters

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  digitalWrite(LED_PIN, HIGH); // Turn on the LED
  delay(500); // Keep the LED on for 0.5 seconds
  digitalWrite(LED_PIN, LOW); // Turn off the LED

  sendSensorData(distance);
}

void sendSensorData(unsigned int data) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client; // Create a WiFi client

    StaticJsonDocument<200> jsonBuffer;
    jsonBuffer["nameOfSensor"] = "madhura@gmail.com-upper";
    jsonBuffer["data"] = data;
    
    

    // Serialize JSON to a string
    String payload;
    serializeJson(jsonBuffer, payload);

    http.begin(client, serverName); // Use WiFiClient with begin function
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("Sensor data sent successfully");
      Serial.println("Response code: " + String(httpResponseCode));
    } else {
      Serial.println("Error sending sensor data");
      Serial.println("Error code: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
