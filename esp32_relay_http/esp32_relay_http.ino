#include <WiFi.h>

// Replace with your network credentials
const char* ssid     = "**************";  // Network SSID (name)
const char* password = "**************";  // Network password

// Set web server port number to 80 (HTTP default)
WiFiServer server(80);

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

// Variables to track the current time and last time the client sent data
unsigned long currentTime = millis();
unsigned long previousTime = 0;
// Idle timeout in milliseconds: drop a client that stops sending for this long
const long timeoutTime = 2000;
// Cap on how many bytes we buffer for a single request line, so a client that
// streams data without a line break can't exhaust the heap.
const size_t MAX_LINE_LENGTH = 256;

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

// Return true if a header line begins with the given (case-insensitive) name.
bool headerIs(const String& line, const char* name) {
  size_t n = strlen(name);
  return line.length() >= n && line.substring(0, n).equalsIgnoreCase(name);
}

// Extract the trimmed value that follows the first ':' in a header line.
String headerValue(const String& line) {
  int colon = line.indexOf(':');
  if (colon < 0) return "";
  String v = line.substring(colon + 1);
  v.trim();
  return v;
}

// Extract the host[:port] portion of a URL (e.g. from an Origin/Referer value).
String urlHost(const String& url) {
  int start = url.indexOf("://");
  start = (start >= 0) ? start + 3 : 0;
  int end = url.indexOf('/', start);
  if (end < 0) end = url.length();
  return url.substring(start, end);
}

// CSRF guard: a state-changing request must originate from the device's own
// page. We require an Origin (preferred) or Referer whose host matches the Host
// header. A request carrying neither is rejected.
bool sameOrigin(const String& host, const String& origin, const String& referer) {
  if (host.length() == 0) return false;
  if (origin.length() > 0)  return urlHost(origin).equalsIgnoreCase(host);
  if (referer.length() > 0) return urlHost(referer).equalsIgnoreCase(host);
  return false;
}

// Apply a relay action for an exact request path (e.g. "/1/on").
void applyAction(const String& path) {
  if (path == "/1/on")       { Device1State = "on";  digitalWrite(Device1, HIGH); Serial.println("Device 1 on"); }
  else if (path == "/1/off") { Device1State = "off"; digitalWrite(Device1, LOW);  Serial.println("Device 1 off"); }
  else if (path == "/2/on")  { Device2State = "on";  digitalWrite(Device2, HIGH); Serial.println("Device 2 on"); }
  else if (path == "/2/off") { Device2State = "off"; digitalWrite(Device2, LOW);  Serial.println("Device 2 off"); }
  else if (path == "/3/on")  { Device3State = "on";  digitalWrite(Device3, HIGH); Serial.println("Device 3 on"); }
  else if (path == "/3/off") { Device3State = "off"; digitalWrite(Device3, LOW);  Serial.println("Device 3 off"); }
  else if (path == "/4/on")  { Device4State = "on";  digitalWrite(Device4, HIGH); Serial.println("Device 4 on"); }
  else if (path == "/4/off") { Device4State = "off"; digitalWrite(Device4, LOW);  Serial.println("Device 4 off"); }
  else if (path == "/5/on")  { Device5State = "on";  digitalWrite(Device5, HIGH); Serial.println("Device 5 on"); }
  else if (path == "/5/off") { Device5State = "off"; digitalWrite(Device5, LOW);  Serial.println("Device 5 off"); }
  else if (path == "/6/on")  { Device6State = "on";  digitalWrite(Device6, HIGH); Serial.println("Device 6 on"); }
  else if (path == "/6/off") { Device6State = "off"; digitalWrite(Device6, LOW);  Serial.println("Device 6 off"); }
}

// Render one device card with a POST form that toggles its relay.
void deviceBlock(WiFiClient& client, int n, const String& state) {
  client.println("<div class=\"device\"><p>Device " + String(n) + " - State " + state + "</p>");
  if (state == "off") {
    client.println("<form action=\"/" + String(n) + "/on\" method=\"POST\"><button class=\"button button-on\">ON</button></form>");
  } else {
    client.println("<form action=\"/" + String(n) + "/off\" method=\"POST\"><button class=\"button button-off\">OFF</button></form>");
  }
  client.println("</div>");
}

