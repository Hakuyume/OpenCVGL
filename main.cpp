#include <GL/glut.h>
#include <cmath>
#include "phys.hpp"
#include "render.hpp"

#define GRAVITY 9.8

bool left_button = false;
int mouse_x0, mouse_y0;
double width = 600;
double height = 600;

Space space;

Eigen::Vector3d conv_pos(int x, int y)
{
  return Eigen::Vector3d(
      (((double)x * 2 / width) - 1) * space.size(0) * (double)width / height,
      -(((double)y * 2 / height) - 1) * space.size(1),
      0);
}

void display(void)
{
  renderer_draw(width, height, space);
}

void reshape(int w, int h)
{
  width = w;
  height = h;
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'a':
    space.add_particle(conv_pos(x, y));
    break;
  case 'd':
    space.remove_particle(conv_pos(x, y));
    break;
  case 'q':
    space.stop_simulate();
    exit(0);
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
    space.gravity << (double)(x - mouse_x0) / width * GRAVITY, -(1 + (double)(y - mouse_y0) / height) * GRAVITY, 0;
}

void idle(void)
{
  glutPostRedisplay();
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(width, height);
  glutCreateWindow("main");

  glutDisplayFunc(&display);
  glutReshapeFunc(&reshape);
  glutKeyboardFunc(&keyboard);
  glutMouseFunc(&mouse);
  glutMotionFunc(&motion);
  glutPassiveMotionFunc(&motion);
  glutIdleFunc(&idle);

  renderer_init();

  space.put_particles(100);
  space.size << 30, 30, 20;
  space.gravity << 0, -GRAVITY, 0;

  space.start_simulate(4);

  glutMainLoop();

  return 0;
}
