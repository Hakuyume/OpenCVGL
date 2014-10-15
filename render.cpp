#include <GL/glut.h>
#include <set>
#include "render.hpp"

#define MC_SIZE 0.5

struct CompVector
{
  bool operator()(const Eigen::Vector3i& a, const Eigen::Vector3i& b);
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

void render_particles(const Space& space)
{
  std::set<Eigen::Vector3i, CompVector> points;

  for (auto& pt : space.particles)
    for (int x = 0; x <= 1; x++)
      for (int y = 0; y <= 1; y++)
	for (int z = 0; z <= 1; z++){
	  Eigen::Vector3i p;
	  p(0) = floor(pt.pos(0) / MC_SIZE) + x;
	  p(1) = floor(pt.pos(1) / MC_SIZE) + y;
	  p(2) = floor(pt.pos(2) / MC_SIZE) + z;

	  points.insert(p);
	}

  GLfloat facecolor[] = {0, 0, 1, 0.7};
  glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor);

  for (auto& p : points){
    glPushMatrix();
    glTranslated(p(0) * MC_SIZE, p(1) * MC_SIZE, p(2) * MC_SIZE);
    glutSolidCube(MC_SIZE);
    glPopMatrix();
  }
}
