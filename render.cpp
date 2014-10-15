#include <GL/glut.h>
#include "render.hpp"

#define MC_SIZE 2

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
  glTranslated(-0.5, -0.5, -0.5);

  for (int x = 0; x <= 1; x++)
    for (int y = 0; y <= 1; y++)
      for (int z = 0; z <= 1; z++){
	if (m[x][y][z] & ~m[x][y][1 - z] & ~m[x][1 - y][z] & ~m[1 - x][y][z]){
	  glBegin(GL_TRIANGLES);
	  glNormal3d(1 - x, 1 - y, 1 - z);
	  glVertex3d(x, y, 0.5);
	  glVertex3d(x, 0.5, z);
	  glVertex3d(0.5, y, z);
	  glEnd();
	}
      }
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

  glScaled(MC_SIZE, MC_SIZE, MC_SIZE);
  for (auto& iter : cubes){
    glPushMatrix();
    glTranslated(
		 iter.first(0),
		 iter.first(1),
		 iter.first(2)
		 );
    iter.second.draw();
    glPopMatrix();
  }
}
