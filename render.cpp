#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include "render.hpp"
#include "mc.hpp"
#include "glsl.h"
#include "Box.h"

static GLuint shader0, shader1;
static GLint texture, cubemap;

static Box *box;

#define TEXWIDTH  128                           /* テクスチャの幅　　　 */
#define TEXHEIGHT 128                           /* テクスチャの高さ　　 */
static GLuint texname[2];                       /* テクスチャ名（番号） */
static const char *texfile[] = {                /* テクスチャファイル名 */
  "room2ny.raw", /* 下 */
  "room2nz.raw", /* 裏 */
  "room2px.raw", /* 右 */
  "room2pz.raw", /* 前 */
  "room2nx.raw", /* 左 */
  "room2py.raw", /* 上 */
};

static const int target[] = {                /* テクスチャのターゲット名 */
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
};


void renderer_draw(const Space& space)
{
  glBindTexture(GL_TEXTURE_2D, texname[0]);
  
  /* 箱のテクスチャのシェーダプログラムを適用する */
  glUseProgram(shader0);

  /* テクスチャユニット０を指定する */
  glUniform1i(texture, 0);

  /* 箱を描く */
  glPushMatrix();
  //  glMultMatrixd(tb2->rotation());
  box->draw();
  glPopMatrix();

  /* 設定対象をキューブマッピングのテクスチャに切り替える*/
  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);

  /* キューブマッピングのシェーダプログラムを適用する */
  glUseProgram(shader1);

  /* テクスチャユニット０を指定する */
  glUniform1i(cubemap, 0);

  /* テクスチャ変換行列にトラックボール式の回転を加える */
  glMatrixMode(GL_TEXTURE);
  //  glLoadTransposeMatrixd(tb2->rotation());
  glMatrixMode(GL_MODELVIEW);

  /* 視点より少し奥にオブジェクトを描いてトラックボール式の回転を加える */
  glPushMatrix();
  glTranslated(0.0, 0.0, -200.0);
  //  glMultMatrixd(tb1->rotation());
  glScaled(5, 5, 5);

  draw_particles(space.particles);

  glPopMatrix();
  
  /* テクスチャ変換行列を元に戻す */
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);

  /* 設定対象を無名テクスチャに戻す */
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
  if (readShaderSource(vertShader, vert)) exit(1);
  if (readShaderSource(fragShader, frag)) exit(1);
  
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

/*
** 初期化
*/
void renderer_init(void)
{
  /* 箱のオブジェクトを生成 */
  box = new Box(500.0f, 500.0f, 500.0f);

  /* テクスチャ名を２個生成 */
  glGenTextures(2, texname);
  
  /* テクスチャ画像はワード単位に詰め込まれている */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  /* 外側の立方体のテクスチャの割り当て（８枚分） */
  glBindTexture(GL_TEXTURE_2D, texname[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXWIDTH * 8, TEXHEIGHT, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, 0);
  
  /* テクスチャ画像の読み込み */
  for (int i = 0; i < 6; ++i) {
    std::ifstream file;

    file.open(texfile[i], std::ios::binary);
    if (file) {
      GLubyte image[TEXHEIGHT * TEXWIDTH * 4]; // テクスチャ画像の読み込み用

      file.read(reinterpret_cast<char *>(image), sizeof image);
      file.close();

      /* 外側の立方体のテクスチャの置き換え */
      glTexSubImage2D(GL_TEXTURE_2D, 0, TEXWIDTH * i, 0, TEXWIDTH, TEXHEIGHT,
        GL_RGBA, GL_UNSIGNED_BYTE, image);

      /* キューブマッピングのテクスチャの割り当て */
      glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);
      glTexImage2D(target[i], 0, GL_RGBA, TEXWIDTH, TEXHEIGHT, 0, 
        GL_RGBA, GL_UNSIGNED_BYTE, image);

      /* 設定対象を外側の立方体のテクスチャに戻す */
      glBindTexture(GL_TEXTURE_2D, texname[0]);
    }
  }
  
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  /* 設定対象をキューブマッピングのテクスチャに切り替える */
  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);

  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  /* 設定対象を無名テクスチャに戻す */
  glBindTexture(GL_TEXTURE_2D, 0);
  
  /* GLSL の初期化 */
  if (glslInit()) exit(1);

  /* シェーダプログラムの作成 */
  shader0 = loadShader("replace.vert", "replace.frag");
  shader1 = loadShader("reflect.vert", "reflect.frag");
  
  /* uniform 変数の位置を取り出す */
  texture = glGetUniformLocation(shader0, "texture");
  cubemap = glGetUniformLocation(shader1, "cubemap");

  /* 初期設定 */
  glClearColor(0.3, 0.3, 1.0, 0.0);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}
