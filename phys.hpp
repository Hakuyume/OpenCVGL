#pragma once

#include <Eigen/Eigen>
#include <vector>
#include <map>
#include <list>

class Particle;
class Space;

typedef long NeighborMapIdx;
typedef std::list< Particle* > ParticlePtrs;
typedef std::map< NeighborMapIdx, ParticlePtrs > NeighborMap;
typedef std::vector<Particle> Particles;

class Particle
{
public:
  Eigen::Vector3d pos, vel, f;
  double rho, prs;
  void advance(const Space& space);
};

class Space
{
private:
  NeighborMap* p_nbr_map;
  void calc_amount(void);
  void calc_force(void);
public:
  Particles particles;
  Eigen::Vector3d gravity;
  void put_particles(void);
  void update_particles(const double dt);
  ParticlePtrs neighbor(Eigen::Vector3d r);
};
