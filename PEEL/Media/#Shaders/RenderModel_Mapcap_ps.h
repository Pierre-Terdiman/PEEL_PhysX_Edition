#version 130

uniform vec3 parallelLightDir;
uniform sampler2D envTexture;
uniform float envMapping;
uniform int useExplicitVertexNormals = 0;

//varying vec3 varyNormal;
varying vec3 varyPos;

void main()
{
//	vec3 dx = dFdx(varyPos.xyz);
//	vec3 dy = dFdy(varyPos.xyz);
//	vec3 normal = normalize(cross(dx, dy));
//	vec3 normal = normalize(gl_TexCoord[0].xyz);

	vec3 normal;
	if(useExplicitVertexNormals > 0)
	{
		normal = normalize(gl_TexCoord[0].xyz);
	}
	else
	{
		vec3 dx = dFdx(varyPos.xyz);
		vec3 dy = dFdy(varyPos.xyz);
		normal = normalize(cross(dx, dy));
	}

	float diffuse = (0.3 + 0.7 * max(dot(normal, -parallelLightDir),0.0));

	diffuse = clamp(diffuse, 0.0, 1.0);

	vec4 color = gl_Color * diffuse;

	if (envMapping > 0)
	{
//		vec2 coord = vec2(varyNormal.x + 0.5, varyNormal.y + 0.5);
//		vec2 coord = vec2(normal.x + 0.5, normal.y + 0.5);

			vec3 r = reflect(varyPos, normal);
			float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z + 1.0)*(r.z + 1.0));
			vec2 coord = vec2(r.x / m + 0.5, r.y / m + 0.5);

		color = mix(texture2D(envTexture, coord), color, 1.0 - envMapping);
	}

	gl_FragColor = vec4(color.xyz, gl_Color.w);
//	gl_FragColor = vec4(normal.xyz, gl_Color.w);
//	gl_FragColor = vec4(varyNormal.xyz, gl_Color.w);

//		vec3 smooth_normal = normalize(gl_TexCoord[0].xyz);
//		gl_FragColor = vec4(smooth_normal.xyz, gl_Color.w);
}


