#include <ICM42605.h>

#define CS_PIN    5
#define SCLK_PIN  9
#define MOSI_PIN  6
#define MISO_PIN  7
#define INT_PIN   8

ICM42605_SPI imu(CS_PIN, SCLK_PIN, MOSI_PIN, MISO_PIN);

void setup() {
  Serial.begin(115200);
  delay(100);

  if (!imu.begin()) {
    Serial.println("IMU initialization failed.");
    while (1);
  }

  Serial.println("ICM-42605 initialized successfully.");
}

void loop() {
  float totalRotation = imu.getTotalXYPlaneRotation();
  Serial.print("Total Z-axis rotation (deg): ");
  Serial.println(totalRotation, 3);

  delay(1000);
}
