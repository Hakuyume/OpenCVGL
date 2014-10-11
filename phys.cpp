#include "phys.hpp"

#define P_PARAM 1000
#define D_PARAM 100
#define V_PARAM 1

#define KERNEL_SIZE 0.5

double kernel(const Eigen::Vector3d& r, const double h)
{
  double q = r.norm() / h;
  double k = 1.0 / (M_PI * h * h * h);

  if (0 <= q && q <= 1)
    return k * (1 - 1.5 * q + 0.75 * q * q);
  else if (1 < q && q <= 2)
    return k * (2 - q) * (2 - q) * (2 - q);
  else
    return 0;
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
  for (auto& pt : this->neighbor)
    f_v += V_PARAM * (pt->v - this->v) / pt->r * kernel(pt->p - this->p, KERNEL_SIZE);

  Eigen::Vector3d f_p(0, 0, 0);

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
