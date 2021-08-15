//#version 130 #extension GL_EXT_GPU_SHADER4 : enable
//#version 130 #extension GL_EXT_texture_array : enable
//#version 140 #extension GL_EXT_texture_array : enable
#version 130
#extension GL_EXT_texture_array : enable
#extension GL_ARB_texture_rectangle : enable

uniform vec3 camPos;
uniform vec3 sunDir;

// scene reflection 
uniform float reflectionCoeff = 0.0;
//uniform float specularCoeff = 0.0;

uniform sampler2DRect reflectionTex;
uniform sampler2D envTexture;
uniform sampler2D texture;

// Shadow map
uniform float shadowAmbient = 0.1;
uniform sampler2DArrayShadow stex;
uniform sampler2DArrayShadow stex2;
uniform sampler2DArrayShadow stex3;
uniform vec2 texSize; // x - size, y - 1/size
uniform vec4 far_d;

// Spot lights
uniform vec3 spotLightDir;
uniform vec3 spotLightPos;
uniform float spotLightCosineDecayBegin;
uniform float spotLightCosineDecayEnd;

uniform vec3 spotLightDir2;
uniform vec3 spotLightPos2;
uniform float spotLightCosineDecayBegin2;
uniform float spotLightCosineDecayEnd2;

uniform vec3 spotLightDir3;
uniform vec3 spotLightPos3;
uniform float spotLightCosineDecayBegin3;
uniform float spotLightCosineDecayEnd3;

uniform vec3 parallelLightDir;
uniform float shadowAdd;
uniform int useTexture;
uniform int useExplicitVertexNormals;
uniform float envMapping;
uniform int numShadows;

varying vec3 varyPos;
varying vec3 varyWorldPos;
varying float varyIntensity;

// for env mapping
//varying vec3 varyPos;
//varying vec3 varyNormal;	// envmap

