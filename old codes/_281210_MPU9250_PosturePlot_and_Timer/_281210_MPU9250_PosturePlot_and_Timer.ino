//refered http://www.lucidarme.me/?p=5057

//using outer file "MPU9250_Func.ino"

#include <Wire.h>
#include<TimerOne.h>


#define MPU9250_ADDRESS 0x68
#define MAG_ADDRESS     0x0C

#define GYRO_FULL_SCALE_250_DPS   0x00
#define GYRO_FULL_SCALE_500_DPS   0x08
#define GYRO_FULL_SCALE_1000_DPS  0x10
#define GYRO_FULL_SCALE_2000_DPS  0x18

#define ACC_FULL_SCALE_2_G    0x00
#define ACC_FULL_SCALE_4_G    0x08
#define ACC_FULL_SCALE_8_G    0x10
#define ACC_FULL_SCALE_16_G   0x18

#define ACC_SCALE 4
#define GYR_SCALE 500
#define SamplingRate 50 //Hz


//this function read Nbytes bytes from I2C device at address Address.
//put read bytes starting at register Register in the data Array.
void I2Cread(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t* Data) {
  //set register address
  //noInterrupts();
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();


  //read Nbytes
  Wire.requestFrom(Address, Nbytes);
  uint8_t index = 0;
  while (Wire.available())
    Data[index++] = Wire.read();
  //interrupts();
}


//Write a byte (Data) in device (Address) at register (Register)
void I2CwriteByte(uint8_t Address, uint8_t Register, uint8_t Data) {
  //Set register address
  //noInterrupts();
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  Wire.endTransmission();
  //interrupts();
}

//Initiallizations
void setup() {
  //Arduino initializations
  Wire.begin();
  Serial.begin(115200);

  pinMode(13, OUTPUT);

  //Configure sensor range
  I2CwriteByte(MPU9250_ADDRESS, 27, GYRO_FULL_SCALE_500_DPS);
  I2CwriteByte(MPU9250_ADDRESS, 28, ACC_FULL_SCALE_4_G);

  //Set by pass mode for the magnetometers
  I2CwriteByte(MPU9250_ADDRESS, 0x37, 0x02);

  //Request first magnetometer single measurement
  I2CwriteByte(MAG_ADDRESS, 0x0A, 0x01);

  Calibration();

  Timer1.initialize(20000);
  Timer1.attachInterrupt(PostureAngle);
}


float data[6], Calib[6] = {0, 0, 0, 0, 0, 0};
float ACCroll = 0, ACCpitch = 0, ACCyaw = 0;
float roll = 0, pitch = 0, yaw = 0;
float k = 0;

long int cpt = 0;

//Main loop, read and display data
void loop() {
  //Calibration();

  while (1) {
    //Serial.print(cpt++, DEC);
    //Serial.print("\t");

    //PostureAngle();



    //-------------------------------------//
    // ::: accelerometer and gyroscope ::: //
    //read accelerometer and gyroscope

    //-------------------------------------//
    // ::: display values ::: /
    //Serial.print(roll,DEC);
    //Serial.print(",");
    //Serial.print(data[1], DEC);
    //Serial.print(",");
    Serial.print(pitch, DEC);
    //Serial.print(",");
    //Serial.print(yaw, DEC);
    //Serial.print(",");
    //Serial.print(ACCroll, DEC);
    //Serial.print(",");
    //Serial.print(ACCpitch, DEC);
    //Serial.print(",");
    //Serial.print(k, DEC);


    //End of line
    Serial.println("");
    //delay(1000 / SamplingRate);
  }
}

void flash() {
  static boolean output = HIGH;
  digitalWrite(13, output);
  output = !output;
}

