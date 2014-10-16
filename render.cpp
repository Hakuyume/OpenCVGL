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

  GLfloat facecolor[] = {0.2, 0.2, 1, 1};
  glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor);

  glPushMatrix();
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
  glPopMatrix();
}

struct Vertex
{
  Eigen::Vector3d pos, norm;
};

void Cube::draw(void)
{
  glutWireCube(1);

  glTranslated(-0.5, -0.5, -0.5);

  std::vector<Vertex> vs;

  for (int i = 0; i <= 1; i++)
    for (int j = 0; j <= 1; j++){
      if (m[0][i][j] & ~m[1][i][j]){
	Vertex v;
	v.pos << 0.5, i, j;
	v.norm << 1, 0, 0;
	vs.push_back(v);
      }else if (~m[0][i][j] & m[1][i][j]){
	Vertex v;
	v.pos << 0.5, i, j;
	v.norm << -1, 0, 0;
	vs.push_back(v);
      }
      if (m[j][0][i] & ~m[j][1][i]){
	Vertex v;
	v.pos << j, 0.5, i;
	v.norm << 0, 1, 0;
	vs.push_back(v);
      }else if (~m[j][0][i] & m[j][1][i]){
	Vertex v;
	v.pos << j, 0.5, i;
	v.norm << 0, -1, 0;
	vs.push_back(v);
      }
      if (m[i][j][0] & ~m[i][j][1]){
	Vertex v;
	v.pos << i, j, 0.5;
	v.norm << 0, 0, 1;
	vs.push_back(v);
      }else if (~m[i][j][0] & m[i][j][1]){
	Vertex v;
	v.pos << i, j, 0.5;
	v.norm << 0, 0, -1;
	vs.push_back(v);
      }
    }

  glBegin(GL_TRIANGLES);
  for (auto v1 = vs.begin(); v1 != vs.end(); v1++)
    for (auto v2 = v1 + 1; v2 != vs.end(); v2++)
      for (auto v3 = v2 + 1; v3 != vs.end(); v3++){
	glNormal3d(v1->norm(0), v1->norm(1), v1->norm(2));
	glVertex3d(v1->pos(0), v1->pos(1), v1->pos(2));

	glNormal3d(v2->norm(0), v2->norm(1), v2->norm(2));
	glVertex3d(v2->pos(0), v2->pos(1), v2->pos(2));

	glNormal3d(v3->norm(0), v3->norm(1), v3->norm(2));
	glVertex3d(v3->pos(0), v3->pos(1), v3->pos(2));
      }
  glEnd();
}
