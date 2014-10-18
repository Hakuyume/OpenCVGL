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
#include "glsl.h"

static GLuint shader0, shader1;
static GLint texture, cubemap;

static GLuint texname[2]; /* テクスチャ名（番号） */

static const int target[] = { // テクスチャのターゲット名
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y};

void renderer_draw(const Space &space)
{
  glBindTexture(GL_TEXTURE_2D, texname[0]);
  glUseProgram(shader0);
  glUniform1i(texture, 0);

  glPushMatrix();
  glScaled(450, 450, 500);
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

  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);
  glUseProgram(shader1);
  glUniform1i(cubemap, 0);

  glPushMatrix();
  glTranslated(0, 50, -200);
  glScaled(10, 10, 10);
  draw_particles(space.particles);
  glPopMatrix();

  glBindTexture(GL_TEXTURE_2D, 0);
}

/*
** シェーダプログラムの作成
*/
static GLuint loadShader(const char *vert, const char *frag)
{
  /* シェーダオブジェクトの作成 */
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

  /* シェーダのソースプログラムの読み込み */
  if (readShaderSource(vertShader, vert))
    exit(1);
  if (readShaderSource(fragShader, frag))
    exit(1);

  /* シェーダプログラムのコンパイル／リンク結果を得る変数 */
  GLint compiled, linked;

  /* バーテックスシェーダのソースプログラムのコンパイル */
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
  printShaderInfoLog(vertShader);
  if (compiled == GL_FALSE) {
    std::cerr << "Compile error in vertex shader." << std::endl;
    exit(1);
  }

  /* フラグメントシェーダのソースプログラムのコンパイル */
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
  printShaderInfoLog(fragShader);
  if (compiled == GL_FALSE) {
    std::cerr << "Compile error in fragment shader." << std::endl;
    exit(1);
  }

  /* プログラムオブジェクトの作成 */
  GLuint gl2Program = glCreateProgram();

  /* シェーダオブジェクトのシェーダプログラムへの登録 */
  glAttachShader(gl2Program, vertShader);
  glAttachShader(gl2Program, fragShader);

  /* シェーダオブジェクトの削除 */
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  /* シェーダプログラムのリンク */
  glLinkProgram(gl2Program);
  glGetProgramiv(gl2Program, GL_LINK_STATUS, &linked);
  printProgramInfoLog(gl2Program);
  if (linked == GL_FALSE) {
    std::cerr << "Link error" << std::endl;
    exit(1);
  }

  return gl2Program;
}

void renderer_init(void)
{
  glGenTextures(2, texname);

  cv::Mat image = cv::imread("bottom.jpg", 1);
  cv::cvtColor(image, image, CV_BGR2RGB);

  glBindTexture(GL_TEXTURE_2D, texname[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

  cv::flip(image, image, 1);

  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);
  for (int i = 0; i < 6; i++)
    if (target[i] != GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
      glTexImage2D(target[i], 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);

  glBindTexture(GL_TEXTURE_2D, texname[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  shader0 = loadShader("shaders/replace.vert", "shaders/replace.frag");
  shader1 = loadShader("shaders/refract.vert", "shaders/refract.frag");

  texture = glGetUniformLocation(shader0, "texture");
  cubemap = glGetUniformLocation(shader1, "cubemap");

  glClearColor(1, 1, 1, 0);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}
