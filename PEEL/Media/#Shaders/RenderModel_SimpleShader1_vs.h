#version 130

//uniform float uvScale = 1.0;

//****varying vec3 varyPos;

void main()
{
	gl_FrontColor = gl_Color;
//	vec4 eyeSpacePos = gl_ModelViewMatrix * gl_Vertex;
//	gl_Position = gl_ProjectionMatrix*eyeSpacePos;
	gl_Position = ftransform();	// This does the same as the two lines above

//	gl_TexCoord[0] = gl_MultiTexCoord0*uvScale;
//	gl_TexCoord[1] = eyeSpacePos;
	gl_TexCoord[0] = gl_ModelViewMatrixInverseTranspose * vec4(gl_Normal.xyz, 0.0);
//	gl_TexCoord[0] = vec4(gl_Normal.xyz, 0.0);
//		gl_TexCoord[2] = normalize(gl_ModelViewMatrixInverseTranspose * vec4(gl_Normal.xyz, 0.0));
//	gl_ClipVertex = vec4(eyeSpacePos.xyz, 1.0);

//	varyPos = normalize(vec3(gl_ModelViewMatrix * gl_Vertex));
//****	varyPos = gl_ModelViewMatrix * gl_Vertex;

}
