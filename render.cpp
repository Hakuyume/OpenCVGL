#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include "render.hpp"
#include "mc.hpp"

static GLuint shader;
static GLint cubemap;

static GLuint backtex, cubetex;

static const int texture_cubes[] = {
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y};

void renderer_draw(const Space &space)
{
  glBindTexture(GL_TEXTURE_2D, backtex);

  glPushMatrix();
  glScaled(500, 500, 500);
  glTranslated(0, 0, -0.5);

  glBegin(GL_QUADS);
  glNormal3d(0, 0, -1);
  glTexCoord2d(0, 1);
  glVertex3d(-0.5, -0.5, 0);
  glTexCoord2d(0, 0);
  glVertex3d(-0.5, +0.5, 0);
  glTexCoord2d(1, 0);
  glVertex3d(+0.5, +0.5, 0);
  glTexCoord2d(1, 1);
  glVertex3d(+0.5, -0.5, 0);
  glEnd();

  glPopMatrix();

  glBindTexture(GL_TEXTURE_CUBE_MAP, cubetex);
  glUseProgram(shader);
  glUniform1i(cubemap, 0);

  glPushMatrix();
  glTranslated(0, 0, -200);
  glScaled(5, 5, 10);
  draw_particles(space.particles);
  glPopMatrix();

  glBindTexture(GL_TEXTURE_2D, 0);
}

void read_source(const GLuint shader, const char *file)
{
  std::ifstream ifs(file);

  std::istreambuf_iterator<char> begin(ifs);
  std::istreambuf_iterator<char> end;
  std::string content(begin, end);

  const char *c_content = content.c_str();
  GLsizei length = content.length();

  glShaderSource(shader, 1, &c_content, &length);
}

GLuint load_shader(const char *vert, const char *frag)
{
  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

  read_source(vert_shader, vert);
  read_source(frag_shader, frag);

  glCompileShader(vert_shader);
  glCompileShader(frag_shader);

  GLuint program = glCreateProgram();

  glAttachShader(program, vert_shader);
  glAttachShader(program, frag_shader);

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  glLinkProgram(program);

  return program;
}

void renderer_init(void)
{
  glGenTextures(1, &backtex);
  glGenTextures(1, &cubetex);

  cv::Mat image = cv::Mat::zeros(500, 500, CV_8UC3);

  for (int x = 0; x < 50; x++)
    for (int y = 0; y < 50; y++)
      if ((x + y) % 2 == 0)
        cv::rectangle(image, cv::Point(x * 10, y * 10), cv::Point((x + 1) * 10, (y + 1) * 10), cv::Scalar(100, 100, 100), -1, CV_AA);
      else
        cv::rectangle(image, cv::Point(x * 10, y * 10), cv::Point((x + 1) * 10, (y + 1) * 10), cv::Scalar(255, 255, 255), -1, CV_AA);

  cv::cvtColor(image, image, CV_BGR2RGB);

  glBindTexture(GL_TEXTURE_2D, backtex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

  cv::flip(image, image, 1);

  glBindTexture(GL_TEXTURE_CUBE_MAP, cubetex);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

  image = cv::Scalar::all(0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubetex);
  for (int i = 0; i < 6; i++)
    if (texture_cubes[i] != GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
      glTexImage2D(texture_cubes[i], 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

  glBindTexture(GL_TEXTURE_2D, backtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, cubetex);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  shader = load_shader("shaders/refract.vert", "shaders/refract.frag");

  cubemap = glGetUniformLocation(shader, "cubemap");

  glClearColor(1, 1, 1, 0);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}
