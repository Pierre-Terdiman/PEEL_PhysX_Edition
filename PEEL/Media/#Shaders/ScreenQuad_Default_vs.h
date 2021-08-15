
#ifdef _VERTEX_

varying vec3 varyPos;

void main()
{
	varyPos = gl_Normal;
	gl_Position = gl_Vertex;
}

#endif

