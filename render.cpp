#include <GL/glut.h>
#include "render.hpp"

#define MC_SIZE 0.5

struct CompVector
{
  bool operator()(const Eigen::Vector3i& a, const Eigen::Vector3i& b);
};

class Cube
{
public:
  bool m[2][2][2];
  Cube(void);
  void draw(void);
};

bool CompVector::operator()(const Eigen::Vector3i& a, const Eigen::Vector3i& b)
{
  for (int i = 0; i < 3; i++)
    if (a(i) < b(i))
      return true;
    else if (a(i) > b(i))
      return false;
  return false;
}

Cube::Cube(void)
{
  for (int x = 0; x <= 1; x++)
    for (int y = 0; y <= 1; y++)
      for (int z = 0; z <= 1; z++)
	m[x][y][z] = false;
}

void Cube::draw(void)
{
  glutSolidCube(MC_SIZE);
}

void render_particles(const Space& space)
{
  std::map<Eigen::Vector3i, Cube, CompVector> cubes;

  for (auto& pt : space.particles)
    for (int x = 0; x <= 1; x++)
      for (int y = 0; y <= 1; y++)
	for (int z = 0; z <= 1; z++){
	  Eigen::Vector3i p;
	  p(0) = floor(pt.pos(0) / MC_SIZE) + x;
	  p(1) = floor(pt.pos(1) / MC_SIZE) + y;
	  p(2) = floor(pt.pos(2) / MC_SIZE) + z;

	  auto iter = cubes.find(p);
	  if (iter == cubes.end()){
	    Cube c;
	    iter = cubes.insert(iter, std::map<Eigen::Vector3i, Cube, CompVector>::value_type(p, c));
	  }
	  iter->second.m[1 - x][1 - y][1 - z] = true;
	}

  GLfloat facecolor[] = {0, 0, 1, 0.7};
  glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor);

  for (auto& iter : cubes){
    glPushMatrix();
    glTranslated(
		 iter.first(0) * MC_SIZE,
		 iter.first(1) * MC_SIZE,
		 iter.first(2) * MC_SIZE
		 );
    iter.second.draw();
    glPopMatrix();
  }
}
