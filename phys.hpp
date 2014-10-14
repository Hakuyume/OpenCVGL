#pragma once

#include <vector>

struct vec3 {
  double x, y, z;
  vec3() {
    x=0; y=0; z=0;
  }
  vec3( const double _x, const double _y, const double _z ) {
    x=_x; y=_y;z=_z;
  }
};

struct Particle
{
  vec3 pos, vel, f;
  double rho, prs;
};

typedef std::vector<Particle> Particles;

void simulation( Particles* );
Particles* new_particles(void);
