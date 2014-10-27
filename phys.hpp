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
  static double poly6kern(const Eigen::Vector3d &r);
  static double lapkern(const Eigen::Vector3d &r);
  static Eigen::Vector3d spikykern_grad(const Eigen::Vector3d &r);

public:
  Eigen::Vector3d pos;
  Particle(void);
  void calc_amount(Space &space);
  void calc_accel(Space &space);
  void move(const double dt);
};

class Space
{
  typedef std::map<Eigen::Vector3d, std::list<Particle *>, CompVector> NeighborMap;

private:
  Barrier br;
  NeighborMap neighbor_map;
  void update_neighbor_map(void);
  std::vector<Particle> particles;
  static void update_particles(Space &space, const size_t id);

public:
  Eigen::Vector3d gravity;
  std::vector<Particle>::const_iterator begin(void) const;
  std::vector<Particle>::const_iterator end(void) const;
  void put_particle(const Eigen::Vector3d &pos);
  void put_particles(size_t n);
  std::list<Particle *> neighbor(const Eigen::Vector3d &r) const;
  void start_simulate(std::vector<std::thread> &threads);
};
