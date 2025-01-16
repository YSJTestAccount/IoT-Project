// Libraries
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

// I changed the original SSID & Password because of privacy reasons
const char* ssid = "VM5067560";
const char* password = "kwr8cbjsCxLq";

// Web Server Port Number
WiFiServer server(80);

String header;

String output26State = "off";

// Assigning GPIO pins
const int output26 = 26;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

// Analytics variables
unsigned long pageVisits = 0;
unsigned long serverUptime = 0;
String lastClientIP = "";

// Wi-Fi Information
int signalStrength = 0;

void setup() {
  Serial.begin(115200);
  pinMode(output26, OUTPUT);
  digitalWrite(output26, LOW);

  // Connecting to the WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    pageVisits++;
    lastClientIP = client.remoteIP().toString();
    signalStrength = WiFi.RSSI();

    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        header += c;
        Serial.write(c);
        if (c == '\n') {

          if (currentLine.length() == 0) {
            // Send the HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();


            if (header.indexOf("GET /26/on") >= 0) {
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              output26State = "off";
              digitalWrite(output26, LOW);
            }

            // HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            if (output26State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Analytics Data
            client.println("<h2>Analytics</h2>");
            client.println("<p>Total Page Visits: " + String(pageVisits) + "</p>");
            client.println("<p>Server Uptime: " + String(serverUptime / 1000) + " seconds</p>");
            client.println("<p>Last Client IP: " + lastClientIP + "</p>");
            client.println("<p>Wi-Fi Signal Strength (RSSI): " + String(signalStrength) + " dBm</p>");
            
            client.println("</body></html>");
            client.println();

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  serverUptime = millis();
}