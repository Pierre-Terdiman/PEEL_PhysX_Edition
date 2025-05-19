#version 130

varying vec3 varyPos;
//varying vec3 varyNormal;	// envmap
varying vec3 varyWorldPos;
varying float varyIntensity;

uniform float uvScale = 1.0;

uniform mat4 worldMatrix;

void main()
{
	gl_FrontColor = gl_Color;
	vec4 eyeSpacePos = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ProjectionMatrix*eyeSpacePos;
//	gl_Position = ftransform();

	gl_TexCoord[0] = gl_MultiTexCoord0*uvScale;
	gl_TexCoord[1] = eyeSpacePos;
//	gl_ClipVertex = vec4(eyeSpacePos.xyz, 1.0);

//	varyPos = gl_ModelViewMatrix * gl_Vertex;
//	varyPos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyPos = vec3(eyeSpacePos.xyz);

//	varyPos = normalize(vec3(gl_ModelViewMatrix * gl_Vertex));
//		varyNormal = normalize(gl_NormalMatrix * gl_Normal);	// envmap
		gl_TexCoord[2] = gl_ModelViewMatrixInverseTranspose * vec4(gl_Normal.xyz, 0.0);	// ***Lego envmap
		varyIntensity = length(gl_Normal.xyz);

	varyWorldPos = vec3(worldMatrix * vec4(gl_Vertex.xyz, 1.0));
	//varyWorldPos = gl_Vertex.xyz;
	//varyWorldPos = vec3(gl_Vertex);
}
