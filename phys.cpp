#include "phys.hpp"
#include <cmath>

static const double SPH_RESTDENSITY = 600.0;
static const double SPH_INTSTIFF    = 3.0;
static const double SPH_PMASS       = 0.00020543;
static const double SPH_SIMSCALE    = 0.004;
static const double H               = 0.01;
static const double DT              = 0.004;
static const double SPH_VISC        = 0.2;
static const double SPH_LIMIT       = 200.0;
static const double SPH_RADIUS      = 0.004;
static const double SPH_EPSILON     = 0.00001;
static const double SPH_EXTSTIFF    = 10000.0;
static const double SPH_EXTDAMP     = 256.0;
static const double SPH_PDIST       = pow(SPH_PMASS / SPH_RESTDENSITY,
                                          1.0/3.0 );
static const Eigen::Vector3d   MIN(  0.0,  0.0, -10.0 );
static const Eigen::Vector3d   MAX( 20.0, 20.0,  10.0 );
static const Eigen::Vector3d   INIT_MIN(  0.0,  0.0, -10.0 );
static const Eigen::Vector3d   INIT_MAX( 10.0, 20.0,  10.0 );
static const double SpikyKern       = -45.0 / ( M_PI * pow( H, 6 ) );
static const double LapKern         = 45.0 / ( M_PI * pow( H, 6 ) );

void Space::put_particles(void)
{
  double d = SPH_PDIST / SPH_SIMSCALE * 0.95;
  for ( double x = INIT_MIN(0)+d; x <= INIT_MAX(0)-d; x += d )
    for ( double y = INIT_MIN(1)+d; y <= INIT_MAX(1)-d; y += d )
      for ( double z = INIT_MIN(2)+d; z <= INIT_MAX(2)-d; z += d ){
	Particle p;
	p.pos << x, y, z;
	this->particles.push_back(p);
      }
}

void Space::new_neighbor_map(void)
{
  this->p_nbr_map = new NeighborMap();
  for (auto& pt : this->particles)
    this->insert_neighbor_map(&pt);
}

void Space::insert_neighbor_map(Particle* pt)
{
  NeighborMapIdx ix = neighbor_map_idx(pt->pos);
  NeighborMap::iterator iter = this->p_nbr_map->find(ix);
  if (iter != p_nbr_map->end())
    iter->second.push_back(pt);
  else{
    std::list<Particle*> ptrs;
    ptrs.push_back(pt);
    this->p_nbr_map->insert(NeighborMap::value_type(ix, ptrs));
  }
}

void Space::delete_neighbor_map(void)
{
  delete this->p_nbr_map;
}

std::list<Particle*> Space::neighbor(const Eigen::Vector3d& r)
{
  std::list<Particle*> ptrs;
  double d = H / SPH_SIMSCALE;

  for ( int x=-1; x<2; x++ )
    for ( int y=-1; y<2; y++ )
      for ( int z=-1; z<2; z++ ){
	Eigen::Vector3d v(x, y, z);
	v = r + v * d;

	if (
	    MIN(0) <= v(0) && v(0) <= MAX(0) &&
	    MIN(1) <= v(1) && v(1) <= MAX(1) &&
	    MIN(2) <= v(2) && v(2) <= MAX(2)){
          NeighborMapIdx ix = neighbor_map_idx(v);
          NeighborMap::iterator x = this->p_nbr_map->find(ix);
          if (x != p_nbr_map->end())
	    for (auto& pt : x->second)
	      ptrs.push_back(pt);
        }
      }
  return ptrs;
}

Space::NeighborMapIdx Space::neighbor_map_idx(const Eigen::Vector3d& r)
{
  int x, y, z;
  int mx, my;
  double d;
  d  = H / SPH_SIMSCALE;
  x  = floor( (r(0) - MIN(0)) / d );
  y  = floor( (r(1) - MIN(1)) / d );
  z  = floor( (r(2) - MIN(2)) / d );
  mx = floor( (MAX(0) - MIN(0)) / d );
  my = floor( (MAX(1) - MIN(1)) / d );
  return x + y * mx + z * mx * my;
}

void Space::update_particles(const double dt)
{
  this->new_neighbor_map();
  for (auto& pt : this->particles)
    pt.calc_amount(*this);
  for (auto& pt : this->particles)
    pt.calc_force(*this);
  for (auto& pt : this->particles)
    pt.advance(*this);
  this->delete_neighbor_map();
}

