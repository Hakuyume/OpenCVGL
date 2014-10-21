#pragma once

#include <Eigen/Eigen>
#include <vector>
#include <map>
#include <list>
#include <thread>
#include "comp.hpp"
#include "barrier.hpp"

class Particle;
class Space;

class Particle
{
private:
  double rho, prs;
  Eigen::Vector3d vel, accel;
  std::list<Particle *> neighbors;
  double poly6kern(const Eigen::Vector3d &r);
  double lapkern(const Eigen::Vector3d &r);
  Eigen::Vector3d spikykern_grad(const Eigen::Vector3d &r);

public:
  Eigen::Vector3d pos;
  Particle(void);
  void calc_amount(Space &space);
  void calc_accel(Space &space);
  void move(const double dt);
};

class Space
{
private:
  Barrier br;
  typedef std::map<Eigen::Vector3d, std::list<Particle *>, CompVector> NeighborMap;
  NeighborMap neighbor_map;
  void update_neighbor_map(void);
  static void update_particles(Space &space, const size_t id);

public:
  std::vector<Particle> particles;
  Eigen::Vector3d gravity;
  void put_particles(size_t n);
  std::list<Particle *> neighbor(const Eigen::Vector3d &r);
  void start_simulate(std::vector<std::thread> &threads);
};
