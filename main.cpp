#include <GL/glut.h>
#include <math.h>
#include <vector>
#include <Eigen/Eigen>

bool left_button = false;
double r = 50;
double theta = 0;
double phi = 0;

std::vector<Eigen::Vector3d> objs;

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

  glRotated(theta, 0, 1, 0);
  glRotated(phi, 1, 0, 0);

  GLfloat facecolor[] = {0, 0, 1, 0.7};
  glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor);

  std::vector<Eigen::Vector3d>::iterator it;
  for (it = objs.begin(); it != objs.end(); it++){
    glPushMatrix();
    glTranslated((*it)(0), (*it)(1), (*it)(2));
    glutSolidSphere(1, 12, 12);
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

  for (int i = 0; i < 5000; i++){
    Eigen::Vector3d p;
    p << rand(), rand(), rand();
    objs.push_back(((p / RAND_MAX).array() - 0.5) * 20);
  }

  glutPostRedisplay();

  glutMainLoop();

  return 0;
}
