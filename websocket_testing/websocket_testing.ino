/*
	Esp8266 Websockets Client

	This sketch:
        1. Connects to a WiFi network
        2. Connects to a Websockets server
        3. Sends the websockets server a message ("Hello Server")
        4. Prints all incoming messages while the connection is open

	Hardware:
        For this sketch you only need an ESP8266 board.

	Created 15/02/2019
	By Gil Maimon
	https://github.com/gilmaimon/ArduinoWebsockets

*/

#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
const char* ssid = "DIGISOL"; //Enter SSID
const char* password = "qas1725utl1"; //Enter Password
const char* websockets_server_host = "3.111.108.14"; //Enter server adress
const uint16_t websockets_server_port = 8878; // Enter server port

using namespace websockets;

WebsocketsClient client;
void setup() {
    Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }

    Serial.println("Connected to Wifi, Connecting to server.");
    // try to connect to Websockets server
    bool connected = client.connect(websockets_server_host, websockets_server_port, "/socket.io/?EIO=4&transport=websocket");
    if(connected) {
        Serial.println("Connecetd!");
            // Create a JSON object

  const size_t capacity = JSON_OBJECT_SIZE(3) + 90;

  // Create a JSON document with the specified capacity
  DynamicJsonDocument jsonDocument(capacity);

  // Create individual JSON objects for each field and assign values
  JsonObject jsonObject = jsonDocument.to<JsonObject>();
  jsonObject["senderName"] = "user111";
  jsonObject["targetUserName"] = "user222";
  jsonObject["message"] = "hare krishna";
  String eventName = "messageSendToUser";
  // Serialize the JSON object to a string
  String serializedJson;
  serializeJson(jsonDocument, serializedJson);

  // Constructing the final message
  String messageToSend = "42[\"" + eventName + "\", " + serializedJson + "]";

  // Send the message
  client.send(messageToSend);
  Serial.println(messageToSend);
        
    } else {
        Serial.println("Not Connected!");
    }
    
    // run callback when messages are received
    client.onMessage([&](WebsocketsMessage message) {
        Serial.print("Got Message: ");
        Serial.println(message.data());
    });
}

void loop() {
    // let the websockets client check for incoming messages
    if(client.available()) {
        client.poll();
    }
    sendMessageToServer("messageSendToUser", "user112233", "user332211", "radhey radhey");
    delay(100);
}

void sendMessageToServer(const char* eventName, const char* inputMessage) {
    // Create a JSON object
  const size_t capacity = JSON_OBJECT_SIZE(3) + 90;

  // Create a JSON document with the specified capacity
  DynamicJsonDocument jsonDocument(capacity);

  // Create individual JSON objects for each field and assign values
  JsonObject jsonObject = jsonDocument.to<JsonObject>();
  jsonObject["senderName"] = senderName;
  jsonObject["targetUserName"] = targetUserName;
  jsonObject["message"] = inputMessage;
 
  // Serialize the JSON object to a string
  String serializedJson;
  serializeJson(jsonDocument, serializedJson);

  // Constructing the final message
  String messageToSend = "42[\"" + eventName + "\", " + inputMessage + "]";

  // Send the message
  client.send(messageToSend);
  // Print the serialized JSON string
  Serial.println(messageToSend); // Output the JSON message to Serial Monitor
}