#pragma once

#include <Eigen/Eigen>
#include <vector>
#include <map>
#include <list>

struct Particle
{
  Eigen::Vector3d pos, vel, f;
  double rho, prs;
};

typedef long NeighborMapIdx;
typedef std::list< Particle* > ParticlePtrs;
typedef std::map< NeighborMapIdx, ParticlePtrs > NeighborMap;
typedef std::vector<Particle> Particles;

class Space
{
private:
  void calc_amount(NeighborMap* p_nbr_map);
  void calc_force(NeighborMap* p_nbr_map);
  void advance(NeighborMap* p_nbr_map);
public:
  Particles particles;
  Eigen::Vector3d gravity;
  void put_particles(void);
  void update_particles(const double dt);
};
