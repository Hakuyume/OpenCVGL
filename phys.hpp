#pragma once

#include <Eigen/Eigen>
#include <vector>
#include <map>
#include <list>

class Particle;
class Space;

typedef std::list< Particle* > ParticlePtrs;
typedef std::vector<Particle> Particles;

class Particle
{
public:
  Eigen::Vector3d pos, vel, f;
  double rho, prs;
  void calc_amount(Space& space);
  void calc_force(Space& space);
  void advance(Space& space);
};

class Space
{
private:
  typedef long NeighborMapIdx;
  typedef std::map< NeighborMapIdx, ParticlePtrs > NeighborMap;
  NeighborMap* p_nbr_map;
  void new_neighbor_map(void);
  void insert_neighbor_map(Particle*);
  void delete_neighbor_map(void);
  NeighborMapIdx neighbor_map_idx(Eigen::Vector3d);
public:
  Particles particles;
  Eigen::Vector3d gravity;
  void put_particles(void);
  void update_particles(const double dt);
  ParticlePtrs neighbor(Eigen::Vector3d r);
};
