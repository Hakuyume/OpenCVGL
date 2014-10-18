// reflect.vert

varying vec3 r;  // ������ȿ�ͥ٥��ȥ�

void main(void)
{
  vec4 p = gl_ModelViewMatrix * gl_Vertex;  // ĺ������
  vec3 v = p.xyz / p.w;                     // �����٥��ȥ�
  vec3 n = gl_NormalMatrix * gl_Normal;     // ˡ���٥��ȥ�
  r = vec3(gl_TextureMatrix[0] * vec4(reflect(v, n), 1.0));
  gl_Position = ftransform();
}
