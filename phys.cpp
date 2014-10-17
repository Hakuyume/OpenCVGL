#include "phys.hpp"
#include <cmath>

static const double SPH_RESTDENSITY = 600.0;
static const double SPH_INTSTIFF    = 3.0;
static const double SPH_PMASS       = 0.00020543;
static const double SPH_SIMSCALE    = 0.004;
static const double KERNEL_SIZE     = 0.01;
static const double SPH_VISC        = 0.2;
static const double SPH_LIMIT       = 200.0;
static const double SPH_RADIUS      = 0.004;
static const double SPH_EPSILON     = 0.00001;
static const double SPH_EXTSTIFF    = 10000.0;
static const double SPH_EXTDAMP     = 256.0;
static const double SPH_PDIST       = pow(SPH_PMASS / SPH_RESTDENSITY, 1.0 / 3.0);
static const Eigen::Vector3d MIN(-15, -15, -10);
static const Eigen::Vector3d MAX(+15, +15, +10);
static const Eigen::Vector3d INIT_MIN(-10, -10, -5);
static const Eigen::Vector3d INIT_MAX(+10, +10, +5);

bool CompVector::operator()(const Eigen::Vector3d& a, const Eigen::Vector3d& b)
{
  double d = KERNEL_SIZE / SPH_SIMSCALE;

  for (int i = 0; i < 3; i++)
    if (floor(a(i) / d) < floor(b(i) / d))
      return true;
    else if (floor(a(i) / d) > floor(b(i) / d))
      return false;
  return false;
}

bool CompVector::operator()(const Eigen::Vector3i& a, const Eigen::Vector3i& b)
{
  for (int i = 0; i < 3; i++)
    if (a(i) < b(i))
      return true;
    else if (a(i) > b(i))
      return false;
  return false;
}

void Space::put_particles(void)
{
  double d = SPH_PDIST / SPH_SIMSCALE * 0.95;
  for (double x = INIT_MIN(0) + d; x <= INIT_MAX(0) - d; x += d)
    for (double y = INIT_MIN(1) + d; y <= INIT_MAX(1) - d; y += d)
      for (double z = INIT_MIN(2) + d; z <= INIT_MAX(2) - d; z += d){
	Particle p;
	p.pos << x, y, z;
	particles.push_back(p);
      }
}

void Space::update_neighbor_map(void)
{
  neighbor_map.clear();
  for (auto& pt : particles){
    auto iter = neighbor_map.find(pt.pos);
    if (iter == neighbor_map.end()){
      std::list<Particle*> pts;
      iter = neighbor_map.insert(iter, NeighborMap::value_type(pt.pos, pts));
    }
    iter->second.push_back(&pt);
  }
}

std::list<Particle*> Space::neighbor(const Eigen::Vector3d& r)
{
  std::list<Particle*> neighbors;
  double d = KERNEL_SIZE / SPH_SIMSCALE;

  for (int x = -1; x <= 1; x++)
    for (int y = -1; y <= 1; y++)
      for (int z = -1; z <= 1; z++){
	Eigen::Vector3d v(x, y, z);

	auto iter = neighbor_map.find(r + v * d);
	if (iter != neighbor_map.end())
	  for (auto& pt : iter->second)
	    neighbors.push_back(pt);
      }
  return neighbors;
}

void Space::update_particles(const double dt)
{
  update_neighbor_map();

  for (auto& pt : particles)
    pt.calc_amount(*this);
  for (auto& pt : particles)
    pt.calc_accel(*this);
  for (auto& pt : particles)
    pt.move(dt);
}

Particle::Particle(void)
{
  vel = Eigen::Vector3d::Zero();
}

double Particle::poly6kern(const Eigen::Vector3d& r)
{
  static const double k = 315.0 / (64.0 * M_PI * pow(KERNEL_SIZE, 9));

  double c = KERNEL_SIZE * KERNEL_SIZE - r.norm() * r.norm() * SPH_SIMSCALE * SPH_SIMSCALE;

  if (c > 0)
    return k * c * c * c;
  else
    return 0;
}

double Particle::lapkern(const Eigen::Vector3d& r)
{
  static const double k = 45.0 / (M_PI * pow(KERNEL_SIZE, 6));

  double c = KERNEL_SIZE - r.norm() * SPH_SIMSCALE;
  if (c > 0)
    return k * SPH_VISC * c;
  else
    return 0;
}

Eigen::Vector3d Particle::spikykern_grad(const Eigen::Vector3d& r)
{
  static const double k = 45.0 / (M_PI * pow(KERNEL_SIZE, 6));

  double c = KERNEL_SIZE - r.norm() * SPH_SIMSCALE;
  if (c > 0)
    return k * c * c * r / r.norm();
  else
    return Eigen::Vector3d::Zero();
}

void Particle::calc_amount(Space& space)
{
  rho = 0;
  for (auto& pt : space.neighbor(pos))
    rho += SPH_PMASS * poly6kern(pt->pos - pos);

  prs = (rho - SPH_RESTDENSITY) * SPH_INTSTIFF;
}

void Particle::calc_accel(Space& space)
{
  Eigen::Vector3d force_v(0, 0, 0);
  Eigen::Vector3d force_p(0, 0, 0);

  for (auto& pt : space.neighbor(pos)){
    if (pos == pt->pos) continue;
    force_v += (pt->vel - vel) * (SPH_PMASS / rho) * (SPH_PMASS / pt->rho) * lapkern(pt->pos - pos);
    force_p -= (prs + pt->prs) / 2 * (SPH_PMASS / rho) * (SPH_PMASS / pt->rho) * spikykern_grad(pt->pos - pos);
  }

  accel = (force_v + force_p) / SPH_PMASS;

  if (accel.norm() > SPH_LIMIT)
    accel *= SPH_LIMIT / accel.norm();

  for (int i = 0; i < 3; i++){
    double diff = 2.0 * SPH_RADIUS - (pos(i) - MIN(i)) * SPH_SIMSCALE;
    if (diff > SPH_EPSILON)
      accel(i) += SPH_EXTSTIFF * diff - SPH_EXTDAMP * vel(i);
    diff = 2.0 * SPH_RADIUS - (MAX(i) - pos(i)) * SPH_SIMSCALE;
    if (diff > SPH_EPSILON)
      accel(i) -= SPH_EXTSTIFF * diff + SPH_EXTDAMP * vel(i);
  }

  accel += space.gravity;
}

void Particle::move(const double dt)
{
  vel += accel * dt;
  pos += vel * dt / SPH_SIMSCALE;
}

