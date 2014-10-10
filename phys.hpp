#pragma once

#include <vector>
#include <Eigen/Eigen>

#define GRAVITY 98
#define P_PARAM 1000
#define D_PARAM 100

class Particle
{
public:
  Eigen::Vector3d p, v, a;
  void update_force(void);
  void update_position(double dt);
};
