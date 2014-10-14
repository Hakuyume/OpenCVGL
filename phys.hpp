#pragma once

#include <Eigen/Eigen>
#include <vector>

struct Particle
{
  Eigen::Vector3d pos, vel, f;
  double rho, prs;
};

typedef std::vector<Particle> Particles;

void simulation( Particles* );
Particles* new_particles(void);

extern Eigen::Vector3d g;
