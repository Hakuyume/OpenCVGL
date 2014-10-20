#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
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
  glTranslated(0, 50, -200);
  glScaled(10, 10, 5);
  draw_particles(space.particles);
  glPopMatrix();

  glBindTexture(GL_TEXTURE_2D, 0);
}

int readShaderSource(GLuint shader, const char *file)
{
  FILE *fp;
  const GLchar *source;
  GLsizei length;
  int ret;

  fp = fopen(file, "rb");
  if (fp == NULL) {
    perror(file);
    return -1;
  }

  /* ファイルの末尾に移動し現在位置（つまりファイルサイズ）を得る */
  fseek(fp, 0L, SEEK_END);
  length = ftell(fp);

  source = (GLchar *)malloc(length);

  fseek(fp, 0L, SEEK_SET);
  ret = fread((void *)source, 1, length, fp) != (size_t)length;
  fclose(fp);

  glShaderSource(shader, 1, &source, &length);

  free((void *)source);

  return ret;
}

GLuint load_shader(const char *vert, const char *frag)
{
  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

  if (readShaderSource(vert_shader, vert))
    exit(1);
  if (readShaderSource(frag_shader, frag))
    exit(1);

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

  cv::Mat image = cv::imread("bottom.jpg", 1);
  cv::cvtColor(image, image, CV_BGR2RGB);

  glBindTexture(GL_TEXTURE_2D, backtex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

  cv::flip(image, image, 1);

  glBindTexture(GL_TEXTURE_CUBE_MAP, cubetex);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

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
