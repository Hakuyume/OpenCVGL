#include "phys.hpp"

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
