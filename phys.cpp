#include "phys.hpp"

#define P_PARAM 1000
#define D_PARAM 100

#define KERNEL_SIZE 0.5

double kernel(const Eigen::Vector3d& r, const double h)
{
  if (r.norm() < h)
    return h / r.norm() - 1;
  else
    return 0;
}

Eigen::Vector3d kernel_grad(const Eigen::Vector3d& r, const double h)
{
  if (r.norm() < h)
    return - h / (r.norm() * r.norm() * r.norm()) * r;
  else
    return Eigen::Vector3d::Zero();
}

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
    pt.update_density(*this);

  for (auto& pt : this->particles)
    pt.update_velocity(*this, dt);

  for (auto& pt : this->particles)
    pt.update_position(dt);
}

void Particle::update_density(Space& space)
{
  space.find_neighbor(this->neighbor, this->p, 2 * KERNEL_SIZE);

  this->r = 0;
  for (auto& pt : this->neighbor)
    this->r += kernel(pt->p - this->p, KERNEL_SIZE);
}

void Particle::update_velocity(Space& space, const double dt)
{
  Eigen::Vector3d f_v(0, 0, 0);
  Eigen::Vector3d f_p(0, 0, 0);

  for (auto& pt : this->neighbor){
    f_v += 10 * (pt->v - this->v) / pt->r * kernel(pt->p - this->p, KERNEL_SIZE);
    f_p -= 1 * (pt->r + this->r - 3) / pt->r * kernel_grad(pt->p - this->p, KERNEL_SIZE);
  }

  Eigen::Vector3d f_e = space.gravity;
  for (int i = 0; i < 3; i++){
    if (this->p(i) < -BOXSIZE)
      f_e(i) -= P_PARAM * (this->p(i) + BOXSIZE) + D_PARAM * this->v(i);
    if (this->p(i) > +BOXSIZE)
      f_e(i) -= P_PARAM * (this->p(i) - BOXSIZE) + D_PARAM * this->v(i);
  }

  this->v += (f_v + f_p + f_e) * dt;
}

void Particle::update_position(const double dt)
{
  this->p += this->v * dt;
}
