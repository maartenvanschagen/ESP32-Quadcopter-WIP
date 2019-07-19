#ifndef QUATERNION_H
#define QUATERNION_H

class Quaternion {
  private:
    
  public:
    double w, x, y, z;
    Quaternion(double w, double x, double y, double z);
    Quaternion(double g_x, double g_y, double g_z);
    Quaternion();
    ~Quaternion();
    
    double* getValue();
    void getEuler(double& yaw, double& roll, double& pitch);
    void setMagnitude(double const dist);
    
    Quaternion operator* (Quaternion const& q2);
    Quaternion& operator*= (Quaternion const& q2);
};

#endif
