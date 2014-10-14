#pragma once

#include <vector>
#include <Eigen/Eigen>

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
  double rho;
  std::vector<Particle*> neighbor;
public:
  Eigen::Vector3d p, v;
  void update_density(Space& space);
  void update_velocity(Space& space, const double dt);
  void update_position(const double dt);
};
