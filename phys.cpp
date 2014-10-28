#include "phys.hpp"
#include <cmath>
#include <chrono>

static const double DT_MAX = 0.004;
static const double SPH_RESTDENSITY = 600.0;
static const double SPH_INTSTIFF = 3.0;
static const double SPH_PMASS = 0.00020543;
static const double SPH_SIMSCALE = 0.004;
static const double KERNEL_SIZE = 0.01;
static const double SPH_VISC = 0.2;
static const double SPH_LIMIT = 100.0;
static const double SPH_RADIUS = 0.004;
static const double SPH_EPSILON = 0.00001;
static const double SPH_EXTSTIFF = 10000.0;
static const double SPH_EXTDAMP = 256.0;

std::vector<Eigen::Vector3d> Space::positions(void)
{
  std::lock_guard<std::mutex> lock{mutex};
  return poses;
}

void Space::add_particle(const Eigen::Vector3d &pos)
{
  Particle p;
  p.pos = pos;

  mutex.lock();
  add_queue.push_back(p);
  mutex.unlock();
}

void Space::remove_particle(const Eigen::Vector3d &pos)
{
  mutex.lock();
  rm = true;
  rm_pos = pos;
  mutex.unlock();
}

void Space::put_particles(size_t n)
{
  for (size_t i = 0; i < n; i++)
    add_particle(Eigen::Vector3d::Random() * cbrt(n * SPH_PMASS / SPH_RESTDENSITY) / SPH_SIMSCALE);
}

void Space::update_neighbor_map(void)
{
  mutex.lock();

  particles.insert(particles.end(), add_queue.begin(), add_queue.end());
  add_queue.clear();

  neighbor_map.clear();
  poses.clear();

  for (auto &pt : particles) {
    if (rm && (pt.pos - rm_pos).norm() < KERNEL_SIZE / SPH_SIMSCALE)
      pt.alive = false;
    if (!pt.alive)
      continue;
    auto iter = neighbor_map.find(pt.pos / (KERNEL_SIZE / SPH_SIMSCALE));
    if (iter == neighbor_map.end()) {
      std::list<Particle *> pts;
      iter = neighbor_map.insert(iter, NeighborMap::value_type{pt.pos / (KERNEL_SIZE / SPH_SIMSCALE), pts});
    }
    iter->second.push_back(&pt);
    poses.push_back(pt.pos);
  }

  rm = false;

  mutex.unlock();
}

void Space::neighbor(const Eigen::Vector3d &r, std::list<Particle *> &neighbors) const
{
  neighbors.clear();
  for (int x = -1; x <= 1; x++)
    for (int y = -1; y <= 1; y++)
      for (int z = -1; z <= 1; z++) {
        Eigen::Vector3d v{(double)x, (double)y, (double)z};

        auto iter = neighbor_map.find(r / (KERNEL_SIZE / SPH_SIMSCALE) + v);
        if (iter != neighbor_map.end())
          for (auto &pt : iter->second)
            neighbors.push_back(pt);
      }
}

void Space::update_particles(Space &space, const size_t id)
{
  size_t threads = space.br.size();

  auto start = std::chrono::high_resolution_clock::now();

  while (true) {
    if (id == 0)
      space.update_neighbor_map();
    space.br.wait();

    for (size_t i = id; i < space.particles.size(); i += threads)
      space.particles.at(i).calc_amount(space);
    space.br.wait();

    for (size_t i = id; i < space.particles.size(); i += threads)
      space.particles.at(i).calc_accel(space);
    space.br.wait();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dt = end - start;
    start = end;

    for (size_t i = id; i < space.particles.size(); i += threads)
      space.particles.at(i).move(dt.count() < DT_MAX ? dt.count() : DT_MAX);
    space.br.wait();
  }
}

void Space::start_simulate(std::vector<std::thread> &threads)
{
  br.size(threads.size());

  for (size_t id = 0; id < threads.size(); id++)
    threads.at(id) = std::thread{update_particles, std::ref(*this), id};
}

Particle::Particle(void)
    : vel{Eigen::Vector3d::Zero()}
{
  alive = true;
}

double Particle::poly6kern(const Eigen::Vector3d &r)
{
  static const double k = 315.0 / (64.0 * M_PI * pow(KERNEL_SIZE, 9));

  double c = KERNEL_SIZE * KERNEL_SIZE - r.norm() * r.norm() * SPH_SIMSCALE * SPH_SIMSCALE;

  if (c > 0)
    return k * c * c * c;
  else
    return 0;
}

double Particle::lapkern(const Eigen::Vector3d &r)
{
  static const double k = 45.0 / (M_PI * pow(KERNEL_SIZE, 6));

  double c = KERNEL_SIZE - r.norm() * SPH_SIMSCALE;
  if (c > 0)
    return k * SPH_VISC * c;
  else
    return 0;
}

Eigen::Vector3d Particle::spikykern_grad(const Eigen::Vector3d &r)
{
  static const double k = 45.0 / (M_PI * pow(KERNEL_SIZE, 6));

  double c = KERNEL_SIZE - r.norm() * SPH_SIMSCALE;
  if (c > 0)
    return k * c * c * r / r.norm();
  else
    return Eigen::Vector3d::Zero();
}

void Particle::calc_amount(Space &space)
{
  space.neighbor(pos, neighbors);

  rho = 0;
  for (auto &pt : neighbors)
    rho += SPH_PMASS * poly6kern(pt->pos - pos);

  prs = (rho - SPH_RESTDENSITY) * SPH_INTSTIFF;
}

void Particle::calc_accel(Space &space)
{
  Eigen::Vector3d force_v{0, 0, 0};
  Eigen::Vector3d force_p{0, 0, 0};

  for (auto &pt : neighbors) {
    if (pos == pt->pos)
      continue;
    force_v += (pt->vel - vel) * (SPH_PMASS / rho) * (SPH_PMASS / pt->rho) * lapkern(pt->pos - pos);
    force_p -= (prs + pt->prs) / 2 * (SPH_PMASS / rho) * (SPH_PMASS / pt->rho) * spikykern_grad(pt->pos - pos);
  }

  accel = (force_v + force_p) / SPH_PMASS;

  if (accel.norm() > SPH_LIMIT)
    accel *= SPH_LIMIT / accel.norm();

  for (int i = 0; i < 3; i++) {
    double diff = 2.0 * SPH_RADIUS - (pos(i) + space.size(i)) * SPH_SIMSCALE;
    if (diff > SPH_EPSILON)
      accel(i) += SPH_EXTSTIFF * diff - SPH_EXTDAMP * vel(i);
    diff = 2.0 * SPH_RADIUS - (space.size(i) - pos(i)) * SPH_SIMSCALE;
    if (diff > SPH_EPSILON)
      accel(i) -= SPH_EXTSTIFF * diff + SPH_EXTDAMP * vel(i);
  }

  accel += space.gravity;
}

void Particle::move(const double dt)
{
  vel += accel * dt;
  pos += vel * dt / SPH_SIMSCALE;
  pos(2) = 0;
}
