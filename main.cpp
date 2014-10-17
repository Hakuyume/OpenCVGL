#include <GL/glut.h>
#include <cmath>
#include <thread>
#include "phys.hpp"
#include "render.hpp"

#define GRAVITY 9.8

bool left_button = false;
static int mouse_x0, mouse_y0;
double r = 50;

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

  glPushMatrix();
  render_particles(space);
  glPopMatrix();

  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHTING);

  glutSwapBuffers();
}

void reshape(int width, int height)
{
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (double)width / height, 0.1, 100);

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
      space.gravity << 0, - GRAVITY, 0;
      break;
    case GLUT_DOWN:
      left_button = true;
      mouse_x0 = x;
      mouse_y0 = y;
      break;
    }
    break;
  }
}

void motion(int x, int y)
{
  if (left_button)
    space.gravity << x - mouse_x0, - ((y - mouse_y0) + GRAVITY), 0;

  glutPostRedisplay();
}

void idle(void)
{
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

  glClearColor(1, 1, 1, 0);

  glutDisplayFunc(&display);
  glutReshapeFunc(&reshape);
  glutKeyboardFunc(&keyboard);
  glutMouseFunc(&mouse);
  glutMotionFunc(&motion);
  glutPassiveMotionFunc(&motion);
  glutIdleFunc(&idle);

  space.put_particles();

  space.gravity << 0, -GRAVITY, 0;

  bool simloop = true;
  std::thread sim([&simloop]{
      while(simloop){
	space.update_particles(0.004);
      }
    });

  glutPostRedisplay();

  glutMainLoop();

  simloop = false;
  sim.join();

  return 0;
}
