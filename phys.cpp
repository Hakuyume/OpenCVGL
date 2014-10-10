#include "phys.hpp"

void Space::add_particle(Eigen::Vector3d pos)
{
  Particle pt;
  pt.p = pos;
  pt.v << 0, 0, 0;
  this->particles.push_back(pt);
}

void Particle::update_force(void)
{
  this->a << 0, -GRAVITY, 0;

  if (this->p(1) < 0)
    this->a(1) -= P_PARAM * this->p(1) + D_PARAM * this->v(1);
}

void Particle::update_position(double dt)
{
  this->p += (this->v + this->a * dt / 2) * dt;
  this->v += this->a * dt;
}
