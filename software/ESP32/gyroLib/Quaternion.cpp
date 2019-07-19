#include "Quaternion.h"
#include <cmath>

Quaternion::Quaternion(double wi, double xi, double yi, double zi){
  w = wi; x = xi; y = yi; x = xi;
}

Quaternion::Quaternion(){
  w = 1; x = 0; y = 0; x = 0;
}

Quaternion::Quaternion(double g_x, double g_y, double g_z){  //creates quaternion from gyro rotations (euler angles)
  double cx = cos(g_x/2), cy = cos(g_y/2), cz = cos(g_z/2);
  double sx = sin(g_x/2), sy = sin(g_y/2), sz = sin(g_z/2);

  w = (cx * cy * cz) - (sx * sy * sz);
  x = (sx * cy * cz) + (cx * sy * sz);
  y = (cx * sy * cz) - (sx * cy * sz);
  z = (sx * sy * cz) + (cx * cy * sz);
}

Quaternion::~Quaternion(){
}


double* Quaternion::getValue(){
  double tmp[4] = {w, x, y, z};
  return tmp;
}

void Quaternion::getEuler(double& yaw, double& roll, double& pitch){   //~90 microseconds  ,  euler in XYZ                      ///////////CODE VAN WIKIPEDIA, EULER is niet helemaal correct
  // roll (x-axis rotation)
  double sinr_cosp = +2.0 * (w * x + y * z);
  double cosr_cosp = +1.0 - 2.0 * (x * x + y * y);
  roll = atan2(sinr_cosp, cosr_cosp);

  // pitch (y-axis rotation)
  double sinp = +2.0 * (w * y - z * x);
  if (fabs(sinp) >= 1)
    pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
  else
    pitch = asin(sinp);

  // yaw (z-axis rotation)
  double siny_cosp = +2.0 * (w * z + x * y);
  double cosy_cosp = +1.0 - 2.0 * (y * y + z * z);  
  yaw = atan2(siny_cosp, cosy_cosp);
}

void Quaternion::setMagnitude(double const dist){   //~40 microseconds
  double factor = dist/sqrt((w*w + x*x + y*y + z*z));
  w *= factor;
  x *= factor;
  y *= factor;
  z *= factor;
}

Quaternion Quaternion::operator* (Quaternion const& q2){  //~80 microseconds
  Quaternion qnew = Quaternion();
  qnew.w = (w * q2.w) - (x * q2.x) - (y * q2.y) - (z * q2.z);
  qnew.x = (x * q2.w) + (w * q2.x) - (z * q2.y) + (y * q2.z);
  qnew.y = (y * q2.w) + (z * q2.x) + (w * q2.y) - (x * q2.z);
  qnew.z = (z * q2.w) - (y * q2.x) + (x * q2.y) + (w * q2.z);
  return qnew;
}

Quaternion& Quaternion::operator*= (Quaternion const& q2){  //~80 microseconds
  w = (w * q2.w) - (x * q2.x) - (y * q2.y) - (z * q2.z);
  x = (x * q2.w) + (w * q2.x) - (z * q2.y) + (y * q2.z);
  y = (y * q2.w) + (z * q2.x) + (w * q2.y) - (x * q2.z);
  z = (z * q2.w) - (y * q2.x) + (x * q2.y) + (w * q2.z);
  return *this;
}

