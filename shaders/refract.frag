uniform samplerCube cubemap;

varying vec3 r;
varying vec3 s;
varying float t;

void main(void)
{
  gl_FragColor = mix(textureCube(cubemap, s), textureCube(cubemap, r), t);
}
