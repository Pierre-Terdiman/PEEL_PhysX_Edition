#version 130

varying vec3 varyPos;

void main()
{
	varyPos = gl_Normal;
	gl_Position = gl_Vertex;
}
