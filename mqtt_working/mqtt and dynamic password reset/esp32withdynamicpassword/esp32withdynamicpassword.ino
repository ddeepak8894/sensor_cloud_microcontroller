#include <ESP8266WiFi.h>

const char* defaultSSID = "DefaultSSID";
const char* defaultPassword = "DefaultPassword";
const char* apSSID = "sensor_cloud_wifi";
const char* apPassword = "123456789";

WiFiServer server(80);
bool connectedToWiFi = false;

// Define the HTML form
const char* htmlForm = R"(
<!DOCTYPE HTML>
<html>
<head>
  <meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0">
  <title>WiFi Creds Form</title>
  <style>
    body {
      background-color: #808080;
      font-family: Arial, Helvetica, Sans-Serif;
      text-align: center;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }

    h3 {
      color: #000000;
    }

    form {
      background-color: #fff;
      border-radius: 10px; /* Rounded borders */
      border: 2px solid #000; /* Solid border */
      padding: 20px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.2);
      width: 300px; /* Set the form width */
      margin: 10 auto; /* Center the form horizontally */
      margin-right: 5px;
    }

    label {
      font-weight: bold;
    }

    input {
      width: 90%;
      padding: 8px;
      margin: 5px 0;
    }

    input[type="submit"] {
      background-color: #008CBA;
      color: white;
      border: none;
      padding: 10px 20px;
      cursor: pointer;
    }

    input[type="submit"]:hover {
      background-color: #005F7F;
    }
  </style>
</head>

<body>
  
  <form action="/" method="post">
    <h3>Enter your WiFi credentials</h3>
    <p>
      <label>SSID: </label>
      <input maxlength="30" name="ssid"><br>
      <label>Key: </label>
      <input maxlength="30" name="password"><br><br>
      <input type="submit" value="Save">
    </p>
  </form>
</body>
</html>

)";

void setup() {
    Serial.begin(115200);

    // Connect to a Wi-Fi network using default credentials
    connectedToWiFi = connectToWiFi(defaultSSID, defaultPassword);

    // Start the server
    server.begin();

    // Print your IP address
    Serial.print("Server IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Check for client connections
    WiFiClient client = server.available();

    if (client) {
        Serial.println("New client connected");
        String request = client.readStringUntil('\r');
        client.flush();

        if (request.indexOf("/reset") != -1) {
            // Handle the form submission
            handleResetRequest(client);
        } else {
            // Serve the HTML form
            serveResetForm(client);
        }

        // Close the connection
        client.stop();
        Serial.println("Client disconnected");
    }

    // Check if we are connected to Wi-Fi
    if (connectedToWiFi) {
        // You can add additional code to run when connected to Wi-Fi here
    } else {
        // We are not connected to Wi-Fi, so enable the Soft AP
        setupSoftAP(apSSID, apPassword);
    }
}

bool connectToWiFi(const char* ssid, const char* pass) {
    Serial.println("Connecting to WiFi...");

    if (WiFi.begin(ssid, pass) == WL_CONNECTED) {
        Serial.println("Connected to WiFi");
        return true;
    } else {
        Serial.println("Connection failed.");
        return false;
    }
}

void setupSoftAP(const char* apSSID, const char* apPass) {
    Serial.println("Setting up Soft AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPass);
    Serial.println("Soft AP mode started.");
    Serial.print("AP SSID: ");
    Serial.println(apSSID);
}

void serveResetForm(WiFiClient client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println(htmlForm);
}

void handleResetRequest(WiFiClient client) {
    String newSSID = "";
    String newPassword = "";
    if (client.available()) {
        String request = client.readStringUntil('\r');
        client.flush();
        int ssidPos = request.indexOf("newSSID=");
        int passPos = request.indexOf("newPassword=");
        if (ssidPos != -1 && passPos != -1) {
            newSSID = request.substring(ssidPos + 8, passPos - 1);
            newPassword = request.substring(passPos + 12);
        }

        // Handle the new credentials (e.g., store them in EEPROM)

        // Send a response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println();
        client.println(htmlForm);
        
        // Once the credentials are set, turn off the Soft AP
        turnOffSoftAP();
    }
}

void turnOffSoftAP() {
    Serial.println("Turning off Soft AP mode...");
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA); // Switch to Station mode
    Serial.println("Soft AP mode turned off.");
}
