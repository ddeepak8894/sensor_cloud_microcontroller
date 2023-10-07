#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NewPing.h> // Include the NewPing library for the ultrasonic sensor
#include <ArduinoJson.h>

const char *ssid = "DIGISOL";
const char *password = "qas1725utl1";
const char *mqtt_server = "3.111.108.14";
const char *sensorName = "sensor_data/vijay@gmail.com-block-43-upper-tank";
const int externalLedPin = 0; // GPIO0 (D3) - LED pin connected to NodeMCU
const int TRIGGER_PIN = 5;    // GPIO5 (D1) - Trigger pin of HC-SR04 connected to NodeMCU
const int ECHO_PIN = 4;       // GPIO4 (D2) - Echo pin of HC-SR04 connected to NodeMCU
const int MAX_DISTANCE = 200; // Maximum distance to measure in centimeters

WiFiClient espClient;

PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an '1' was received as the first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);
    digitalWrite(externalLedPin, LOW);
  } else {
    digitalWrite(BUILTIN_LED, HIGH);
    digitalWrite(externalLedPin, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), "root", "manager")) {
      Serial.println("connected");
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(externalLedPin, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    int distance = sonar.ping_cm();

    // Create the JSON payload
    String jsonPayload = createJsonPayload( distance,40);

    Serial.print("Publish message: ");
    Serial.println(jsonPayload);

    // Publish the JSON payload to the MQTT topic
    client.publish(sensorName, jsonPayload.c_str());
  }
}

String createJsonPayload(int distance, int maxValue) {
  StaticJsonDocument<200> jsonDoc;
  
  jsonDoc["data"] = distance;
  jsonDoc["maxValue"] = maxValue;

  String payload;
  serializeJson(jsonDoc, payload);

  return payload;
}



