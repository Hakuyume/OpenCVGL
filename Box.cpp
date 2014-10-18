#if defined(WIN32)
#  include "glut.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

#include "Box.h"

/* 箱の基本サイズ */
static const float original[][4][3] = {
  { // 下
    { -0.5f, -0.5f, -0.5f },
    {  0.5f, -0.5f, -0.5f },
    {  0.5f, -0.5f,  0.5f },
    { -0.5f, -0.5f,  0.5f }
  },
  { // 裏
    {  0.5f, -0.5f, -0.5f },
    { -0.5f, -0.5f, -0.5f },
    { -0.5f,  0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f }
  },
  { // 右
    {  0.5f, -0.5f,  0.5f },
    {  0.5f, -0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f },
    {  0.5f,  0.5f,  0.5f }
  },
  {  // 前
    { -0.5f, -0.5f,  0.5f },
    {  0.5f, -0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f },
    { -0.5f,  0.5f,  0.5f }
  },
  { // 左
    { -0.5f, -0.5f, -0.5f },
    { -0.5f, -0.5f,  0.5f },
    { -0.5f,  0.5f,  0.5f },
    { -0.5f,  0.5f, -0.5f }
  },
  { // 上
    { -0.5f,  0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f },
    {  0.5f,  0.5f, -0.5f },
    { -0.5f,  0.5f, -0.5f }
  },
};

/* 面の法線ベクトル */
static const float normal[][4][3] = {
  { // 下
    {  0.0f, -1.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f }
  },
  { // 裏
    {  0.0f,  0.0f, -1.0f },
    {  0.0f,  0.0f, -1.0f },
    {  0.0f,  0.0f, -1.0f },
    {  0.0f,  0.0f, -1.0f }
  },
  { // 右
    {  1.0f,  0.0f,  0.0f },
    {  1.0f,  0.0f,  0.0f },
    {  1.0f,  0.0f,  0.0f },
    {  1.0f,  0.0f,  0.0f }
  },
  { // 前
    {  0.0f,  0.0f,  1.0f },
    {  0.0f,  0.0f,  1.0f },
    {  0.0f,  0.0f,  1.0f },
    {  0.0f,  0.0f,  1.0f }
  },
  { // 左
    { -1.0f,  0.0f,  0.0f },
    { -1.0f,  0.0f,  0.0f },
    { -1.0f,  0.0f,  0.0f },
    { -1.0f,  0.0f,  0.0f }
  },
  { // 上
    {  0.0f,  1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f }
  },
};

/* 頂点のテクスチャ座標 */
static const float texcoord[][4][2] = {
  { // 下
    { 0.0f,   1.0f },
    { 0.125f, 1.0f },
    { 0.125f, 0.0f },
    { 0.0f,   0.0f }
  },
  { // 裏
    { 0.125f, 1.0f },
    { 0.25f,  1.0f },
    { 0.25f,  0.0f },
    { 0.125f, 0.0f }
  },
  { // 右
    { 0.25f,  1.0f },
    { 0.375f, 1.0f },
    { 0.375f, 0.0f },
    { 0.25f,  0.0f }
  },
  { // 前
    { 0.375f, 1.0f },
    { 0.5f,   1.0f },
    { 0.5f,   0.0f },
    { 0.375f, 0.0f }
  },
  { // 左
    { 0.5f,   1.0f },
    { 0.625f, 1.0f },
    { 0.625f, 0.0f },
    { 0.5f,   0.0f }
  },
  { // 上
    { 0.625f, 1.0f },
    { 0.75f,  1.0f },
    { 0.75f,  0.0f },
    { 0.625f, 0.0f }
  },
};

/*
** 箱のコンストラクタ
*/
Box::Box(float x, float y, float z)
{
  size(x, y, z);
}

/*
** 箱のサイズ設定
*/
void Box::size(float x, float y, float z)
{
  for (int j = 0; j < 6; ++j) {
    for (int i = 0; i < 4; ++i) {
      vertex[j][i][0] = original[j][i][0] * x;
      vertex[j][i][1] = original[j][i][1] * y;
      vertex[j][i][2] = original[j][i][2] * z;
    }
  }
}

/*
** 箱の描画
*/
void Box::draw(void)
{
  /* 頂点データ，法線データ，テクスチャ座標の配列を有効にする */
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  /* 頂点データ，法線データ，テクスチャ座標の場所を指定する */
  glVertexPointer(3, GL_FLOAT, 0, vertex);
  glNormalPointer(GL_FLOAT, 0, normal);
  glTexCoordPointer(2, GL_FLOAT, 0, texcoord);

  /* 図形を描画する */
  glDrawArrays(GL_QUADS, 0, sizeof(vertex) / sizeof(vertex[0][0]));

  /* 頂点データ，法線データ，テクスチャ座標の配列を無効にする */
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
