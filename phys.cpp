#include "phys.hpp"

#include <iostream>
#include <fstream>
#include <cmath>

static const double SPH_RESTDENSITY = 600.0;
static const double SPH_INTSTIFF    = 3.0;
static const double SPH_PMASS       = 0.00020543;
static const double SPH_SIMSCALE    = 0.004;
static const double H               = 0.01;
static const double PI              = 3.141592653589793;
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
static const double Poly6Kern       = 315.0 / ( 64.0 * PI * pow( H, 9 ) );
static const double SpikyKern       = -45.0 / ( PI * pow( H, 6 ) );
static const double LapKern         = 45.0 / ( PI * pow( H, 6 ) );

void Space::put_particles(void)
{
  double d = SPH_PDIST / SPH_SIMSCALE * 0.95;
  for ( double x = INIT_MIN(0)+d; x <= INIT_MAX(0)-d; x += d )
    for ( double y = INIT_MIN(1)+d; y <= INIT_MAX(1)-d; y += d )
      for ( double z = INIT_MIN(2)+d; z <= INIT_MAX(2)-d; z += d ){
	Particle p;
	p.pos << x, y, z;
	p.vel = Eigen::Vector3d::Zero();
	p.f = Eigen::Vector3d::Zero();
	p.rho  = 0.0;
	p.prs  = 0.0;
	this->particles.push_back( p );
      }
}

#define FOR_EACH_PARTICLE( p_p, p_ps ) \
  for( Particles::iterator (p_p) = (p_ps)->begin(); (p_p) != (p_ps)->end(); (p_p)++ )

NeighborMap*   new_neighbor_map( Particles* p_ps );
void           _insert_neighbor_map( Particle*, NeighborMap* );
void           delete_neighbor_map( NeighborMap* );
ParticlePtrs   neighbor( NeighborMap*, Eigen::Vector3d );
NeighborMapIdx neighbor_map_idx( Eigen::Vector3d );

#define FOR_EACH_PARTICLE_PTR( p_ptr, p_ptrs ) \
  for( ParticlePtrs::iterator p_ptr = (p_ptrs)->begin(); (p_ptr) != (p_ptrs)->end(); (p_ptr)++ )

NeighborMap* new_neighbor_map( Particles* p_ps )
{
  NeighborMap* p_nbr_map;
  p_nbr_map = new NeighborMap();
  FOR_EACH_PARTICLE( p_p, p_ps )
    _insert_neighbor_map( &*p_p, p_nbr_map );
  return p_nbr_map;
}

void _insert_neighbor_map( Particle* p_p, NeighborMap* p_nbr_map )
{
  NeighborMapIdx ix = neighbor_map_idx( p_p->pos );
  NeighborMap::iterator iter = p_nbr_map->find( ix );
  if ( iter != p_nbr_map->end() )
    {
      iter->second.push_back( p_p );
    }
  else
    {
      ParticlePtrs ptrs;
      ptrs.push_back( p_p );
      p_nbr_map->insert( NeighborMap::value_type( ix, ptrs ) );
    }
}

void delete_neighbor_map( NeighborMap* p_nbr_map )
{
  delete p_nbr_map;
}

ParticlePtrs neighbor( NeighborMap* p_nbr_map, Eigen::Vector3d r )
{
  ParticlePtrs ptrs;
  double d = H / SPH_SIMSCALE;
  for ( int x=-1; x<2; x++ )
  for ( int y=-1; y<2; y++ )
  for ( int z=-1; z<2; z++ )
    {
      Eigen::Vector3d v(x, y, z);
      v = r + v * d;
	if ( MIN(0) <= v(0) && v(0) <= MAX(0) &&
	     MIN(1) <= v(1) && v(1) <= MAX(1) &&
	     MIN(2) <= v(2) && v(2) <= MAX(2) )
        {
          NeighborMapIdx ix = neighbor_map_idx( v );
          NeighborMap::iterator x = p_nbr_map->find(ix);
          if ( x != p_nbr_map->end() )
            {
              FOR_EACH_PARTICLE_PTR( p_ptr, &(x->second) )
                {
                  ptrs.push_back( *p_ptr );
                }
            }
        }
    }
  return ptrs;
}

NeighborMapIdx neighbor_map_idx( Eigen::Vector3d r )
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
  NeighborMap* p_nbr_map = new_neighbor_map(&(this->particles));
  calc_amount(p_nbr_map);
  calc_force(p_nbr_map);
  advance(p_nbr_map);
  delete_neighbor_map(p_nbr_map);
}

