#include <GL/glut.h>
#include <cmath>
#include <thread>
#include "phys.hpp"
#include "render.hpp"

#define GRAVITY 9.8

bool left_button = false;
static int mouse_x0, mouse_y0;

Space space;

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  renderer_draw(space);

  glutSwapBuffers();
}

void reshape(int width, int height)
{
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, (double)width / height, 100, 500);
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  }
}

void mouse(int button, int state, int x, int y)
{
  switch (button) {
  case GLUT_LEFT_BUTTON:
    switch (state) {
    case GLUT_UP:
      left_button = false;
      space.gravity << 0, -GRAVITY, 0;
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
    space.gravity << x - mouse_x0, -((y - mouse_y0) + GRAVITY), 0;
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

  glutDisplayFunc(&display);
  glutReshapeFunc(&reshape);
  glutKeyboardFunc(&keyboard);
  glutMouseFunc(&mouse);
  glutMotionFunc(&motion);
  glutPassiveMotionFunc(&motion);
  glutIdleFunc(&idle);

  renderer_init();

  space.put_particles(600);

  space.gravity << 0, -GRAVITY, 0;

  bool simloop = true;
  std::thread sim([&simloop] {
    while (simloop) {
      space.update_particles(0.004);
    }
  });

  glutMainLoop();

  simloop = false;
  sim.join();

  return 0;
}
