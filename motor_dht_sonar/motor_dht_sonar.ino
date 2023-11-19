#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NewPing.h> // Include the NewPing library for the ultrasonic sensor
#include <ArduinoJson.h>
#include <DHT.h>     // Include the DHT library for the DHT11 sensor

// const char *ssid = "www.dolphinlabs.in";
// const char *password = "123456789";

const char *ssid = "DIGISOL";
const char *password = "qas1725utl1";

const char *mqtt_server = "3.111.108.14";
String current_motor_mode = "";
const char *sensorName = "vijay@gmail.com-block-43-upper-tank";
const int externalLedPin = 0; // GPIO0 (D3) - LED pin connected to NodeMCU
const int TRIGGER_PIN = 5;    // GPIO5 (D1) - Trigger pin of HC-SR04 connected to NodeMCU
const int ECHO_PIN = 4;       // GPIO4 (D2) - Echo pin of HC-SR04 connected to NodeMCU
const int MAX_DISTANCE = 200; // Maximum distance to measure in centimeters

const int motorPin1 = 14;   // GPIO14 (D5) - Input pin 1 (e.g., IN1) of L293D connected to NodeMCU
const int motorPin2 = 13;   // GPIO13 (D7) - Input pin 2 (e.g., IN2) of L293D connected to NodeMCU



const int DHTPIN = 2;  // GPIO2 (D4) - Data pin of DHT11 sensor
const int DHTTYPE = DHT11; // Type of the DHT sensor
boolean isMotorOn = false; 
WiFiClient espClient;

PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
DHT dht(DHTPIN, DHTTYPE);

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



void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), "root", "manager")) {
      Serial.println("connected");
      char topicSubscribe[150]; // Adjust the buffer size as needed
      strcpy(topicSubscribe, sensorName );
      strcat(topicSubscribe, "/change_status");
      
      client.subscribe(topicSubscribe);
      String motorControlTopic = String(sensorName) + "/motor/control";
      client.subscribe(motorControlTopic.c_str());
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

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  digitalWrite(motorPin1, LOW);  // Initially, set input pin 1 (IN1) to LOW
  digitalWrite(motorPin2, LOW);  // Initially, set input pin 2 (IN2) to LOW

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    int distance = sonar.ping_cm();
    // Read DHT11 sensor data
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

       // Create the JSON payload
    String jsonPayload = createJsonPayload(distance, 40, humidity, temperature,current_motor_mode);

    Serial.print("Publish message: ");
    Serial.println(jsonPayload);
    char topicPublish[150]; // Adjust the buffer size as needed
    strcpy(topicPublish, "sensor_data/");
    strcat(topicPublish, sensorName);
    
    client.publish(topicPublish, jsonPayload.c_str());
  }
}

String createJsonPayload(int distance, int maxValue, float humidity, float temperature, String current_motor_mode) {
  StaticJsonDocument<200> jsonDoc;
  
  jsonDoc["data"] = distance;
  jsonDoc["maxValue"] = maxValue;
  jsonDoc["currentStatus"] = (digitalRead(externalLedPin) == LOW) ? "off" : "on"; // Check the state of externalLedPin
  jsonDoc["humidity"] = humidity; 

  jsonDoc["temperature"] = temperature;
  jsonDoc["current_motor_mode"] = current_motor_mode;
  String payload;
  serializeJson(jsonDoc, payload);

  return payload;
}

void controlMotor(String command) {
  if (command == "on") {
    isMotorOn = true;
// Run in clockwise direction
  } else if (command == "full") {
    isMotorOn = true;
    analogWrite(motorPin1, 255);  // Set motor to full speed
    digitalWrite(motorPin2, LOW); 
  }
   else if (command == "off") {
    isMotorOn = false;
    digitalWrite(motorPin1, LOW); // Stop the motor
    digitalWrite(motorPin2, LOW);
  } else if (command == "half") {
    isMotorOn = true;
    analogWrite(motorPin1, 215);  // Set motor to half speed
    digitalWrite(motorPin2, LOW); // Run in clockwise direction
  } else if (command == "quarter") {
    isMotorOn = true;
    analogWrite(motorPin1, 198);   // Set motor to quarter speed
    digitalWrite(motorPin2, LOW); // Run in clockwise direction
  } else if (command == "clockwise") {
    if (isMotorOn) {
      digitalWrite(motorPin1, HIGH); // Set input pin 1 (IN1) to HIGH
      digitalWrite(motorPin2, LOW);  // Set input pin 2 (IN2) to LOW
    }
  } else if (command == "counterclockwise") {
    if (isMotorOn) {
      digitalWrite(motorPin1, LOW);  // Set input pin 1 (IN1) to LOW
      digitalWrite(motorPin2, HIGH); // Set input pin 2 (IN2) to HIGH
    }
  }
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

  String topicStr = String(topic);

 if (topicStr == String(sensorName) + "/motor/control") {
    String motorCommand;
    for (int i = 0; i < length; i++) {
      motorCommand += (char)payload[i];
    }
    current_motor_mode=motorCommand;
    controlMotor(motorCommand);
  }
}





