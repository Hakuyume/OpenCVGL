#include "phys.hpp"

void Space::add_particle(const Eigen::Vector3d& pos)
{
  Particle pt;
  pt.p = pos;
  pt.v << 0, 0, 0;
  this->particles.push_back(pt);
}

void Space::find_neighbor(std::vector<Particle*>& neighbor, const Eigen::Vector3d& pos, const double r)
{
  neighbor.clear();

  for (auto& pt : this->particles)
    if (pos != pt.p && (pos - pt.p).norm() <= r)
      neighbor.push_back(&pt);
}

void Space::update_particles(const double dt)
{
  for (auto& pt : this->particles)
    pt.update_force(*this);

  for (auto& pt : this->particles)
    pt.update_position(dt);
}

void Particle::update_force(Space& space)
{
  this->a = space.gravity;

  for (int i = 0; i < 3; i++){
    if (this->p(i) < -BOXSIZE)
      this->a(i) -= P_PARAM * (this->p(i) + BOXSIZE) + D_PARAM * this->v(i);
    if (this->p(i) > +BOXSIZE)
      this->a(i) -= P_PARAM * (this->p(i) - BOXSIZE) + D_PARAM * this->v(i);
  }

  std::vector<Particle*> neighbor;
  space.find_neighbor(neighbor, this->p, 1);

  for (auto& pt : neighbor){
    double r = (pt->p - this->p).norm();
    this->a -= P_PARAM * (1 - r) * (pt->p - this->p) / r;
    this->a += D_PARAM * (1 - r) * (pt->v - this->v);
  }
}

void Particle::update_position(const double dt)
{
  this->p += (this->v + this->a * dt / 2) * dt;
  this->v += this->a * dt;
}
