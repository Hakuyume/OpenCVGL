#pragma once

#include <vector>
#include <Eigen/Eigen>

#define P_PARAM 1000
#define D_PARAM 100

#define BOXSIZE 10

class Space;
class Particle;

class Space
{
public:
  Eigen::Vector3d gravity;
  std::vector<Particle> particles;
  void add_particle(const Eigen::Vector3d& pos);
  void find_neighbor(std::vector<Particle*>& neighbor, const Eigen::Vector3d& pos, const double r);
  void update_particles(const double dt);
};

class Particle
{
private:
  Eigen::Vector3d a;
public:
  Eigen::Vector3d p, v;
  void update_force(Space& space);
  void update_position(const double dt);
};
