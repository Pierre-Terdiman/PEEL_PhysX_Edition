#version 130

varying vec3 varyPos;

void main()
{
	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
//	gl_TexCoord[0] = gl_ModelViewMatrixInverseTranspose * vec4(gl_Normal.xyz,0.0);
	varyPos = vec3(gl_ModelViewMatrix * gl_Vertex);
}
