#include <GL/glut.h>
#include "mc.hpp"

#define MC_SIZE 0.75
#define MC_NEIGHBOR 3
#define MC_THRESHOLD 2

struct Vertex
{
  Eigen::Vector3d pos, norm;
};

class Cube
{
public:
  double q[2][2][2];
  Cube(void);
  void draw(void);
};

Cube::Cube(void)
{
  for (int x = 0; x <= 1; x++)
    for (int y = 0; y <= 1; y++)
      for (int z = 0; z <= 1; z++)
        q[x][y][z] = 0;
}

void Cube::draw(void)
{
  glTranslated(-0.5, -0.5, -0.5);

  std::vector<Vertex> vs;

  bool m[2][2][2];
  for (int x = 0; x <= 1; x++)
    for (int y = 0; y <= 1; y++)
      for (int z = 0; z <= 1; z++)
        m[x][y][z] = q[x][y][z] > MC_THRESHOLD;

  for (int i = 0; i <= 1; i++)
    for (int j = 0; j <= 1; j++) {
      if (m[0][i][j] & ~m[1][i][j]) {
        Vertex v;
        v.pos << 0.5, i, j;
        v.norm << 1, 0, 0;
        vs.push_back(v);
      } else if (~m[0][i][j] & m[1][i][j]) {
        Vertex v;
        v.pos << 0.5, i, j;
        v.norm << -1, 0, 0;
        vs.push_back(v);
      }
      if (m[j][0][i] & ~m[j][1][i]) {
        Vertex v;
        v.pos << j, 0.5, i;
        v.norm << 0, 1, 0;
        vs.push_back(v);
      } else if (~m[j][0][i] & m[j][1][i]) {
        Vertex v;
        v.pos << j, 0.5, i;
        v.norm << 0, -1, 0;
        vs.push_back(v);
      }
      if (m[i][j][0] & ~m[i][j][1]) {
        Vertex v;
        v.pos << i, j, 0.5;
        v.norm << 0, 0, 1;
        vs.push_back(v);
      } else if (~m[i][j][0] & m[i][j][1]) {
        Vertex v;
        v.pos << i, j, 0.5;
        v.norm << 0, 0, -1;
        vs.push_back(v);
      }
    }

  glBegin(GL_TRIANGLES);
  for (auto v1 = vs.begin(); v1 != vs.end(); v1++)
    for (auto v2 = v1 + 1; v2 != vs.end(); v2++)
      for (auto v3 = v2 + 1; v3 != vs.end(); v3++) {
        glNormal3d(v1->norm(0), v1->norm(1), v1->norm(2));
        glVertex3d(v1->pos(0), v1->pos(1), v1->pos(2));

        glNormal3d(v2->norm(0), v2->norm(1), v2->norm(2));
        glVertex3d(v2->pos(0), v2->pos(1), v2->pos(2));

        glNormal3d(v3->norm(0), v3->norm(1), v3->norm(2));
        glVertex3d(v3->pos(0), v3->pos(1), v3->pos(2));
      }
  glEnd();
}

void draw_particles(const std::vector<Particle> particles)
{
  std::map<Eigen::Vector3i, Cube, CompVector> cubes;

  for (auto &pt : particles)
    for (int x = -MC_NEIGHBOR; x <= MC_NEIGHBOR; x++)
      for (int y = -MC_NEIGHBOR; y <= MC_NEIGHBOR; y++)
        for (int z = -MC_NEIGHBOR; z <= MC_NEIGHBOR; z++) {
          Eigen::Vector3i p;
          p(0) = floor(pt.pos(0) / MC_SIZE + x);
          p(1) = floor(pt.pos(1) / MC_SIZE + y);
          p(2) = floor(pt.pos(2) / MC_SIZE + z);

          auto iter = cubes.find(p);
          if (iter == cubes.end()) {
            Cube c;
            iter = cubes.insert(iter, std::map<Eigen::Vector3i, Cube, CompVector>::value_type(p, c));
          }

          for (int dx = 0; dx <= 1; dx++)
            for (int dy = 0; dy <= 1; dy++)
              for (int dz = 0; dz <= 1; dz++) {
                Eigen::Vector3d r = pt.pos;
                r(0) -= (double)(p(0) + dx) * MC_SIZE;
                r(1) -= (double)(p(1) + dy) * MC_SIZE;
                r(2) -= (double)(p(2) + dz) * MC_SIZE;

                double c = MC_SIZE * MC_NEIGHBOR - r.norm();
                if (c > 0)
                  iter->second.q[dx][dy][dz] += c * c * c;
              }
        }

  glPushMatrix();
  glScaled(MC_SIZE, MC_SIZE, MC_SIZE);
  for (auto &iter : cubes) {
    glPushMatrix();
    glTranslated(iter.first(0), iter.first(1), iter.first(2));
    iter.second.draw();
    glPopMatrix();
  }
  glPopMatrix();
}
