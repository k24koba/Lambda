/*****************************************************************
  LSM9DS1_Basic_I2C.ino
  SFE_LSM9DS1 Library Simple Example Code - I2C Interface
  Jim Lindblom @ SparkFun Electronics
  Original Creation Date: April 30, 2015
  https://github.com/sparkfun/LSM9DS1_Breakout

  The LSM9DS1 is a versatile 9DOF sensor. It has a built-in
  accelerometer, gyroscope, and magnetometer. Very cool! Plus it
  functions over either SPI or I2C.

  This Arduino sketch is a demo of the simple side of the
  SFE_LSM9DS1 library. It'll demo the following:
  How to create a LSM9DS1 object, using a constructor (global
  variables section).
  How to use the begin() function of the LSM9DS1 class.
  How to read the gyroscope, accelerometer, and magnetometer
  using the readGryo(), readAccel(), readMag() functions and
  the gx, gy, gz, ax, ay, az, mx, my, and mz variables.
  How to calculate actual acceleration, rotation speed,
  magnetic field strength using the calcAccel(), calcGyro()
  and calcMag() functions.
  How to use the data from the LSM9DS1 to calculate
  orientation and heading.

  Hardware setup: This library supports communicating with the
  LSM9DS1 over either I2C or SPI. This example demonstrates how
  to use I2C. The pin-out is as follows:
  LSM9DS1 --------- Arduino
   SCL ---------- SCL (A5 on older 'Duinos')
   SDA ---------- SDA (A4 on older 'Duinos')
   VDD ------------- 3.3V
   GND ------------- GND
  (CSG, CSXM, SDOG, and SDOXM should all be pulled high.
  Jumpers on the breakout board will do this for you.)

  The LSM9DS1 has a maximum voltage of 3.6V. Make sure you power it
  off the 3.3V rail! I2C pins are open-drain, so you'll be
  (mostly) safe connecting the LSM9DS1's SCL and SDA pins
  directly to the Arduino.

  Development environment specifics:
  IDE: Arduino 1.6.3
  Hardware Platform: SparkFun Redboard
  LSM9DS1 Breakout Version: 1.0

  This code is beerware. If you see me (or any other SparkFun
  employee) at the local, and you've found our code helpful,
  please buy us a round!

  Distributed as-is; no warranty is given.
*****************************************************************/
// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>
#include <MadgwickAHRS.h>

//////////////////////////
// LSM9DS1 Library Init //
//////////////////////////
// Use the LSM9DS1 class to create an object. [imu] can be
// named anything, we'll refer to that throught the sketch.
LSM9DS1 imu;
Madgwick filter;

///////////////////////
// Example I2C Setup //
///////////////////////
// SDO_XM and SDO_G are both pulled high, so our addresses are:
#define LSM9DS1_M 0x1C // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6A // Would be 0x6A if SDO_AG is LOW

////////////////////////////
// Sketch Output Settings //
////////////////////////////
#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 25 // 25 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract
// a declination to get a more accurate heading. Calculate
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION 7.35 // Declination (degrees) in Tokyo, JAPAN.


// if this flag == 1, then execute initial process to calibrate gyro offset
// define the number of sample to get data to calibrate
int init_flag = 1;
float accelX, accelY, accelZ, gyroX, gyroY, gyroZ, magX, magY, magZ, roll, pitch, heading;
float offset_gx = 0;
float offset_gy = 0;
float offset_gz = 0;
#define NUM_OF_SAMPLES_FOR_INIT 500

void setup()
{

  Serial.begin(115200);

  // Before initializing the IMU, there are a few settings
  // we may need to adjust. Use the settings struct to set
  // the device's communication mode and addresses:
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;


  setupGyro();
  setupAccel();
  setupMag();

  // The above lines will only take effect AFTER calling
  // imu.begin(), which verifies communication with the IMU
  // and turns it on.
  if (!imu.begin())
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                   "work for an out of the box LSM9DS1 " \
                   "Breakout, but may need to be modified " \
                   "if the board jumpers are.");
    while (1)
      ;
  }
  filter.begin(40);
}

void loop()
{
  // initial process to subtract gyro offset from measured data
  if (init_flag == 1) {
    init_gyro_process();
    Serial.println("finished initialization !");
    Serial.println();
    init_flag = 0;
  }


  // Update the sensor values whenever new data is available
  if ( imu.gyroAvailable() )
  {
    // To read from the gyroscope,  first call the
    // readGyro() function. When it exits, it'll update the
    // gx, gy, and gz variables with the most current data.
    imu.readGyro();
  }

  if ( imu.accelAvailable() )
  {
    // To read from the accelerometer, first call the
    // readAccel() function. When it exits, it'll update the
    // ax, ay, and az variables with the most current data.
    imu.readAccel();
  }

  if ( imu.magAvailable() )
  {
    // To read from the magnetometer, first call the
    // readMag() function. When it exits, it'll update the
    // mx, my, and mz variables with the most current data.
    imu.readMag();
  }

  if ((lastPrint + PRINT_SPEED) < millis()) {
    gyroX = imu.calcGyro(imu.gx - offset_gx);
    gyroY = imu.calcGyro(imu.gy - offset_gy);
    gyroZ = imu.calcGyro(imu.gz - offset_gz);
    accelX = imu.calcAccel(imu.ax);
    accelY = imu.calcAccel(imu.ay);
    accelZ = imu.calcAccel(imu.az);
    magX = imu.calcMag(imu.mx);
    magY = imu.calcMag(imu.my);
    magZ = imu.calcMag(imu.mz);
    //    magX = imu.mx;
    //    magY = imu.my;
    //    magZ = imu.mz;

    filter.update(gyroX, gyroY, gyroZ, accelX, accelY, accelZ, magX, magY, magZ);

    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();

    printPosture(roll, pitch, heading);

    lastPrint = millis(); // Update lastPrint time
    //    Serial.println(lastPrint);
  }
}
