#include "accel.h"
#include "I2C.h"
#include "Quaternion.h"
#include <cstdio>
#include <cmath>

namespace accel {
  //TODO: enable calibration, maybe something better suited then a single-point calibration scheme
  //TODO: add gyro offset angle vs thrust angle (manuel or automatic)


  double roll, pitch;
  short accelX, accelY, accelZ;
  Quaternion rotation;


  double range(double val){
    return fmod((val + M_PI), (2 * M_PI)) - M_PI;
  }
  
  
  void init(){
    //change settings
      //Set all three axes offsets to 0    because calibration is disabled
    I2C::writeRegister(ACCEL, OFSX, 0b00000000);
    I2C::writeRegister(ACCEL, OFSY, 0b00000000);
    I2C::writeRegister(ACCEL, OFSZ, 0b00000000);
      //Activity/inactivity
    I2C::writeRegister(ACCEL, THRESH_ACT,   4);           // 250mg    62.5 mg/LSB
    I2C::writeRegister(ACCEL, THRESH_INACT, 4);           // 250mg    62.5 mg/LSB
    I2C::writeRegister(ACCEL, TIME_INACT,   10);          // 10sec    1 sec/LSB
    I2C::writeRegister(ACCEL, ACT_INACT_CTL,0b11111111);  // enable all axis, and set to not fixed values, but reletive values
      //Freefall detection
    I2C::writeRegister(ACCEL, THRESH_FF,    6);           //312.5mg   62.5 mg/LSB
    I2C::writeRegister(ACCEL, TIME_FF,      6);           //312.5mg   62.5 mg/LSB
      //Power control
    I2C::writeRegister(ACCEL, POWER_CTL,    0b00101000);  //enabke activity and inactivity link, disable autosleep on activity, enable measuring mode, disable sleep
      //Interupts
    I2C::writeRegister(ACCEL, INT_ENABLE,   0b00000000);  //disable interrupts for now
    I2C::writeRegister(ACCEL, INT_MAP,      0b00000000);
      //Data settings
    I2C::writeRegister(ACCEL, BW_RATE,      0b00001101);  //disable low power mode and set datarate to 800 Hz
    I2C::writeRegister(ACCEL, DATA_FORMAT,  0b00001011);  //disable self test, set full res high 16g data range         //TODO: enable and setup self test
    I2C::writeRegister(ACCEL, FIFO_CTL,     0b00000000);  //disable FIFO
  }

  bool isReady(){
    return ((I2C::getRegister(ACCEL, INT_SOURCE) & 0b10000000) == 0b10000000);  //check if data is ready
  }

  void readAccel(short& accelX, short& accelY, short& accelZ){
    uint8_t accelData[6];
    I2C::getRegister(ACCEL, DATAX0, &accelData[0], 6);//put data in accelData with pointer address, incement is automatic
    accelX = accelData[1] << 8 | accelData[0]; //DATAX1 .. DATAX0
    accelY = accelData[3] << 8 | accelData[2]; //DATAY1 .. DATAY0
    accelZ = accelData[5] << 8 | accelData[4]; //DATAZ1 .. DATAZ0
  }

  void readAccel(){
    readAccel(accelX, accelY, accelZ);
  }

  void calibrate(short& accelX, short& accelY, short& accelZ){
    long accelXTotal = 0;
    long accelYTotal = 0;
    long accelZTotal = 0;
    short accelXdata, accelYdata, accelZdata;
    int samplesize = 1000;                    //datasheet recommends more then 0.1 seconds of data

    //Set all three axes offsets to 0
    I2C::writeRegister(ACCEL, OFSX, 0);
    I2C::writeRegister(ACCEL, OFSY, 0);
    I2C::writeRegister(ACCEL, OFSZ, 0);

    for(int i = 0; i < samplesize; i++){
      while(!isReady()){}
      readAccel(accelXdata, accelYdata, accelZdata);
      accelXTotal += accelXdata; accelYTotal += accelYdata; accelZTotal += accelZdata;
    }

    accelX = accelXTotal / samplesize; accelY = accelYTotal / samplesize; accelZ = accelZTotal / samplesize;

    //Set all three axes offsets
    I2C::writeRegister(ACCEL, OFSX, -(uint8_t)(accelX/4));
    I2C::writeRegister(ACCEL, OFSY, -(uint8_t)(accelY/4));
    I2C::writeRegister(ACCEL, OFSZ, -(uint8_t)(accelZ/4));
  }

  Quaternion calcRot(){
    double roll, pitch;
    calcRot(roll, pitch);
    
    return Quaternion(roll, 0, pitch);
  }

  void calcRot(double& roll, double& pitch){
    roll = range(atan2(-accelY, accelZ)+M_PI);                      //van internet
    pitch = -atan2(accelX, sqrt(accelY*accelY + accelZ*accelZ));    //van internet
  }

  void accelLoop(){
    if(isReady()){
      readAccel(accelX, accelY, accelZ);

      //TODO: Trim
      calcRot(roll, pitch);
      
      rotation = calcRot();
    }
  }
}

