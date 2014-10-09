#include <GL/glut.h>

bool left_button = false;
double r = 50;
double theta = 0;
double phi = 0;

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, 1.0, 0.1, 100);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(
	    0, 0, r,
	    0, 0, 0,
	    0, 1, 0
	    );

  glRotated(theta, 0, 1, 0);
  glRotated(phi, 1, 0, 0);

  glDisable(GL_DEPTH_TEST);

  glutSwapBuffers();
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
  static int x0, y0;

  if (left_button){
    theta += 0.5 * (x - x0);
    phi += 0.5 * (y - y0);
  }
  x0 = x;
  y0 = y;

  glutPostRedisplay();
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(600, 600);
  glutCreateWindow("main");

  glClearColor(0, 0, 0, 0);

  glutDisplayFunc(&display);
  glutKeyboardFunc(&keyboard);
  glutMouseFunc(&mouse);
  glutMotionFunc(&motion);
  glutPassiveMotionFunc(&motion);

  glutPostRedisplay();

  glutMainLoop();

  return 0;
}
