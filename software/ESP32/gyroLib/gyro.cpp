#include "gyro.h"

#include <cstdint>
#include <math.h>
#include <vector>

#include "FreeRTOS.h"
#include "freertos/task.h"

#include "Quaternion.h"
#include "I2C.h"
#include "accel.h"

namespace gyro { 
  short gyroX, gyroY, gyroZ;  //2 bytes (just like the register)
  double trimX, trimY, trimZ;
  
  double roll, yaw, pitch;
  
  Quaternion rotation = Quaternion(1, 0, 0, 0);

  long lastMicros;
  int timePast = 0;

  #ifdef DEBUG_GYROREADBUFFER
    std::vector<Quaternion> rotationBuffer;
  #endif

   
  void readGyro(short& gyroX, short& gyroY, short& gyroZ){ // ~280 microseconds
    uint8_t gyroData[6];
    I2C::getRegister(GYRO, OUT_X_L | (0b10000000), &gyroData[0], 6);//put data in gyroData with pointer address  -  incement the address after read register (with 0b10000000)
    gyroX = gyroData[1] << 8 | gyroData[0]; //OUT_X_H .. OUT_X_L
    gyroY = gyroData[3] << 8 | gyroData[2]; //OUT_Y_H .. OUT_Y_L
    gyroZ = gyroData[5] << 8 | gyroData[4]; //OUT_Z_H .. OUT_Z_L
  }
  
  void calcRot(double& rotX, double& rotY, double& rotZ, int trimX, int trimY, int trimZ, long timePast){ // ~15 microseconds
    rotX = -(gyroX - trimX) * 0.0175 * (timePast / 1000000.0) * (M_PI/180.0);    // (gyro trimmed) * sensitivity op 500 dps * timePast(s) * degToRad
    rotY = -(gyroY - trimY) * 0.0175 * (timePast / 1000000.0) * (M_PI/180.0);    // formula gets calculated by compiler (I think)
    rotZ = (gyroZ - trimZ) * 0.0175 * (timePast / 1000000.0) * (M_PI/180.0);
  }
  
  bool isReady(){
    return ((I2C::getRegister(GYRO, STATUS_REG) & 0b00001000) == 0b00001000); // check if data is ready
                                                                              // TODO: use STATUS_REG 7 to check if data has been overwritten and if so freq must be too high so set odr lower
  }
  
  void setTrim(double& TrimXI, double& TrimYI, double& TrimZI, int samplesize){
    long totX = 0, totY = 0, totZ = 0;
    short gyroX, gyroY, gyroZ;
    for(int i = 0; i < samplesize;){
        if(isReady()){
          i++;
          readGyro(gyroX, gyroY, gyroZ);
          totX += gyroX;
          totY += gyroY;
          totZ += gyroZ;
        }
      }
      TrimXI = (totX / samplesize);
      TrimYI = (totY / samplesize);
      TrimZI = (totZ / samplesize);
      
      TrimXI = 0;
      TrimYI = 0;
      TrimZI = 0;
  }
  
  
  void init(){                                          //TODO: make settings dependant on defines
    //change settings
    I2C::writeRegister(GYRO, CTRL_REG2, 0b00100001); //set high pass filter freq to 8Hz
    I2C::writeRegister(GYRO, CTRL_REG3, 0b00000000); //interrups default
    I2C::writeRegister(GYRO, CTRL_REG4, 0b00010000); //500 dps
    I2C::writeRegister(GYRO, REFERENCE, 0b00000000);
    
    I2C::writeRegister(GYRO, INT1_THS_XH, 0b00000000); //sets thresholds to 100
    I2C::writeRegister(GYRO, INT1_THS_XL, 0b01100100);
    I2C::writeRegister(GYRO, INT1_THS_YH, 0b00000000);
    I2C::writeRegister(GYRO, INT1_THS_YL, 0b01100100);
    I2C::writeRegister(GYRO, INT1_THS_ZH, 0b00000000);
    I2C::writeRegister(GYRO, INT1_THS_ZL, 0b01100100);
    
    I2C::writeRegister(GYRO, INT1_DURATION, 0b00000000); //disable wait on default
    I2C::writeRegister(GYRO, INT1_CFG , 0b00000000); //interrups default
    I2C::writeRegister(GYRO, CTRL_REG5, 0b00011000); //enable high pass filter (for interrupt?)
    I2C::writeRegister(GYRO, CTRL_REG1, 0b11101111); //turn on 800Hz, 50Hz cut-off, X, Y, Z enabled


    //set trim
    vTaskDelay(10 / portTICK_PERIOD_MS);
    setTrim(trimX, trimY, trimZ, 1000); //sample size 1000 (takes 5/4 seconds)

    accel::readAccel();                                                                       /////////////repareer
    rotation = accel::calcRot();
    printf("-------------------------- %f\t%f\t%f\t%f", rotation.w, rotation.x, rotation.y, rotation.z);
  }
  
  void gyroLoop(){
    if(isReady()){
      readGyro(gyroX, gyroY, gyroZ);
          
      timePast = esp_timer_get_time() - lastMicros;
      lastMicros = esp_timer_get_time();
      
      double rotX, rotY, rotZ;
      calcRot(rotX, rotY, rotZ, trimX, trimY, trimZ, timePast);
  
      #ifdef DEBUG_GYROREADBUFFER
        rotationBuffer.push_back(Quaternion(rotX, rotY, rotZ));
        if(rotationBuffer.size() > DEBUG_GYROREADBUFFER_MAXSIZE){rotationBuffer.erase(rotationBuffer.begin());}
      #endif
      
      rotation *= Quaternion(rotX, rotY, rotZ); //rotate with the measured amount
      rotation.setMagnitude(1);
    
      rotation.getEuler(yaw, roll, pitch);
    }
  }
}

