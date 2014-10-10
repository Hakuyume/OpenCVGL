#pragma once

#include <vector>
#include <Eigen/Eigen>

#define GRAVITY 98
#define P_PARAM 1000
#define D_PARAM 100

class Space;
class Particle;

class Space
{
public:
  std::vector<Particle> particles;
  void add_particle(Eigen::Vector3d pos);
};

class Particle
{
private:
  Eigen::Vector3d a;
public:
  Eigen::Vector3d p, v;
  void update_force(void);
  void update_position(double dt);
};
