#include <GL/glut.h>
#include <cmath>
#include "phys.hpp"

#define INTERVAL 0.004
#define GRAVITY 9.8

bool left_button = false;
double r = 50;
double theta = 0;

Space space;

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(
	    0, 0, r,
	    0, 0, 0,
	    0, 1, 0
	    );

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  GLfloat lightpos[] = {50, 50, 50, 0};
  GLfloat diffuse[] = {10, 10, 10, 1};
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

  glRotated(theta * 180 / M_PI, 0, 0, 1);

  GLfloat facecolor[] = {0, 0, 1, 0.7};
  glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor);

  glTranslated(-10, -10, 0);

  for (auto& pt : space.particles){
    glPushMatrix();
    glTranslated(pt.pos(0), pt.pos(1), pt.pos(2));
    glutSolidSphere(0.5, 12, 12);
    glPopMatrix();
  }

  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHTING);

  glutSwapBuffers();
}

void reshape(int width, int height)
{
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, (double)width / height, 0.1, 100);

  glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
  switch(key){
  }

  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  switch(button){
  case GLUT_LEFT_BUTTON:
    switch(state){
    case GLUT_UP:
      left_button = false;
      break;
    case GLUT_DOWN:
      left_button = true;
      break;
    }
    break;
  }
}

void motion(int x, int y)
{
  static int x0;

  if (left_button){
    theta += 0.01 * (x - x0);

    space.gravity << sin(theta), cos(theta), 0;
    space.gravity *= -GRAVITY;
  }
  x0 = x;

  glutPostRedisplay();
}

void timer(int value)
{
  glutTimerFunc(INTERVAL * 1000, &timer, 0);

  space.update_particles(INTERVAL);

  glutPostRedisplay();
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(600, 600);
  glutCreateWindow("main");

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  glClearColor(0, 0, 0, 0);

  glutDisplayFunc(&display);
  glutReshapeFunc(&reshape);
  glutKeyboardFunc(&keyboard);
  glutMouseFunc(&mouse);
  glutMotionFunc(&motion);
  glutPassiveMotionFunc(&motion);
  glutTimerFunc(0, &timer, 0);

  space.put_particles();

  space.gravity << 0, -GRAVITY, 0;

  glutPostRedisplay();

  glutMainLoop();

  return 0;
}
