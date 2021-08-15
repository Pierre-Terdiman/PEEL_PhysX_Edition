#version 130

uniform vec3 parallelLightDir;
varying vec3 varyPos;

//#define GREY_FOG
//#define FOG_TEST

void main()
{
#ifdef GREY_FOG
	float z = -varyPos.z/2000.0;
	gl_FragColor = vec4(vec3(z), 1.0);
	return;
#endif

	vec3 dx = dFdx(varyPos.xyz);
	vec3 dy = dFdy(varyPos.xyz);
	vec3 normal = normalize(cross(dx, dy));

	//gl_FragColor = vec4(normal, 1.0);
	//return;

	float brightness = dot(normal, -parallelLightDir);
	//float diffuse = (0.3 + 0.7 * max(brightness,0.0));
	//float diffuse = 0.5 + brightness*0.5;
//	float diffuse = max(brightness, 0.0);
	float diffuse = mix(max(brightness, 0.0), 0.5 + brightness*0.5, 0.2);
	diffuse = clamp(diffuse, 0.0, 1.0);

	// Fake IBL, would need normal in world space
	//diffuse = length(sin(normal*2.5)*0.5+0.5)/sqrt(3.0) * smoothstep(-1.0, 1.0, normal.z);

	//gl_FragColor = vec4(diffuse, diffuse, diffuse, 1.0);
	//gl_FragColor = vec4(parallelLightDir, 1.0);
	//return;

	vec4 color = gl_Color * diffuse;

//		float diffuse2 = (0.7 * max(dot(normal, parallelLightDir),0.0));
//		diffuse2 = clamp(diffuse2, 0.0, 1.0);
//		vec4 color2 = vec4(1,0,0,0) * diffuse2;
//		color += color2;

	gl_FragColor = vec4(color.xyz, gl_Color.w);

#ifdef FOG_TEST
	float z_fog_max = 2000.0;
	float z_fog_start = 100.0;
	float fogCoeff = clamp((-varyPos.z - z_fog_start)/(z_fog_max - z_fog_start), 0.0, 1.0);
	//gl_FragColor = vec4(vec3(fogCoeff), 1.0);
	//vec3 fogColor = vec3(1.0, 1.0, 1.0);
	vec3 fogColor = gl_Color.xyz;
	gl_FragColor.xyz = mix(fogColor, color.xyz, 1.0 - fogCoeff);
#endif
}
