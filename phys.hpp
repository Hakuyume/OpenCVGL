#pragma once

#include <Eigen/Eigen>
#include <vector>
#include <map>
#include <list>

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
  void calc_force(Space& space);
  void advance(Space& space);
};

class Space
{
private:
  typedef long NeighborMapIdx;
  typedef std::map<NeighborMapIdx, std::list<Particle*> > NeighborMap;
  NeighborMap* p_nbr_map;
  void new_neighbor_map(void);
  void insert_neighbor_map(Particle*);
  void delete_neighbor_map(void);
  NeighborMapIdx neighbor_map_idx(const Eigen::Vector3d&);
public:
  std::vector<Particle> particles;
  Eigen::Vector3d gravity;
  void put_particles(void);
  void update_particles(const double dt);
  std::list<Particle*> neighbor(const Eigen::Vector3d& r);
};