float shadowCoeff1()
{
	const int index = 0;

	//int index = 3;
	//
	//if(gl_FragCoord.z < far_d.x)
	//	index = 0;
	//else if(gl_FragCoord.z < far_d.y)
	//	index = 1;
	//else if(gl_FragCoord.z < far_d.z)
	//	index = 2;
	
	vec4 shadow_coord = gl_TextureMatrix[index]*vec4(gl_TexCoord[1].xyz, 1);

if(shadow_coord.x<0.0 || shadow_coord.y<0.0)
	return 1.0;
if(shadow_coord.x>1.0 || shadow_coord.y>1.0)
	return 1.0;

	shadow_coord.w = shadow_coord.z + shadowAdd;
		// tell glsl in which layer to do the look up
	shadow_coord.z = 0.0;

	// Gaussian 3x3 filter
//	return shadow2DArray(stex, shadow_coord).x;


	//Bilinear weighted 4-tap filter 
//	vec2 pos = mod( shadow_coord.xy * 2048.0, 1.0); 
//	vec2 offset = (0.5 - step( 0.5, pos)) * 2048.0; 
//	float ret2 = shadow2DArray( stex, shadow_coord + vec4( offset, 0, 0)).x * (pos.x) * (pos.y); 
//	ret2 += shadow2DArray( stex, shadow_coord + vec4( offset.x, -offset.y, 0, 0)).x * (pos.x) * (1-pos.y); 
//	ret2 += shadow2DArray( stex, shadow_coord + vec4( -offset.x, offset.y, 0, 0)).x * (1-pos.x) * (pos.y); 
//	ret2 += shadow2DArray( stex, shadow_coord + vec4( -offset.x, -offset.y, 0, 0)).x * (1-pos.x) * (1-pos.y); 
//	return ret2;

		//Bilinear weighted 4-tap filter 
//		vec2 pos = mod( shadow_coord.xy * texSize.y, 1.0);
//		vec2 offset = (0.5 - step( 0.5, pos)) * texSize.y;
//		float ret2 = shadow2DArray( stex, shadow_coord + vec4( offset, 0, 0)).x * (pos.x) * (pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( offset.x, -offset.y, 0, 0)).x * (pos.x) * (1-pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( -offset.x, offset.y, 0, 0)).x * (1-pos.x) * (pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( -offset.x, -offset.y, 0, 0)).x * (1-pos.x) * (1-pos.y); 
//		return ret2;

		float X = texSize.y;
		float ret2 = shadow2DArray(stex, shadow_coord + vec4( -X, 0, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex, shadow_coord + vec4( 0, -X, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex, shadow_coord + vec4( 0, X, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex, shadow_coord + vec4( X, 0, 0, 0)).x * 0.25;
		return ret2;

	//float X = texSize.y;
	//const float X = 1.0;
	float ret = shadow2DArray(stex, shadow_coord).x * 0.25;
/*	ret += shadow2DArrayOffset(stex, shadow_coord, ivec2( -X, -X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex, shadow_coord, ivec2( -X, 0)).x * 0.125;
	ret += shadow2DArrayOffset(stex, shadow_coord, ivec2( -X, X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex, shadow_coord, ivec2( 0, -X)).x * 0.125;
	ret += shadow2DArrayOffset(stex, shadow_coord, ivec2( 0, X)).x * 0.125;
	ret += shadow2DArrayOffset(stex, shadow_coord, ivec2( X, -X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex, shadow_coord, ivec2( X, 0)).x * 0.125;
	ret += shadow2DArrayOffset(stex, shadow_coord, ivec2( X, X)).x * 0.0625;*/
	ret += shadow2DArray(stex, shadow_coord + vec4( -X, -X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex, shadow_coord + vec4( -X, 0, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex, shadow_coord + vec4( -X, X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex, shadow_coord + vec4( 0, -X, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex, shadow_coord + vec4( 0, X, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex, shadow_coord + vec4( X, -X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex, shadow_coord + vec4( X, 0, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex, shadow_coord + vec4( X, X, 0, 0)).x * 0.0625;

	return ret;
}

float shadowCoeff2()
{
	const int index = 1;

	//int index = 3;
	//if(gl_FragCoord.z < far_d.x)
	//	index = 0;
	//else if(gl_FragCoord.z < far_d.y)
	//	index = 1;
	//else if(gl_FragCoord.z < far_d.z)
	//	index = 2;
	
	vec4 shadow_coord = gl_TextureMatrix[index]*vec4(gl_TexCoord[1].xyz, 1);

if(shadow_coord.x<0.0 || shadow_coord.y<0.0)
	return 1.0;
if(shadow_coord.x>1.0 || shadow_coord.y>1.0)
	return 1.0;

	shadow_coord.w = shadow_coord.z + shadowAdd;
	shadow_coord.z = 0.0;
//	return shadow2DArray(stex2, shadow_coord).x;

		//Bilinear weighted 4-tap filter 
//		vec2 pos = mod( shadow_coord.xy * texSize.x, 1.0);
//		vec2 offset = (0.5 - step( 0.5, pos)) * texSize.x;
//		float ret2 = shadow2DArray( stex, shadow_coord + vec4( offset, 0, 0)).x * (pos.x) * (pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( offset.x, -offset.y, 0, 0)).x * (pos.x) * (1-pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( -offset.x, offset.y, 0, 0)).x * (1-pos.x) * (pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( -offset.x, -offset.y, 0, 0)).x * (1-pos.x) * (1-pos.y); 
//		return ret2;

		float X = texSize.y;
		float ret2 = shadow2DArray(stex2, shadow_coord + vec4( -X, 0, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex2, shadow_coord + vec4( 0, -X, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex2, shadow_coord + vec4( 0, X, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex2, shadow_coord + vec4( X, 0, 0, 0)).x * 0.25;
		return ret2;

	//float X = texSize.y;
	//const float X = 1.0;
	float ret = shadow2DArray(stex2, shadow_coord).x * 0.25;
/*	ret += shadow2DArrayOffset(stex2, shadow_coord, ivec2( -X, -X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex2, shadow_coord, ivec2( -X, 0)).x * 0.125;
	ret += shadow2DArrayOffset(stex2, shadow_coord, ivec2( -X, X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex2, shadow_coord, ivec2( 0, -X)).x * 0.125;
	ret += shadow2DArrayOffset(stex2, shadow_coord, ivec2( 0, X)).x * 0.125;
	ret += shadow2DArrayOffset(stex2, shadow_coord, ivec2( X, -X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex2, shadow_coord, ivec2( X, 0)).x * 0.125;
	ret += shadow2DArrayOffset(stex2, shadow_coord, ivec2( X, X)).x * 0.0625;*/
	ret += shadow2DArray(stex2, shadow_coord + vec4( -X, -X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex2, shadow_coord + vec4( -X, 0, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex2, shadow_coord + vec4( -X, X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex2, shadow_coord + vec4( 0, -X, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex2, shadow_coord + vec4( 0, X, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex2, shadow_coord + vec4( X, -X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex2, shadow_coord + vec4( X, 0, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex2, shadow_coord + vec4( X, X, 0, 0)).x * 0.0625;

	return ret;
}

float shadowCoeff3()
{
	const int index = 2;

	//int index = 3;
	//if(gl_FragCoord.z < far_d.x)
	//	index = 0;
	//else if(gl_FragCoord.z < far_d.y)
	//	index = 1;
	//else if(gl_FragCoord.z < far_d.z)
	//	index = 2;
	
	vec4 shadow_coord = gl_TextureMatrix[index]*vec4(gl_TexCoord[1].xyz, 1);

if(shadow_coord.x<0.0 || shadow_coord.y<0.0)
	return 1.0;
if(shadow_coord.x>1.0 || shadow_coord.y>1.0)
	return 1.0;

	shadow_coord.w = shadow_coord.z + shadowAdd;
	shadow_coord.z = 0.0;
//	return shadow2DArray(stex3, shadow_coord).x;

		//Bilinear weighted 4-tap filter 
//		vec2 pos = mod( shadow_coord.xy * texSize.x, 1.0);
//		vec2 offset = (0.5 - step( 0.5, pos)) * texSize.x;
//		float ret2 = shadow2DArray( stex, shadow_coord + vec4( offset, 0, 0)).x * (pos.x) * (pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( offset.x, -offset.y, 0, 0)).x * (pos.x) * (1-pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( -offset.x, offset.y, 0, 0)).x * (1-pos.x) * (pos.y); 
//		ret2 += shadow2DArray( stex, shadow_coord + vec4( -offset.x, -offset.y, 0, 0)).x * (1-pos.x) * (1-pos.y); 
//		return ret2;

		float X = texSize.y;
		float ret2 = shadow2DArray(stex3, shadow_coord + vec4( -X, 0, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex3, shadow_coord + vec4( 0, -X, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex3, shadow_coord + vec4( 0, X, 0, 0)).x * 0.25;
		ret2 += shadow2DArray(stex3, shadow_coord + vec4( X, 0, 0, 0)).x * 0.25;
		return ret2;

	//float X = texSize.y;
	//const float X = 1.0;
	float ret = shadow2DArray(stex3, shadow_coord).x * 0.25;
/*	ret += shadow2DArrayOffset(stex3, shadow_coord, ivec2( -X, -X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex3, shadow_coord, ivec2( -X, 0)).x * 0.125;
	ret += shadow2DArrayOffset(stex3, shadow_coord, ivec2( -X, X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex3, shadow_coord, ivec2( 0, -X)).x * 0.125;
	ret += shadow2DArrayOffset(stex3, shadow_coord, ivec2( 0, X)).x * 0.125;
	ret += shadow2DArrayOffset(stex3, shadow_coord, ivec2( X, -X)).x * 0.0625;
	ret += shadow2DArrayOffset(stex3, shadow_coord, ivec2( X, 0)).x * 0.125;
	ret += shadow2DArrayOffset(stex3, shadow_coord, ivec2( X, X)).x * 0.0625;*/
	ret += shadow2DArray(stex3, shadow_coord + vec4( -X, -X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex3, shadow_coord + vec4( -X, 0, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex3, shadow_coord + vec4( -X, X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex3, shadow_coord + vec4( 0, -X, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex3, shadow_coord + vec4( 0, X, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex3, shadow_coord + vec4( X, -X, 0, 0)).x * 0.0625;
	ret += shadow2DArray(stex3, shadow_coord + vec4( X, 0, 0, 0)).x * 0.125;
	ret += shadow2DArray(stex3, shadow_coord + vec4( X, X, 0, 0)).x * 0.0625;

	return ret;
}

/////////////////////////////////

float saturate(float value)
{
	return clamp(value, 0.0, 1.0);
}

vec2 SphereIntersection (vec3 rayStart, vec3 rayDir, vec3 sphereCenter, float sphereRadius)
{
	rayStart -= sphereCenter;
	float a = dot(rayDir, rayDir);
	float b = 2.0 * dot(rayStart, rayDir);
	float c = dot(rayStart, rayStart) - (sphereRadius * sphereRadius);
	float d = b * b - 4 * a * c;
	if (d < 0)
	{
		return vec2(-1.0, -1.0);
	}
	else
	{
		d = sqrt(d);
		return vec2(-b - d, -b + d) / (2 * a);
	}
}
vec2 PlanetIntersection (vec3 rayStart, vec3 rayDir)
{
	return SphereIntersection(rayStart, rayDir, vec3(0, -6371000, 0), 6371000);
}
vec2 AtmosphereIntersection (vec3 rayStart, vec3 rayDir)
{
	return SphereIntersection(rayStart, rayDir, vec3(0, -6371000, 0), 6371000 + 100000);
}

float PhaseRayleigh (float costh)
{
	return 3 * (1 + costh*costh) / (16 * 3.14159265359);
}
float PhaseMie (float costh, float g)
{
	g = min(g, 0.9381);
	float k = 1.55*g - 0.55*g*g*g;
	float kcosth = k*costh;
	return (1 - k*k) / ((4 * 3.14159265359) * (1-kcosth) * (1-kcosth));
}

float AtmosphereHeight (vec3 positionWS)
{
	return distance(positionWS, vec3(0, -6371000, 0)) - 6371000;
}
float DensityRayleigh (float h)
{
	return exp(-max(0, h / (100000 * 0.08)));
}
float DensityMie (float h)
{
	return exp(-max(0, h / (100000 * 0.012)));
}
float DensityOzone (float h)
{
	return max(0, 1 - abs(h - 25000.0) / 15000.0);
}
vec3 AtmosphereDensity (float h)
{
	return vec3(DensityRayleigh(h), DensityMie(h), DensityOzone(h));
}

vec3 IntegrateOpticalDepth (vec3 rayStart, vec3 rayDir)
{
	vec2 intersection = AtmosphereIntersection(rayStart, rayDir);
	float  rayLength    = intersection.y;

	int    sampleCount  = 8;
	float  stepSize     = rayLength / sampleCount;
	vec3 opticalDepth = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < sampleCount; i++)
	{
		vec3 localPosition = rayStart + rayDir * (float(i) + 0.5) * stepSize;
		float  localHeight   = AtmosphereHeight(localPosition);
		vec3 localDensity  = AtmosphereDensity(localHeight) * stepSize;

		opticalDepth += localDensity;
	}

	return opticalDepth;
}

vec3 Absorb (vec3 opticalDepth)
{
	return exp(-(opticalDepth.x * (vec3(5.802, 13.558, 33.100) * 1e-6) + opticalDepth.y * (vec3(3.996,  3.996,  3.996) * 1e-6) * 1.1 + opticalDepth.z * (vec3(0.650,  1.881,  0.085) * 1e-6)) * 1);
}

vec3 IntegrateScattering (vec3 rayStart, vec3 rayDir, float rayLength, vec3 lightDir, vec3 lightColor, out vec3 transmittance)
{
	float  rayHeight = AtmosphereHeight(rayStart);
	float  sampleDistributionExponent = 1 + saturate(1 - rayHeight / 100000) * 8; // Slightly arbitrary max exponent of 9

	vec2 intersection = AtmosphereIntersection(rayStart, rayDir);
	
	rayLength = min(rayLength, intersection.y);
	if (intersection.x > 0)
	{
		rayStart += rayDir * intersection.x;
		rayLength -= intersection.x;
	}

	float  costh     = dot(rayDir, lightDir);
	float  phaseRayleigh = PhaseRayleigh(costh);
	float  phaseMie = PhaseMie(costh, 0.85);
	vec3 rayleigh = vec3(0.0, 0.0, 0.0);
	vec3 mie      = vec3(0.0, 0.0, 0.0);

	//int    sampleCount = 64;
	int    sampleCount = 16;
	vec3 opticalDepth = vec3(0.0, 0.0, 0.0);
	
	float prevRayTime = 0;
	for (int i = 0; i < sampleCount; i++)
	{
		float  rayTime = pow(float(i) / sampleCount, sampleDistributionExponent) * rayLength;
		float  stepSize = (rayTime - prevRayTime);
		
		vec3 localPosition = rayStart + rayDir * rayTime;
	
		float  localHeight   = AtmosphereHeight(localPosition);
		vec3 localDensity  = AtmosphereDensity(localHeight) * stepSize;

		opticalDepth += localDensity;
	
		vec3 opticalDepthlight  = IntegrateOpticalDepth(localPosition, lightDir);
		vec3 lightTransmittance = Absorb(opticalDepth + opticalDepthlight);
	
		rayleigh += lightTransmittance * phaseRayleigh * localDensity.x;
		mie      += lightTransmittance * phaseMie      * localDensity.y;

		prevRayTime = rayTime;
	}

	transmittance = Absorb(opticalDepth);

	return (rayleigh * (vec3(5.802, 13.558, 33.100) * 1e-6) + mie * (vec3(3.996,  3.996,  3.996) * 1e-6)) * lightColor * 20;
}

/////////////////////////////////

const float R0 = 6360e3;
const float Ra = 6380e3;
const vec3 bR = vec3(58e-7, 135e-7, 331e-7);
const vec3 bMs = vec3(2e-5);
const vec3 bMe = bMs * 1.1;
const float I = 10.;
const vec3 C = vec3(0., -R0, 0.);

vec2 densitiesRM(vec3 p) {
	float h = max(0., length(p - C) - R0);
	return vec2(exp(-h/8e3), exp(-h/12e2));
}

float escape(vec3 p, vec3 d, float R)
{
	vec3 v = p - C;
	float b = dot(v, d);
	float det = b * b - dot(v, v) + R*R;
	if (det < 0.) return -1.;
	det = sqrt(det);
	float t1 = -b - det;
	float t2 = -b + det;
	return (t1 >= 0.) ? t1 : t2;
}

vec2 scatterDepthInt(vec3 o, vec3 d, float L, float steps) {
	vec2 depthRMs = vec2(0.);

	L /= steps;
	d *= L;
	
	for (float i = 0.; i < steps; ++i)
		depthRMs += densitiesRM(o + d * i);

	return depthRMs * L;
}

vec2 totalDepthRM;
vec3 I_R;
vec3 I_M;

vec3 sundir;

void scatterIn(vec3 o, vec3 d, float L, float steps)
{
	L /= steps;
	d *= L;

	for (float i = 0.; i < steps; ++i)
	{
		vec3 p = o + d * i;

		vec2 dRM = densitiesRM(p) * L;

		totalDepthRM += dRM;

		vec2 depthRMsum = totalDepthRM + scatterDepthInt(p, sundir, escape(p, sundir, Ra), 4.);

		vec3 A = exp(-bR * depthRMsum.x - bMe * depthRMsum.y);

		I_R += A * dRM.x;
		I_M += A * dRM.y;
	}
}

vec3 scatter(vec3 o, vec3 d, float L, vec3 Lo)
{
	totalDepthRM = vec2(0.);

	I_R = I_M = vec3(0.);

	scatterIn(o, d, L, 16.);

	float mu = dot(d, sundir);

	return Lo * exp(-bR * totalDepthRM.x - bMe * totalDepthRM.y)

		+ I * (1. + mu * mu) * (
			I_R * bR * .0597 +
			I_M * bMs * .0196 / pow(1.58 - 1.52 * mu, 1.5));
}

/////////////////////////////////

//uniform float RollOff = 0.5;
void main()
{
	//### use these 3 lines for flat shading
//	vec3 dx = dFdx(varyPos.xyz);
//	vec3 dy = dFdy(varyPos.xyz);
//	vec3 normal = normalize(cross(dx, dy));
//	vec3 normal = normalize(gl_TexCoord[2].xyz);	// ***Lego envmap

	vec3 normal;
	if(useExplicitVertexNormals > 0)
	{
		normal = normalize(gl_TexCoord[2].xyz);	// ***Lego envmap
	}
	else
	{
		vec3 dx = dFdx(varyPos.xyz);
		vec3 dy = dFdy(varyPos.xyz);
		normal = normalize(cross(dx, dy));
	}

//	gl_FragColor = vec4(normal, 1.0);
//	return;

// This produces weird black artefacts on the edges of models when using smooth normals for lighting
//	if(dot(normal, gl_TexCoord[1].xyz) > 0)
//	{
//		normal.xyz *= -1;
//	}	

	float s = 1.0 / (1.0 + numShadows);	//*********
	//float s = 1.0;//*********
	float brightness = dot(normal, -parallelLightDir);
	
//	float diffuse = s * (0.3 + 0.7 * max(brightness,0.0));

		float diffuse = s * mix(max(brightness, 0.0), 0.5 + brightness*0.5, 0.2);

		diffuse*=varyIntensity;

	if(numShadows >= 1)
	{
		vec3 lvec = normalize(spotLightPos - gl_TexCoord[1].xyz);
		float cosine = dot(lvec, spotLightDir);
		float intensity = smoothstep(spotLightCosineDecayBegin, spotLightCosineDecayEnd, cosine);
		float ldn = dot(normal, lvec);
		float diffComp = max(0.0, ldn);

		float shadowC = shadowCoeff1();
//		float shadowC = shadowCoeff(0);

//			float interp_shadow = shadowC;
//			interp_shadow -= dFdx(interp_shadow) * (float(int(gl_FragCoord.x) & 1) - 0.5);
//			interp_shadow -= dFdy(interp_shadow) * (float(int(gl_FragCoord.y) & 1) - 0.5);
//			if(interp_shadow>=0.0)
//			{
//				shadowC = interp_shadow;
//			}

		float shadowFactor = ((1.0 - shadowAmbient)*shadowC + shadowAmbient);
		diffuse += s * diffComp*intensity * shadowFactor;		//*********
		//diffuse *= shadowC;
	}

	if(numShadows >= 2)
	{
		vec3 lvec = normalize(spotLightPos2 - gl_TexCoord[1].xyz);
		float cosine = dot(lvec, spotLightDir2);
		float intensity = smoothstep(spotLightCosineDecayBegin2, spotLightCosineDecayEnd2, cosine);
		float ldn = dot(normal, lvec);
		float diffComp = max(0.0, ldn);

		float shadowC = shadowCoeff2();
//		float shadowC = shadowCoeff(1);
		float shadowFactor = ((1.0 - shadowAmbient)*shadowC + shadowAmbient);
		diffuse += s * diffComp*intensity * shadowFactor;
	}

	if(numShadows >= 3)
	{
		vec3 lvec = normalize(spotLightPos3 - gl_TexCoord[1].xyz);
		float cosine = dot(lvec, spotLightDir3);
		float intensity = smoothstep(spotLightCosineDecayBegin3, spotLightCosineDecayEnd3, cosine);
		float ldn = dot(normal, lvec);
		float diffComp = max(0.0, ldn);

		float shadowC = shadowCoeff3();
//		float shadowC = shadowCoeff(2);
		float shadowFactor = ((1.0 - shadowAmbient)*shadowC + shadowAmbient);
		diffuse += s * diffComp*intensity * shadowFactor;
	}

	diffuse *= 1.0 + 0.1 * numShadows;	//*********
	
	diffuse = clamp(diffuse, 0.0, 1.0);

//		vec4 color = gl_Color;
//		gl_FragColor = gl_Color;

	vec4 color;
	if (useTexture > 0)
		color = texture2D(texture, gl_TexCoord[0].st);
		//color = texture(texture, gl_TexCoord[0].st);
	else
		color = gl_Color;

		vec4 Saved = color;

/*	if (envMapping > 0)
	{
		vec3 nn = varyNormal;
		if(dot(nn, gl_TexCoord[1].xyz) > 0)
		{
			nn.xyz *= -1;
		}
		vec3 r = reflect(varyPos, nn);*/
/*		vec3 r = reflect(varyPos, varyNormal);
//		vec3 r = reflect(varyPos, normal);
		float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z + 1.0)*(r.z + 1.0));
		//float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
		vec2 coord = vec2(r.x / m + 0.5, r.y / m + 0.5);
		color = mix(texture2D(envTexture, coord), color, 1.0 - envMapping);
		//color = texture2D(envTexture, coord);
		//color = color + texture2D(envTexture, coord)*envMapping;
	}*/

	color *= diffuse;

/*// envmap
	if (envMapping > 0)
	{
		//vec2 coord = vec2(varyNormal.x + 0.5, varyNormal.y + 0.5);
		//vec2 coord = vec2(normal.x + 0.5, normal.y + 0.5);
		vec2 coord = vec2(varyNormal.x*0.5 + 0.5, varyNormal.y*0.5 + 0.5);

//			vec3 r = reflect(varyPos, varyNormal);
//			float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z + 1.0)*(r.z + 1.0));
//			vec2 coord = vec2(r.x / m + 0.5, r.y / m + 0.5);


		color = mix(texture2D(envTexture, coord), color, 1.0 - envMapping);
	}*/

	if (envMapping > 0)
	{
//		vec3 smooth_normal = normalize(gl_TexCoord[2].xyz);
//		vec3 r = reflect(varyPos, smooth_normal);
//		vec3 r = reflect(varyPos, varyNormal);
		vec3 r = reflect(varyPos, normal);
		float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z + 1.0)*(r.z + 1.0));
		vec2 coord = vec2(r.x / m + 0.5, r.y / m + 0.5);

		color += texture2D(envTexture, coord) * envMapping;
		//color += texture(envTexture, coord) * envMapping;
	}


//	color += vec4(0.2, 0.2, 0.2, 0.0);
//	color += vec4(0.1, 0.1, 0.1, 0.0);

//	const vec4 specularColor = vec4(1.0, 1.0, 1.0, 1.0);
//	vec3 r = reflect(spotLightDir, normal);	

//	vec3 eyeVec = normalize(gl_TexCoord[1].xyz);
//	color += specularCoeff * 20.0*pow(max(0.0, dot(r,eyeVec)), 100.0)*specularColor;

	if(reflectionCoeff!=0)
	{
		vec4 reflectColor = texture2DRect(reflectionTex, gl_FragCoord.xy);
//		reflectColor *= (intensity*shadowC + intensity2*shadowC2 + intensity3*shadowC3);
		reflectColor *= diffuse;
		color = reflectionCoeff * reflectColor + (1.0 - reflectionCoeff) * color;
//color = texture2DRect(reflectionTex, gl_FragCoord.xy);
	}
	
//	gl_FragColor = vec4(color.xyz, gl_Color.w);
		gl_FragColor = vec4(color.xyz, Saved.w);
			//vec3 gamma_corrected = pow(vec3(color.xyz), vec3(0.4545));
			//gl_FragColor = vec4(gamma_corrected.xyz, Saved.w);

//#ifdef USE_FOG
//	float fog = clamp(gl_Fog.scale*(gl_Fog.end+gl_TexCoord[1].z), 0.0, gl_Color.w);
//	vec4 fogCol = gl_Fog.color;
//	gl_FragColor = mix(fogCol, gl_FragColor, fog);
//#endif

		//vec3 vv = mod(abs(varyWorldPos), 255.0);
		//gl_FragColor = vec4(vv/255.0, 1.0);

//	vec3 ld = vec3(1.0, 1.0, 1.0);
//	ld = normalize(ld);
//	vec3 rd = normalize(varyWorldPos - camPos);
//	float sundot = clamp(dot(rd,ld),0.0,1.0);
//	//gl_FragColor = vec4(sundot, sundot, sundot, Saved.w);
//	//color.xyz += 0.3*vec3(1.3,0.7,0.3)*pow( sundot, 8.0 );
//	color.xyz += 0.3*vec3(2.0,2.0,2.0)*pow( sundot, 8.0 );
//	gl_FragColor = vec4(color.xyz, Saved.w);


	if(numShadows == 42)
	{
				sundir = sunDir;
				vec3 O	= camPos;
				vec3 D	= varyWorldPos - camPos;
				float rayLength2 = length(D);
				D /= rayLength2;

				//float L = rayLength2;//escape(O, D, Ra);
				//float L = escape(O, D, rayLength2);
				float L = rayLength2*2.0;
				color.xyz = scatter(O, D, L, color.xyz);

				gl_FragColor = vec4(sqrt(color.xyz), 1.);
	}

	return;

	if(numShadows == 42)
	{

	vec3 fogColor0 = 0.65*vec3(0.4, 0.65, 1.0);
	vec3 fogColor1 = vec3(1.0, 1.0, 1.0);

//	float maxz = 1000.0;
//	float t = gl_TexCoord[1].z;
//	float fo = 1.0-exp(-pow(t/maxz, 1.5) );
	float maxz = 1000.0;
	float t = -gl_TexCoord[1].z;
	if(t<0.0)
		t=0.0;
	if(t>maxz)
		t=maxz;
//	float fo = t/maxz;
	float fo = 1.0-exp(-pow(t/maxz, 1.5) );
//	gl_FragColor = vec4(fo, fo, fo, Saved.w);
	vec3 fogColor2 = vec3(fo);
	//vec3 colf = mix( fogColor2, color.xyz, 1.0-(t/maxz) );
	vec3 colf = fogColor2;
	gl_FragColor = vec4(colf, Saved.w);
	return;

	float miny = 0.0;
	float maxy = 100.0;
	float t2 = varyWorldPos.y;
	if(t2>maxy)
		t2=maxy;
	if(t2<miny)
		t2=miny;
	t2 = (t2-miny)/(maxy-miny);
	//float fo2 = 1.0-exp(-pow(t2, 1.5) );
	//float fo2 = t2*(fo);
	float fo2 = t2*(1.0-fo);
	vec3 col2 = mix( fogColor1, color.xyz, fo2 );
	gl_FragColor = vec4(col2, Saved.w);


//
//	float limit2 = 100.0;
//	float t2 = varyWorldPos.y;
//	float fo2 = 1.0-exp(-pow(t2/limit2, 1.5) );
//	vec3 fogColor1 = vec3(1.0, 1.0, 1.0);
//
//	vec3 col = mix( color.xyz, fogColor0, fo );
//	vec3 col2 = mix( fogColor1, col, fo2*(1.0-fo) );
//
//	gl_FragColor = vec4(col2, Saved.w);

		return;
	}

//### varyPos is in camera space!
	//vec3 rayStart = camPos;
	//vec3 rayDir = varyPos - camPos;
//		vec3 rayDir = varyPos;
//		float rayLength = length(rayDir);
//		rayDir /= rayLength;
	//gl_FragColor = vec4(rayDir.xyz, 1.0);
//	gl_FragColor = vec4(varyPos.x, varyPos.x, varyPos.x, 1.0);
//	gl_FragColor.xyz = rayDir*0.5 + vec3(gl_FragColor.xyz)*0.5;

	vec3 rayStart = camPos;
	vec3 rayDir = varyWorldPos - camPos;
	float rayLength = length(rayDir);
	rayDir /= rayLength;
	//gl_FragColor = vec4(rayDir.xyz, 1.0);
	//gl_FragColor = vec4(varyWorldPos, 1.0);


//	vec3 lightDir = normalize(parallelLightDir);
//	gl_FragColor = vec4(lightDir, 1.0);

				//vec3 lightDir = vec3(1.0, 0.1, 1.0);
				//lightDir = normalize(lightDir);
	vec3 lightDir = sunDir;
				//gl_FragColor = vec4(lightDir, 1.0);

				//gl_FragColor = vec4(parallelLightDir, 1.0);
				//lightDir = normalize(lightDir);
				vec3 lightColor = vec3(1.0, 1.0, 1.0);

//				float dp = dot(normal, lightDir);
//				gl_FragColor.xyz = vec3(dp);
//				gl_FragColor.xyz = normal;

//	gl_FragColor.xyz = gl_Color.xyz;
//	{
//		vec3 dx = dFdx(varyPos.xyz);
//		vec3 dy = dFdy(varyPos.xyz);
//		vec3 n = normalize(cross(dx, dy));
//		gl_FragColor.xyz = n;
//		float dp = dot(n, lightDir);
//		gl_FragColor.xyz = vec3(dp);
//
//	float diffuse = (0.3 + 0.7 * max(dot(n, -lightDir),0.0));
//	diffuse = clamp(diffuse, 0.0, 1.0);
//	vec4 c = gl_Color * diffuse;
//	gl_FragColor.xyz = c.xyz;
//	}


				// Directional light transmittance (planet shadow)
				vec3 lightTransmittance = Absorb(IntegrateOpticalDepth(varyWorldPos, lightDir));
					//gl_FragColor = vec4(lightTransmittance, 1.0);
					//return;
				// Get a very rough ambient term by sampling the sky straight upwards
				vec3 foo;
				vec3 ambient = vec3(0.1) * lightTransmittance;
				//vec3 ambient = IntegrateScattering(varyWorldPos, vec3(0, 1, 0), 5000000, lightDir, lightColor, foo);
					//gl_FragColor = vec4(ambient, 1.0);
					//return;
				// Combine lighting
				//vec3 color2 = vec3(gl_Color.xyz) * max(0, dot(normalize(normal), lightDir)) * (ambient + lightColor * lightTransmittance);
				//vec3 color2 = vec3(color.xyz) * (ambient + lightColor * lightTransmittance);
				//vec3 color2 = (ambient + vec3(color.xyz)) * lightTransmittance;
				vec3 color2 = ambient + (vec3(color.xyz) * lightTransmittance);
				//vec3 color2 = vec3(color.xyz) * lightTransmittance;
					//gl_FragColor = vec4(color2, 1.0);
					//return;
				// Calculate and apply atmospheric scattering + transmittance
				vec3 transmittance;
				vec3 scattering = IntegrateScattering(rayStart, rayDir, rayLength/**10000*/, lightDir, lightColor, transmittance);
				color2 = color2 * transmittance + scattering;
				vec3 gamma_corrected = color2;//pow(vec3(color2.xyz), vec3(0.4545));
				gl_FragColor.xyz = gamma_corrected;
				return;

//	vec3 transmittance;
//	{
//		vec2 planetIntersection = PlanetIntersection(rayStart, rayDir);
//		if (planetIntersection.x > 0)
//			rayLength = min(rayLength, planetIntersection.x);
//	}
//	vec3 tmp = IntegrateScattering(rayStart, rayDir, rayLength, lightDir, lightColor, transmittance);
//	gl_FragColor = vec4( vec3(color.xyz)*transmittance + tmp, 1.0 );
}

