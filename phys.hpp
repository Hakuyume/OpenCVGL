#pragma once

#include <Eigen/Eigen>
#include <vector>
#include <map>
#include <list>

struct CompVector {
  bool operator()(const Eigen::Vector3d& a, const Eigen::Vector3d& b);
};

class Particle;
class Space;

class Particle
{
private:
  double rho, prs;
  Eigen::Vector3d vel, accel;
  double poly6kern(const Eigen::Vector3d& r);
  double lapkern(const Eigen::Vector3d& r);
  Eigen::Vector3d spikykern_grad(const Eigen::Vector3d& r);
public:
  Eigen::Vector3d pos;
  Particle(void);
  void calc_amount(Space& space);
  void calc_accel(Space& space);
  void move(const double dt);
};

class Space
{
private:
  typedef std::map<Eigen::Vector3d, std::list<Particle*>, CompVector> NeighborMap;
  NeighborMap neighbor_map;
  void update_neighbor_map(void);
public:
  std::vector<Particle> particles;
  Eigen::Vector3d gravity;
  void put_particles(void);
  void update_particles(const double dt);
  std::list<Particle*> neighbor(const Eigen::Vector3d& r);
};
