// reflect.frag

uniform samplerCube cubemap;

varying vec3 r;  // 視線の反射ベクトル

void main(void)
{
  gl_FragColor = textureCube(cubemap, r);
}
