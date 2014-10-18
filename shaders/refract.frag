// refract.frag

uniform samplerCube cubemap;

varying vec3 r;  // ������ȿ�ͥ٥��ȥ�
varying vec3 s;  // �����ζ��ޥ٥��ȥ�
varying float t; // �����̤Ǥ�ȿ��Ψ

void main(void)
{
  gl_FragColor = mix(textureCube(cubemap, s), textureCube(cubemap, r), t);
}
