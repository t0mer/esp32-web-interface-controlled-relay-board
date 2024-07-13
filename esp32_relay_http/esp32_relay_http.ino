#include <WiFi.h>

// Replace with your network credentials
const char* ssid     = "**************";  // Network SSID (name)
const char* password = "**************";  // Network password

// Set web server port number to 80 (HTTP default)
WiFiServer server(80);

// Variable to store the HTTP request from the client
String header;

// Variables to store the current state of each device (ON/OFF)
String Device1State = "off";
String Device2State = "off";
String Device3State = "off";
String Device4State = "off";
String Device5State = "off";
String Device6State = "off";

// Assign each device to a GPIO pin
const int Device1 = 4;
const int Device2 = 5;
const int Device3 = 18;
const int Device4 = 19;
const int Device5 = 21;
const int Device6 = 22;

// Variables to track the current time and last time a client made a request
unsigned long currentTime = millis();
unsigned long previousTime = 0;
// Define timeout time in milliseconds (2000ms = 2 seconds)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the GPIO pins for the devices as outputs and set them to LOW
  pinMode(Device1, OUTPUT);
  pinMode(Device2, OUTPUT);
  pinMode(Device3, OUTPUT);
  pinMode(Device4, OUTPUT);
  pinMode(Device5, OUTPUT);
  pinMode(Device6, OUTPUT);
  digitalWrite(Device1, LOW);
  digitalWrite(Device2, LOW);
  digitalWrite(Device3, LOW);
  digitalWrite(Device4, LOW);
  digitalWrite(Device5, LOW);
  digitalWrite(Device6, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print the local IP address once connected
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the Relays on and off
            if (header.indexOf("GET /1/on") >= 0) {
              Serial.println("Device 1 on");
              Device1State = "on";
              digitalWrite(Device1, HIGH);
            } else if (header.indexOf("GET /1/off") >= 0) {
              Serial.println("Device 1 off");
              Device1State = "off";
              digitalWrite(Device1, LOW);
            } else if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("Device 2 on");
              Device2State = "on";
              digitalWrite(Device2, HIGH);
            } else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("Device 2 off");
              Device2State = "off";
              digitalWrite(Device2, LOW);
            } else if (header.indexOf("GET /3/on") >= 0) {
              Serial.println("Device 3 on");
              Device3State = "on";
              digitalWrite(Device3, HIGH);
            } else if (header.indexOf("GET /3/off") >= 0) {
              Serial.println("Device 3 off");
              Device3State = "off";
              digitalWrite(Device3, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("Device 4 on");
              Device4State = "on";
              digitalWrite(Device4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("Device 4 off");
              Device4State = "off";
              digitalWrite(Device4, LOW);
            } else if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("Device 5 on");
              Device5State = "on";
              digitalWrite(Device5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("Device 5 off");
              Device5State = "off";
              digitalWrite(Device5, LOW);
            } else if (header.indexOf("GET /6/on") >= 0) {
              Serial.println("Device 6 on");
              Device6State = "on";
              digitalWrite(Device6, HIGH);
            } else if (header.indexOf("GET /6/off") >= 0) {
              Serial.println("Device 6 off");
              Device6State = "off";
              digitalWrite(Device6, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // Enhanced CSS for a compact look
            client.println("<style>");
            client.println("html { font-family: 'Roboto', sans-serif; background: #f5f5f5; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }");
            client.println("body { max-width: 600px; background: #fff; padding: 20px; margin: 20px; border-radius: 5px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }");
            client.println("h1 { color: #333; text-align: center; }");
            client.println(".device { background: linear-gradient(to right, #dae2f8, #d6a4a4); padding: 15px; margin: 10px 0; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); text-align: center; }"); // Reduced padding and margin
            client.println("p { font-size: 18px; color: #555; margin: 10px 0; }");
            client.println(".button { border: none; padding: 15px 35px; text-align: center;"); // Slightly reduced padding
            client.println("text-decoration: none; display: inline-block; font-size: 22px; margin: 10px; cursor: pointer; border-radius: 5px; transition: all 0.3s ease; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2); }");
            client.println(".button:hover { box-shadow: 0 4px 8px rgba(0, 0, 0, 0.4); }");
            client.println(".button-on { background-color: #4CAF50; color: white; }");
            client.println(".button-off { background-color: #f44336; color: white; }");
            client.println("</style></head>");

            // Web Page Heading
            client.println("<body><h1>Smart Home Automation</h1>");

            // Display current state, and ON/OFF buttons for Relay 1
            client.println("<div class=\"device\"><p>Device 1 - State " + Device1State + "</p>");
            if (Device1State == "off") {
              client.println("<a href=\"/1/on\"><button class=\"button button-on\">ON</button></a>");
            } else {
              client.println("<a href=\"/1/off\"><button class=\"button button-off\">OFF</button></a>");
            }
            client.println("</div>");

            // Display current state, and ON/OFF buttons for Relay 2
            client.println("<div class=\"device\"><p>Device 2 - State " + Device2State + "</p>");
            if (Device2State == "off") {
              client.println("<a href=\"/2/on\"><button class=\"button button-on\">ON</button></a>");
            } else {
              client.println("<a href=\"/2/off\"><button class=\"button button-off\">OFF</button></a>");
            }
            client.println("</div>");

            // Display current state, and ON/OFF buttons for Relay 3
            client.println("<div class=\"device\"><p>Device 3 - State " + Device3State + "</p>");
            if (Device3State == "off") {
              client.println("<a href=\"/3/on\"><button class=\"button button-on\">ON</button></a>");
            } else {
              client.println("<a href=\"/3/off\"><button class=\"button button-off\">OFF</button></a>");
            }
            client.println("</div>");

            // Display current state, and ON/OFF buttons for Relay 4
            client.println("<div class=\"device\"><p>Device 4 - State " + Device4State + "</p>");
            if (Device4State == "off") {
              client.println("<a href=\"/4/on\"><button class=\"button button-on\">ON</button></a>");
            } else {
              client.println("<a href=\"/4/off\"><button class=\"button button-off\">OFF</button></a>");
            }
            client.println("</div>");
            
            // Display current state, and ON/OFF buttons for Relay 5
            client.println("<div class=\"device\"><p>Device 5 - State " + Device5State + "</p>");
            if (Device5State == "off") {
              client.println("<a href=\"/5/on\"><button class=\"button button-on\">ON</button></a>");
            } else {
              client.println("<a href=\"/5/off\"><button class=\"button button-off\">OFF</button></a>");
            }
            client.println("</div>");

            // Display current state, and ON/OFF buttons for Relay 6
            client.println("<div class=\"device\"><p>Device 6 - State " + Device6State + "</p>");
            if (Device6State == "off") {
              client.println("<a href=\"/6/on\"><button class=\"button button-on\">ON</button></a>");
            } else {
              client.println("<a href=\"/6/off\"><button class=\"button button-off\">OFF</button></a>");
            }
            client.println("</div>");

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
