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
    if (pos != it->p && (pos - it->p).norm() <= r)
      neighbor.push_back(&(*it));
}

void Space::update_particles(const double dt)
{
  std::vector<Particle>::iterator it;

  for (it = this->particles.begin(); it != this->particles.end(); it++)
    it->update_force(*this);

  for (it = this->particles.begin(); it != this->particles.end(); it++)
    it->update_position(dt);
}

void Particle::update_force(Space space)
{
  this->a << 0, -GRAVITY, 0;

  for (int i = 0; i < 3; i++){
    if (this->p(i) < -BOXSIZE)
      this->a(i) -= P_PARAM * (this->p(i) + BOXSIZE) + D_PARAM * this->v(i);
    if (this->p(i) > +BOXSIZE)
      this->a(i) -= P_PARAM * (this->p(i) - BOXSIZE) + D_PARAM * this->v(i);
  }

  std::vector<Particle*> neighbor;
  space.find_neighbor(neighbor, this->p, 1);

  std::vector<Particle*>::iterator it;
  for (it = neighbor.begin(); it != neighbor.end(); it++){
    double r = ((*it)->p - this->p).norm();
    this->a -= P_PARAM * (1 - r) * ((*it)->p - this->p) / r;
    this->a += D_PARAM * (1 - r) * ((*it)->v - this->v);
  }
}

void Particle::update_position(const double dt)
{
  this->p += (this->v + this->a * dt / 2) * dt;
  this->v += this->a * dt;
}
