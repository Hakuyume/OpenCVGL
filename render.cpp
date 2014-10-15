#include <GL/glut.h>
#include "render.hpp"

void render_particles(const Space& space)
{
  GLfloat facecolor[] = {0, 0, 1, 0.7};
  glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor);

  for (auto& pt : space.particles){
    glPushMatrix();
    glTranslated(pt.pos(0), pt.pos(1), pt.pos(2));
    glutSolidSphere(0.5, 12, 12);
    glPopMatrix();
  }
}