Particle::Particle(void)
{
  this->vel = Eigen::Vector3d::Zero();
}

double Particle::poly6kern(const Eigen::Vector3d& r)
{
  static const double k = 315.0 / (64.0 * M_PI * pow(H, 9));

  double c = H * H - r.norm() * r.norm() * SPH_SIMSCALE * SPH_SIMSCALE;

  if (c > 0)
    return k * c * c * c;
  else
    return 0;
}

void Particle::calc_amount(Space& space)
{
  this->rho  = 0.0;
  for (auto& pt : space.neighbor(this->pos))
    this->rho += SPH_PMASS * poly6kern(pt->pos - this->pos);

  this->prs = (this->rho - SPH_RESTDENSITY) * SPH_INTSTIFF;
  this->rho = 1.0 / this->rho;
}

void Particle::calc_force(Space& space)
{
  double pterm, vterm, r, c;
  Eigen::Vector3d dr, force, fcurr;
  
  force << 0.0, 0.0, 0.0;

  for (auto& pt : space.neighbor(this->pos)){
    if ( this->pos == pt->pos ) continue;
    dr = (this->pos - pt->pos) * SPH_SIMSCALE;
    r  = dr.norm();
    if (H > r){
	c = H - r;
	pterm = -0.5 * c * SpikyKern * (this->prs + pt->prs) / r;
	vterm = LapKern * SPH_VISC;
	fcurr = pterm * dr + vterm * (pt->vel - this->vel);
	fcurr *= c * this->rho * pt->rho;
	force += fcurr;
    }
  }
  this->f = force;
}

void Particle::advance(Space& space)
{
  Eigen::Vector3d accel, norm;
  double diff, adj;
  
  accel = this->f * SPH_PMASS;

  if (accel.norm() > SPH_LIMIT)
    accel *= SPH_LIMIT / accel.norm();

  // Z-axis walls
  diff = 2.0 * SPH_RADIUS - ( this->pos(2) - MIN(2) ) * SPH_SIMSCALE;
  if ( diff > SPH_EPSILON )
    {
      norm << 0.0, 0.0, 1.0;
      adj = SPH_EXTSTIFF * diff - SPH_EXTDAMP * this->vel(2);
      accel += adj * norm;
    }
  diff = 2.0 * SPH_RADIUS - ( MAX(2) - this->pos(2) ) * SPH_SIMSCALE;
  if ( diff > SPH_EPSILON )
    {
      norm << 0.0, 0.0, -1.0;
      adj = SPH_EXTSTIFF * diff + SPH_EXTDAMP * this->vel(2);
      accel += adj * norm;
    }
  
  // X-axis walls
  diff = 2.0 * SPH_RADIUS - ( this->pos(0) - MIN(0) ) * SPH_SIMSCALE;
  if ( diff > SPH_EPSILON )
    {
      norm << 1.0, 0.0, 0.0;
      adj = SPH_EXTSTIFF * diff - SPH_EXTDAMP * this->vel(0);
      accel += adj * norm;
    }
  diff = 2.0 * SPH_RADIUS - ( MAX(0) - this->pos(0) ) * SPH_SIMSCALE;
  if ( diff > SPH_EPSILON )
    {
      norm << -1.0, 0.0, 0.0;
      adj = SPH_EXTSTIFF * diff + SPH_EXTDAMP * this->vel(0);
      accel += adj * norm;
    }
  
  // Y-axis walls
  diff = 2.0 * SPH_RADIUS - ( this->pos(1) - MIN(1) ) * SPH_SIMSCALE;
  if ( diff > SPH_EPSILON )
    {
      norm << 0.0, 1.0, 0.0;
      adj = SPH_EXTSTIFF * diff - SPH_EXTDAMP * this->vel(1);
      accel += adj * norm;
    }
  diff = 2.0 * SPH_RADIUS - ( MAX(1) - this->pos(1) ) * SPH_SIMSCALE;
  if ( diff > SPH_EPSILON )
    {
      norm << 0.0, -1.0, 0.0;
      adj = SPH_EXTSTIFF * diff + SPH_EXTDAMP * this->vel(1);
      accel += adj * norm;
    }
      
  accel += space.gravity;
  this->vel += accel * DT;
  this->pos += this->vel * DT / SPH_SIMSCALE;
}

