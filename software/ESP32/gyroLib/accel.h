#ifndef ACCEL_H
#define ACCEL_H

#include "Quaternion.h"

//ADXL345
#define ACCEL             0x53
#define DEVID           0x00
#define OFSX            0x1E
#define OFSY            0x1F
#define OFSZ            0x20
#define THRESH_ACT      0x24
#define THRESH_INACT    0x25
#define TIME_INACT      0x26
#define ACT_INACT_CTL   0x27
#define THRESH_FF       0x28
#define TIME_FF         0x29
#define BW_RATE         0x2C
#define POWER_CTL       0x2D
#define INT_ENABLE      0x2E
#define INT_MAP         0x2F
#define INT_SOURCE      0x30
#define DATA_FORMAT     0x31
#define DATAX0          0x32
#define DATAX1          0x33
#define DATAY0          0x34
#define DATAY1          0x35
#define DATAZ0          0x36
#define DATAZ1          0x37
#define FIFO_CTL        0x38
#define FIFO_STATUS     0x39

namespace accel {
  extern double roll, pitch;
  extern short accelX, accelY, accelZ;
  extern Quaternion rotation;
  
  void accelLoop();
  void init();
  bool isReady();
  void readAccel(short& accelX, short& accelY, short& accelZ);
  void readAccel();
  void calibrate(short& accelX, short& accelY, short& accelZ);
  void calcRot(double& roll, double& pitch);
  Quaternion calcRot();
};

#endif
