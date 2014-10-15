#include <GL/glut.h>
#include "render.hpp"

#define MC_SIZE 2

class Cube
{
public:
  bool m[2][2][2];
  Cube(void);
  void draw(void);
};

Cube::Cube(void)
{
  for (int x = 0; x <= 1; x++)
    for (int y = 0; y <= 1; y++)
      for (int z = 0; z <= 1; z++)
	m[x][y][z] = false;
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

void Cube::draw(void)
{
  glTranslated(-0.5, -0.5, -0.5);

  for (int x = 0; x <= 1; x++)
    for (int y = 0; y <= 1; y++)
      for (int z = 0; z <= 1; z++)
	if (m[x][y][z] & ~m[x][y][1 - z] & ~m[x][1 - y][z] & ~m[1 - x][y][z]){
	  glBegin(GL_TRIANGLES);
	  glNormal3d(1 - x, 1 - y, 1 - z);
	  glVertex3d(x, y, 0.5);
	  glVertex3d(x, 0.5, z);
	  glVertex3d(0.5, y, z);
	  glEnd();
	}

  for (int i = 0; i <= 1; i++)
    for (int j = 0; j <= 1; j++)
      if (
	  m[i][j][0] & ~m[i][1 - j][0] & ~m[1 - i][j][0] &
	  m[i][j][1] & ~m[i][1 - j][1] & ~m[1 - i][j][1]
	  ){
	glBegin(GL_QUADS);
	glNormal3d(1 - i, 1 - j, 0);
	glVertex3d(i, 0.5, 0);
	glVertex3d(0.5, j, 0);
	glVertex3d(0.5, j, 1);
	glVertex3d(i, 0.5, 1);
	glEnd();
      }
  for (int i = 0; i <= 1; i++)
    for (int j = 0; j <= 1; j++)
      if (
	  m[0][i][j] & ~m[0][i][1 - j] & ~m[0][1 - i][j] &
	  m[1][i][j] & ~m[1][i][1 - j] & ~m[1][1 - i][j]
	  ){
	glBegin(GL_QUADS);
	glNormal3d(0, 1 - i, 1 - j);
	glVertex3d(0, i, 0.5);
	glVertex3d(0, 0.5, j);
	glVertex3d(1, 0.5, j);
	glVertex3d(1, i, 0.5);
	glEnd();
      }
  for (int i = 0; i <= 1; i++)
    for (int j = 0; j <= 1; j++)
      if (
	  m[j][0][i] & ~m[1 - j][0][i] & ~m[j][0][1 - i] &
	  m[j][1][i] & ~m[1 - j][1][i] & ~m[j][1][1 - i]
	  ){
	glBegin(GL_QUADS);
	glNormal3d(1 - j, 0, 1 - i);
	glVertex3d(0.5, 0, i);
	glVertex3d(j, 0, 0.5);
	glVertex3d(j, 1, 0.5);
	glVertex3d(0.5, 1, i);
	glEnd();
      }

  for (int i = 0; i <= 1; i++)
    if (
	m[i][0][0] & ~m[1 - i][0][0] &
	m[i][0][1] & ~m[1 - i][0][1] &
	m[i][1][1] & ~m[1 - i][1][1] &
	m[i][1][0] & ~m[1 - i][1][0]
	){
      glBegin(GL_QUADS);
      glNormal3d(1 - i, 0, 0);
      glVertex3d(0.5, 0, 0);
      glVertex3d(0.5, 0, 1);
      glVertex3d(0.5, 1, 1);
      glVertex3d(0.5, 1, 0);
      glEnd();
    }
  for (int i = 0; i <= 1; i++)
    if (
	m[0][i][0] & ~m[1][1 - i][0] &
	m[1][i][0] & ~m[1][1 - i][0] &
	m[1][i][1] & ~m[1][1 - i][1] &
	m[0][i][1] & ~m[0][1 - i][1]
	){
      glBegin(GL_QUADS);
      glNormal3d(0, 1 - i, 0);
      glVertex3d(0, 0.5, 0);
      glVertex3d(1, 0.5, 0);
      glVertex3d(1, 0.5, 1);
      glVertex3d(0, 0.5, 1);
      glEnd();
    }
  for (int i = 0; i <= 1; i++)
    if (
	m[0][0][i] & ~m[0][0][1 - i] &
	m[0][1][i] & ~m[0][1][1 - i] &
	m[1][1][i] & ~m[1][1][1 - i] &
	m[1][0][i] & ~m[1][0][1 - i]
	){
      glBegin(GL_QUADS);
      glNormal3d(0, 0, 1 - i);
      glVertex3d(0, 0, 0.5);
      glVertex3d(0, 1, 0.5);
      glVertex3d(1, 1, 0.5);
      glVertex3d(1, 0, 0.5);
      glEnd();
    }
}
