#ifndef GYRO_H
#define GYRO_H

#include "Quaternion.h"
#include <vector>

//#define DEBUG_GYROREADBUFFER      //Slows down gyro reading and fills up serial monitor
#define DEBUG_GYROREADBUFFER_MAXSIZE 10

//L3G4200D
#define GYRO            0x69
#define OUT_X_L       0x28
#define OUT_X_H       0x29
#define OUT_Y_L       0x2a
#define OUT_Y_H       0x2b
#define OUT_Z_L       0x2c
#define OUT_Z_H       0x2d

#define WHOAMI        0x0f
#define CTRL_REG1     0x20
#define CTRL_REG2     0x21
#define CTRL_REG3     0x22
#define CTRL_REG4     0x23
#define CTRL_REG5     0x24
#define REFERENCE     0x25
#define INT1_CFG      0x30
#define INT1_DURATION 0x38
#define STATUS_REG    0x27

#define INT1_THS_XH   0x32
#define INT1_THS_XL   0x33
#define INT1_THS_YH   0x34
#define INT1_THS_YL   0x35
#define INT1_THS_ZH   0x36
#define INT1_THS_ZL   0x37

namespace gyro {
  #ifdef DEBUG_GYROREADBUFFER
  extern std::vector<Quaternion> rotationBuffer;
  #endif
  
  extern short gyroX, gyroY, gyroZ;  //2 bytes (just like the register)
  extern double trimX, trimY, trimZ;
  extern long loopAmount; // to not devide by zero
  extern long plpbeg;
  extern double roll, yaw, pitch;
  extern long test;
  extern long test1;
  extern Quaternion rotation;
  extern long lastMicros;
  extern int timePast;
  
  void gyroLoop();
  void init();
  void setTrim(double& TrimXI, double& TrimYI, double& TrimZI, int samplesize);
  bool isReady();
  void calcRot(double& rotX, double& rotY, double& rotZ, int trimX, int trimY, int trimZ, long timePast);
  void readGyro(short& gyroX, short& gyroY, short& gyroZ);
}

#endif
