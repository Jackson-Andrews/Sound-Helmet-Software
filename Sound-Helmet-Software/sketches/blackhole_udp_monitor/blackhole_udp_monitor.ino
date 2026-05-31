#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WiFiUDP udp;
const int localPort = 12345;
uint8_t audioBuffer[1024];
const int ledPin = 13;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  udp.begin(localPort);
  Serial.print("Listening on port ");
  Serial.println(localPort);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    int len = udp.read(audioBuffer, sizeof(audioBuffer));
    Serial.print("Received ");
    Serial.print(len);
    Serial.println(" bytes of audio");

    bool allZero = true;
    for (int i = 0; i < len; i++) {
      if (audioBuffer[i] != 0) {
        allZero = false;
        break;
      }
    }

    digitalWrite(ledPin, allZero ? LOW : HIGH);

    for (int i = 0; i < min(len, 32); i++) {
      if (i % 8 == 0) {
        Serial.print("\n");
      }
      Serial.printf("%02X ", audioBuffer[i]);
    }
    Serial.println("\n--------------------------");
  }
}
