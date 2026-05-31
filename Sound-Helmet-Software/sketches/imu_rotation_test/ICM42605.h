/******************************************************************************
 * ICM42605.h - Library Header File for the ICM-42605 IMU over SPI
 ******************************************************************************/

 #ifndef ICM42605_H
 #define ICM42605_H
 
 #include <Arduino.h>
 #include <SPI.h>
 
 class ICM42605_SPI {
 public:
   // Constructor
   ICM42605_SPI(int cs_pin, int sclk_pin, int mosi_pin, int miso_pin);
 
   // Public Methods
   bool begin();
   float getNetXYPlaneRotation();
   float getTotalXYPlaneRotation();
 
 private:
   // SPI Pin numbers
   int _cs_pin;
   int _sclk_pin;
   int _mosi_pin;
   int _miso_pin;
 
   // SPI Settings
   SPISettings _imuSpiSettings;
 
   // Last measurement time for integration
   unsigned long _lastTime;

   // Track total rotation
   float _totalRotation;
   
   // Gyroscope sensitivity
   float _gyro_sensitivity;
 
   // Helper functions for SPI communication
   void writeRegister(uint8_t reg, uint8_t data);
   void readBurst(uint8_t reg, uint8_t* buffer, int len);
   uint8_t readRegister(uint8_t reg);
 
   // Register Map (User Bank 0)
   static const uint8_t REG_DEVICE_CONFIG   = 0x11;
   static const uint8_t REG_PWR_MGMT0       = 0x4E;
   static const uint8_t REG_GYRO_CONFIG0    = 0x4F;
   static const uint8_t REG_ACCEL_CONFIG0   = 0x50;
   static const uint8_t REG_GYRO_DATA_Z1    = 0x29;
   static const uint8_t REG_WHO_AM_I        = 0x75;
   static const uint8_t WHO_AM_I_VALUE      = 0x42;
 };
 
 #endif // ICM42605_H
 