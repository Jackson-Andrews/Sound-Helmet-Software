#include <WiFi.h>
#include <cstring>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

#pragma pack(push, 1)
struct __attribute__((packed)) Data {
  int16_t seq;
  int32_t distance;
  float voltage;
  char text[50];
} data;
#pragma pack(pop)

WiFiServer server(10000);
WiFiClient client;

void printWifiStatus() {
  Serial.print("\nSSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
  Serial.begin(115200);

  Serial.printf("Connecting to %s...\n", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi.");

  server.begin();
  printWifiStatus();
  Serial.println("Server listening on port 10000");
}

void loop() {
  if (!client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("Client connected.");
    } else {
      return;
    }
  }

  if (client.available() >= sizeof(data)) {
    uint8_t buffer[sizeof(data)];
    int bytesRead = client.readBytes(buffer, sizeof(data));

    if (bytesRead == sizeof(data)) {
      memcpy(&data, buffer, sizeof(data));
      data.text[sizeof(data.text) - 1] = '\0';

      Serial.printf("Received %d bytes. ", bytesRead);
      Serial.printf("seq: %d, distance: %ld, voltage: %f, text: '%s'\n",
                    data.seq, data.distance, data.voltage, data.text);
    } else {
      Serial.printf("Read error. Expected %d bytes, got %d.\n", sizeof(data), bytesRead);
    }
  }
}