void Space::calc_amount(NeighborMap* p_nbr_map)
{
  double H2, sum, r2, c;
  Eigen::Vector3d dr;
  ParticlePtrs ptrs;
  Particle* p_pj;
  
  H2 = H*H;
  
  FOR_EACH_PARTICLE( p_p, &(this->particles) )
    {
      sum  = 0.0;
      ptrs = neighbor( p_nbr_map, p_p->pos );
      FOR_EACH_PARTICLE_PTR( p_ptr, &ptrs )
        {
          p_pj= *p_ptr;
          dr = (p_p->pos - p_pj->pos) * SPH_SIMSCALE;
          r2 = dr.norm() * dr.norm();
          if ( H2 > r2 )
            {
              c = H2 - r2;
              sum += c * c * c;
            }
        }
      p_p->rho = sum * SPH_PMASS * Poly6Kern;
      p_p->prs = ( p_p->rho - SPH_RESTDENSITY ) * SPH_INTSTIFF;
      p_p->rho = 1.0 / p_p->rho;
    }
}

void Space::calc_force(NeighborMap* p_nbr_map)
{
  double pterm, vterm, r, c;
  Eigen::Vector3d dr, force, fcurr;
  ParticlePtrs ptrs;
  Particle* p_pj;
  
  FOR_EACH_PARTICLE( p_p, &(this->particles) )
    {
      force << 0.0, 0.0, 0.0;
      ptrs = neighbor( p_nbr_map, p_p->pos );
      FOR_EACH_PARTICLE_PTR( p_ptr, &ptrs )
        {
          p_pj = *p_ptr;
          if ( p_p->pos == p_pj->pos ) continue;
          dr = (p_p->pos - p_pj->pos) * SPH_SIMSCALE;
          r  = dr.norm();
          if ( H > r )
            {
              c = H - r;
              pterm = -0.5 * c * SpikyKern * (p_p->prs + p_pj->prs) / r;
              vterm = LapKern * SPH_VISC;
              fcurr = pterm * dr + vterm * (p_pj->vel - p_p->vel);
              fcurr *= c * p_p->rho * p_pj->rho;
              force += fcurr;
            }
        }
      p_p->f = force;
    }
}

void Space::advance(NeighborMap* p_nbr_map)
{
  Eigen::Vector3d accel, norm;
  double speed, diff, adj;

  FOR_EACH_PARTICLE( p_p, &(this->particles) )
    {
      accel = p_p->f * SPH_PMASS;
      
      speed = accel.norm() * accel.norm();
      if ( speed > SPH_LIMIT*SPH_LIMIT ) {
        accel *= SPH_LIMIT / sqrt(speed);
      }
      
      // Z-axis walls
      diff = 2.0 * SPH_RADIUS - ( p_p->pos(2) - MIN(2) ) * SPH_SIMSCALE;
      if ( diff > SPH_EPSILON )
        {
          norm << 0.0, 0.0, 1.0;
          adj = SPH_EXTSTIFF * diff - SPH_EXTDAMP * p_p->vel(2);
          accel += adj * norm;
        }
      diff = 2.0 * SPH_RADIUS - ( MAX(2) - p_p->pos(2) ) * SPH_SIMSCALE;
      if ( diff > SPH_EPSILON )
        {
          norm << 0.0, 0.0, -1.0;
          adj = SPH_EXTSTIFF * diff + SPH_EXTDAMP * p_p->vel(2);
          accel += adj * norm;
        }
     
      // X-axis walls
      diff = 2.0 * SPH_RADIUS - ( p_p->pos(0) - MIN(0) ) * SPH_SIMSCALE;
      if ( diff > SPH_EPSILON )
        {
          norm << 1.0, 0.0, 0.0;
          adj = SPH_EXTSTIFF * diff - SPH_EXTDAMP * p_p->vel(0);
          accel += adj * norm;
        }
      diff = 2.0 * SPH_RADIUS - ( MAX(0) - p_p->pos(0) ) * SPH_SIMSCALE;
      if ( diff > SPH_EPSILON )
        {
          norm << -1.0, 0.0, 0.0;
          adj = SPH_EXTSTIFF * diff + SPH_EXTDAMP * p_p->vel(0);
          accel += adj * norm;
        }

      // Y-axis walls
      diff = 2.0 * SPH_RADIUS - ( p_p->pos(1) - MIN(1) ) * SPH_SIMSCALE;
      if ( diff > SPH_EPSILON )
        {
          norm << 0.0, 1.0, 0.0;
          adj = SPH_EXTSTIFF * diff - SPH_EXTDAMP * p_p->vel(1);
          accel += adj * norm;
        }
      diff = 2.0 * SPH_RADIUS - ( MAX(1) - p_p->pos(1) ) * SPH_SIMSCALE;
      if ( diff > SPH_EPSILON )
        {
          norm << 0.0, -1.0, 0.0;
          adj = SPH_EXTSTIFF * diff + SPH_EXTDAMP * p_p->vel(1);
          accel += adj * norm;
        }
      
      accel += this->gravity;
      p_p->vel += accel * DT;
      p_p->pos += p_p->vel * DT / SPH_SIMSCALE;
    }
}

