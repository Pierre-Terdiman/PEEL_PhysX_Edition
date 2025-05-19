#version 130

uniform vec3 parallelLightDir;
//****varying vec3 varyPos;

void main()
{
//	gl_FragColor = gl_Color;
	vec3 normal = normalize(gl_TexCoord[0].xyz);
//		vec3 normal = gl_TexCoord[2].xyz;
//	if(dot(normal, gl_TexCoord[1].xyz) > 0)
//	{
//		normal.xyz *= -1;
//	}	
//	gl_FragColor = vec4(normal, gl_Color.w);
//	return;

//****	vec3 dx = dFdx(varyPos.xyz);
//****	vec3 dy = dFdy(varyPos.xyz);
//****	normal = normalize(cross(dx, dy));


	float diffuse = (0.3 + 0.7 * max(dot(normal, -parallelLightDir),0.0));
//	float diffuse = (0.7 * max(dot(normal, -parallelLightDir),0.0));
//	float diffuse = max(dot(normal, -parallelLightDir), 0.0);

	diffuse = clamp(diffuse, 0.0, 1.0);

	vec4 color = gl_Color * diffuse;

//		float diffuse2 = (0.7 * max(dot(normal, parallelLightDir),0.0));
//		diffuse2 = clamp(diffuse2, 0.0, 1.0);
//		vec4 color2 = vec4(1,0,0,0) * diffuse2;
//		color += color2;

	gl_FragColor = vec4(color.xyz, gl_Color.w);
}