// Send the full HTML control page reflecting the current relay states.
void sendPage(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  // Enhanced CSS for a compact look
  client.println("<style>");
  client.println("html { font-family: 'Roboto', sans-serif; background: #f5f5f5; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }");
  client.println("body { max-width: 600px; background: #fff; padding: 20px; margin: 20px; border-radius: 5px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }");
  client.println("h1 { color: #333; text-align: center; }");
  client.println(".device { background: linear-gradient(to right, #dae2f8, #d6a4a4); padding: 15px; margin: 10px 0; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); text-align: center; }");
  client.println(".device form { display: inline; margin: 0; }");
  client.println("p { font-size: 18px; color: #555; margin: 10px 0; }");
  client.println(".button { border: none; padding: 15px 35px; text-align: center;");
  client.println("text-decoration: none; display: inline-block; font-size: 22px; margin: 10px; cursor: pointer; border-radius: 5px; transition: all 0.3s ease; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2); }");
  client.println(".button:hover { box-shadow: 0 4px 8px rgba(0, 0, 0, 0.4); }");
  client.println(".button-on { background-color: #4CAF50; color: white; }");
  client.println(".button-off { background-color: #f44336; color: white; }");
  client.println("</style></head>");

  // Web Page Heading
  client.println("<body><h1>Smart Home Automation</h1>");

  deviceBlock(client, 1, Device1State);
  deviceBlock(client, 2, Device2State);
  deviceBlock(client, 3, Device3State);
  deviceBlock(client, 4, Device4State);
  deviceBlock(client, 5, Device5State);
  deviceBlock(client, 6, Device6State);

  client.println("</body></html>");
  client.println();
}

// Route a fully-received request: change state only for a same-origin POST,
// then send the control page.
void handleRequest(WiFiClient& client, const String& reqLine,
                   const String& host, const String& origin, const String& referer) {
  // Parse "METHOD PATH HTTP/x.x" from the request line only.
  int sp1 = reqLine.indexOf(' ');
  int sp2 = reqLine.indexOf(' ', sp1 + 1);
  String method = (sp1 > 0) ? reqLine.substring(0, sp1) : "";
  String path   = (sp1 > 0 && sp2 > sp1) ? reqLine.substring(sp1 + 1, sp2) : "/";

  // State-changing actions require POST + same-origin (CSRF protection).
  if (method == "POST") {
    if (sameOrigin(host, origin, referer)) {
      applyAction(path);
    } else {
      Serial.println("Rejected cross-origin POST");
    }
  }

  sendPage(client);
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (!client) return;

  Serial.println("New Client.");
  String currentLine = "";   // holds the current line being received
  String reqLine = "";       // the HTTP request line (first line)
  String host = "";          // Host header value
  String origin = "";        // Origin header value
  String referer = "";       // Referer header value
  bool firstLine = true;

  currentTime = millis();
  previousTime = currentTime;

  // Loop while the client is connected and hasn't gone idle past the timeout.
  while (client.connected() && currentTime - previousTime <= timeoutTime) {
    currentTime = millis();
    if (client.available()) {
      previousTime = currentTime;           // reset idle timeout on activity
      char c = client.read();
      Serial.write(c);
      if (c == '\n') {                       // end of a line
        if (currentLine.length() == 0) {     // blank line -> end of headers
          handleRequest(client, reqLine, host, origin, referer);
          break;
        }
        if (firstLine) {
          reqLine = currentLine;
          firstLine = false;
        } else if (headerIs(currentLine, "Host:")) {
          host = headerValue(currentLine);
        } else if (headerIs(currentLine, "Origin:")) {
          origin = headerValue(currentLine);
        } else if (headerIs(currentLine, "Referer:")) {
          referer = headerValue(currentLine);
        }
        currentLine = "";
      } else if (c != '\r') {
        // Buffer the line, but cap its length to protect the heap.
        if (currentLine.length() < MAX_LINE_LENGTH) currentLine += c;
      }
    }
  }

  // Close the connection
  client.stop();
  Serial.println("Client disconnected.");
  Serial.println("");
}
