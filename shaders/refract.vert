varying vec3 r;
varying vec3 s;
varying float t;

const float eta = 0.67;
const float f = (1.0 - eta) * (1.0 - eta) / ((1.0 + eta) * (1.0 + eta));

void main(void)
{
  vec4 p = gl_ModelViewMatrix * gl_Vertex;
  vec3 v = normalize(p.xyz / p.w);
  vec3 n = gl_NormalMatrix * gl_Normal;
  r = vec3(gl_TextureMatrix[0] * vec4(reflect(v, n), 1.0));
  s = vec3(gl_TextureMatrix[0] * vec4(refract(v, n, eta), 1.0));
  t = f + (1.0 - f) * pow(1.0 - dot(-v, n), 5.0);
  gl_Position = ftransform();
}
