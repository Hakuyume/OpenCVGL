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
  Eigen::Vector3d vel, accel, d_color;
  std::list<Particle *> neighbors;
  static double poly6kern(const Eigen::Vector3d &r);
  static double lapkern(const Eigen::Vector3d &r);
  static Eigen::Vector3d spikykern_grad(const Eigen::Vector3d &r);

public:
  bool alive;
  Eigen::Vector3d pos;
  Eigen::Vector3d color;
  Particle(void);
  void calc_amount(Space &space);
  void calc_accel(Space &space);
  void move(const double dt);
};

class ParticleInfo
{
public:
  Eigen::Vector3d pos, color;
  ParticleInfo(const Particle &pt);
  ParticleInfo(const Eigen::Vector3d &pos, const Eigen::Vector3d &color);
};

class Space
{
  typedef std::map<Eigen::Vector3d, std::list<Particle *>, CompVector> NeighborMap;

private:
  std::mutex mutex;
  Barrier br;
  NeighborMap neighbor_map;
  void pre_calc(void);
  std::vector<Particle> particles;
  std::vector<ParticleInfo> particle_info;
  std::vector<Particle> add_queue;
  bool rm;
  Eigen::Vector3d rm_pos;
  static void update_particles(Space &space, const size_t id);
  std::vector<std::thread> threads;
  bool simulate;

public:
  Eigen::Vector3d size, gravity;
  std::vector<ParticleInfo> get_partilce_info(void);
  void add_particle(const ParticleInfo &pt_info);
  void remove_particle(const Eigen::Vector3d &pos);
  void put_particles(size_t n, const Eigen::Vector3d &color);
  void neighbor(const Eigen::Vector3d &r, std::list<Particle *> &neigbors) const;
  void start_simulate(size_t n);
  void stop_simulate(void);
};
