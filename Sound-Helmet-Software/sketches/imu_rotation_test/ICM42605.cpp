/******************************************************************************
 * ICM42605.cpp - Library Source File for the ICM-42605 IMU over SPI
 ******************************************************************************/

 #include "ICM42605.h"

 /**
  * @brief Constructor for the ICM42605_SPI class.
  * @param cs_pin The Chip Select pin.
  * @param sclk_pin The Serial Clock pin.
  * @param mosi_pin The MOSI (SDI) pin.
  * @param miso_pin The MISO (SDO) pin.
  */
 ICM42605_SPI::ICM42605_SPI(int cs_pin, int sclk_pin, int mosi_pin, int miso_pin)
   : _cs_pin(cs_pin),
     _sclk_pin(sclk_pin),
     _mosi_pin(mosi_pin),
     _miso_pin(miso_pin),
     _imuSpiSettings(8000000, MSBFIRST, SPI_MODE0) // 8MHz, MSB first, SPI Mode 0
 {
   // Default sensitivity for +/- 2000 dps
   _gyro_sensitivity = 16.4;
 }
 
 /**
  * @brief Initializes the SPI communication and configures the ICM-42605 sensor.
  * @return True if initialization was successful, false otherwise.
  */
 bool ICM42605_SPI::begin() {
   // Configure the Chip Select pin
   pinMode(_cs_pin, OUTPUT);
   digitalWrite(_cs_pin, HIGH); // Deselect device initially
 
   // Initialize SPI with the specified pins
   SPI.begin(_sclk_pin, _miso_pin, _mosi_pin, _cs_pin);
 
   // Check the WHO_AM_I register to verify communication
   uint8_t whoAmI = readRegister(REG_WHO_AM_I);
   if (whoAmI != WHO_AM_I_VALUE) {
     Serial.print("Failed to find ICM-42605. WHO_AM_I: 0x");
     Serial.println(whoAmI, HEX);
     return false;
   }
 
   // Wake up sensor and enable Gyro/Accel in Low Noise (LN) mode
   writeRegister(REG_PWR_MGMT0, 0x0F);
   delay(1);
 
   // Configure Gyro: +/- 2000 dps, 1 kHz ODR
   writeRegister(REG_GYRO_CONFIG0, 0x06);
   delay(1);
   
   // Configure Accel: +/- 16g, 1 kHz ODR
   writeRegister(REG_ACCEL_CONFIG0, 0x06);
   delay(1);
 
   // Initialize the timer
   _lastTime = micros();

   // Set initial total rotation
   _totalRotation = 0.0;
   
   return true;
 }
 
 /**
  * @brief Polls the IMU and calculates the net rotation in the XY plane.
  * @return The net degrees of rotation in the XY plane since the last call.
  */
 float ICM42605_SPI::getNetXYPlaneRotation() {
   // Calculate the time elapsed since the last measurement
   unsigned long currentTime = micros();
   float deltaTime = (currentTime - _lastTime) / 1000000.0;
   _lastTime = currentTime;
 
   // Read the Z-axis gyroscope data
   uint8_t gyroData[2];
   readBurst(REG_GYRO_DATA_Z1, gyroData, 2);
 
   // Combine the two bytes into a 16-bit signed integer
   int16_t rawGyroZ = (int16_t)((gyroData[0] << 8) | gyroData[1]);
 
   // Convert raw data to degrees per second
   float gyroZ_dps = (float)rawGyroZ / _gyro_sensitivity;
 
   // Integrate to get rotation in degrees
   float rotation_degrees = gyroZ_dps * deltaTime;
 
   return rotation_degrees;
 }

 /**
 * @brief Calculates and returns the total net rotation in the XY plane.
 * @return The cumulative rotation in degrees around the Z-axis.
 */
float ICM42605_SPI::getTotalXYPlaneRotation() {
  // Calculate the time elapsed
  unsigned long currentTime = micros();
  float deltaTime = (currentTime - _lastTime) / 1000000.0;
  _lastTime = currentTime;

  // Read Z-axis gyro data
  uint8_t gyroData[2];
  readBurst(REG_GYRO_DATA_Z1, gyroData, 2);
  int16_t rawGyroZ = (int16_t)((gyroData[0] << 8) | gyroData[1]);

  // Convert to degrees per second
  float gyroZ_dps = (float)rawGyroZ / _gyro_sensitivity;

  // Integrate and accumulate rotation
  _totalRotation += gyroZ_dps * deltaTime;

  return _totalRotation;
}
 
 // ============================================================================
 //  PRIVATE HELPER FUNCTIONS
 // ============================================================================
 
 void ICM42605_SPI::writeRegister(uint8_t reg, uint8_t data) {
   SPI.beginTransaction(_imuSpiSettings);
   digitalWrite(_cs_pin, LOW);
   SPI.transfer(reg & 0x7F);
   SPI.transfer(data);
   digitalWrite(_cs_pin, HIGH);
   SPI.endTransaction();
 }
 
 void ICM42605_SPI::readBurst(uint8_t reg, uint8_t* buffer, int len) {
   SPI.beginTransaction(_imuSpiSettings);
   digitalWrite(_cs_pin, LOW);
   SPI.transfer(reg | 0x80);
   for (int i = 0; i < len; i++) {
     buffer[i] = SPI.transfer(0x00);
   }
   digitalWrite(_cs_pin, HIGH);
   SPI.endTransaction();
 }
 
 uint8_t ICM42605_SPI::readRegister(uint8_t reg) {
   uint8_t data;
   readBurst(reg, &data, 1);
   return data;
 }
 