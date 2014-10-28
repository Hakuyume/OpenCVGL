uniform samplerCube cubemap;

varying vec3 r;
varying vec3 s;
varying vec3 c;
varying float t;

void main(void)
{
  gl_FragColor = mix(textureCube(cubemap, s), textureCube(cubemap, r), t);
  gl_FragColor[0] *= c[0];
  gl_FragColor[1] *= c[1];
  gl_FragColor[2] *= c[2];
}
