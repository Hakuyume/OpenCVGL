#include "phys.hpp"

void Space::add_particle(Eigen::Vector3d pos)
{
  Particle pt;
  pt.p = pos;
  pt.v << 0, 0, 0;
  this->particles.push_back(pt);
}

void Space::find_neighbor(std::vector<Particle*>& neighbor, const Eigen::Vector3d& pos, const double r)
{
  neighbor.clear();

  std::vector<Particle>::iterator it;
  for (it = this->particles.begin(); it != this->particles.end(); it++)
    if ((pos - it->p).norm() <= r)
      neighbor.push_back(&(*it));
}

void Particle::update_force(Space space)
{
  this->a << 0, -GRAVITY, 0;

  if (this->p(1) < 0)
    this->a(1) -= P_PARAM * this->p(1) + D_PARAM * this->v(1);

  std::vector<Particle*> neighbor;
  space.find_neighbor(neighbor, this->p, 0.5);

  std::vector<Particle*>::iterator it;
  for (it = neighbor.begin(); it != neighbor.end(); it++)
    this->a -= P_PARAM * ((*it)->p - this->p);
}

void Particle::update_position(const double dt)
{
  this->p += (this->v + this->a * dt / 2) * dt;
  this->v += this->a * dt;
}
