///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "GLPointRenderer.h"
#include "GLShader.h"
#include "Camera.h"

// vertex shader
static const char *vertexPointShader = "#version 130\n" STRINGIFY(

uniform float pointRadius;  // point size in world space
uniform float pointScale;   // scale to calculate size in pixels

uniform mat4 lightTransform; 
uniform vec3 lightDir;
uniform vec3 lightDirView;

uniform vec4 colors[8];

uniform vec4 transmission;
uniform int mode;

//in int density;
in float density;
in int phase;
in vec4 velocity;

void main()
{
    // calculate window-space point size
	vec4 viewPos = gl_ModelViewMatrix*vec4(gl_Vertex.xyz, 1.0);

	gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
	gl_PointSize = -pointScale * (pointRadius / viewPos.z);

	gl_TexCoord[0] = gl_MultiTexCoord0;

	gl_TexCoord[4].xyz = gl_Vertex.xyz;
	gl_TexCoord[5].xyz = viewPos.xyz;
}
);

// pixel shader for rendering points as shaded spheres
static const char *fragmentPointShader = "#version 130\n" STRINGIFY(

uniform vec3 lightDir;
uniform vec3 lightPos;
uniform float spotMin;
uniform float spotMax;
uniform int mode;

uniform sampler2DShadow shadowTex;
uniform vec2 shadowTaps[12];
uniform float pointRadius;  // point size in world space

void main()
{
    // calculate normal from texture coordinates
    vec3 normal;
    normal.xy = gl_TexCoord[0].xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(normal.xy, normal.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
   	normal.z = sqrt(1.0-mag);

//	normal = gl_ModelViewMatrixInverseTranspose * vec4(normal.xyz,0.0);
//	normal = vec4(normal.xyz,0.0) * gl_ModelViewMatrixInverseTranspose;
//	normal = gl_ModelViewMatrix * vec4(normal.xyz,0.0);
//	normal = vec4(normal.xyz,0.0) * gl_ModelViewMatrix;


//	if (mode == 2)
//	{
		vec3 parallelLightDir = vec3(1.0, 0.0, 0.0);

		float diffuse = (0.3 + 0.7 * max(dot(normal, -parallelLightDir),0.0));

		diffuse = clamp(diffuse, 0.0, 1.0);

		vec4 color = gl_Color * diffuse;

//		gl_FragColor = vec4(color.xyz, gl_Color.w);
		gl_FragColor = vec4(normal.xyz, gl_Color.w);
//		return;
//	}



	vec3 eyePos = gl_TexCoord[5].xyz + normal*pointRadius;//*2.0;
	vec4 ndcPos = gl_ProjectionMatrix * vec4(eyePos, 1.0);
	ndcPos.z /= ndcPos.w;
	gl_FragDepth = ndcPos.z*0.5 + 0.5;

}
);




static GLuint mPositionVBO = 0;
static int sprogram = -1;

static int gScreenWidth = 0;
static int gScreenHeight = 0;

void GLPointRenderer::Init()
{
	glGenBuffers(1, &mPositionVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * numFluidParticles, 0, GL_DYNAMIC_DRAW);

	if (sprogram == -1)
	{
		sprogram = GLShader::CompileProgram(vertexPointShader, fragmentPointShader, null);
	}
}

void GLPointRenderer::Close()
{
	glDeleteBuffers(1, &mPositionVBO);
}

void GLPointRenderer::SetScreenResolution(int width, int height)
{
	gScreenWidth = width;
	gScreenHeight = height;
}

void GLPointRenderer::Draw(const Point& color, udword nb_pts, const Point* pts, udword stride)
{
	if(1)
	{
//		float radius = 1.0f;
		float radius = 0.2f;
		float screenWidth = float(gScreenWidth);
		float screenHeight = float(gScreenHeight);
		float screenAspect = screenWidth / screenHeight;
//		float fov = PI / 4.0f;
		float fov = GetCameraFOV()*DEGTORAD;

//			void PointRenderer::Draw(int n, int offset, float radius, float screenWidth, float screenAspect, float fov)
			{
				if (sprogram)
				{
					glEnable(GL_POINT_SPRITE);
					glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
					glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
					//glDepthMask(GL_TRUE);
					glEnable(GL_DEPTH_TEST);
				
					int mode = 0;
			//		if (showDensity)
			//			mode = 1;
			//		if (shadowMap == NULL)
						mode = 2;

					glUseProgram(sprogram);
					glUniform1f( glGetUniformLocation(sprogram, "pointRadius"), radius);
					glUniform1f( glGetUniformLocation(sprogram, "pointScale"), screenWidth/screenAspect * (1.0f / (tanf(fov*0.5f))));
			//		glUniform1f( glGetUniformLocation(sprogram, "spotMin"), g_spotMin);
			//		glUniform1f( glGetUniformLocation(sprogram, "spotMax"), g_spotMax);
					glUniform1i( glGetUniformLocation(sprogram, "mode"), mode);
			//		glUniform4fv( glGetUniformLocation(sprogram, "colors"), 8, (float*)&g_colors[0].r);

					// set shadow parameters
			//		ShadowApply(sprogram, lightPos, lightTarget, lightTransform, shadowMap->texture);

//					glEnableClientState(GL_VERTEX_ARRAY);
//					glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
//					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * nb_pts, 0, GL_DYNAMIC_DRAW);

//					glVertexPointer(4, GL_FLOAT, 0, 0);

			/*		int d = glGetAttribLocation(sprogram, "density");
					int p = glGetAttribLocation(sprogram, "phase");

					if (d != -1)
					{
						glEnableVertexAttribArray(d);
						glBindBuffer(GL_ARRAY_BUFFER, colors);
						glVertexAttribPointer(d, 1,  GL_FLOAT, GL_FALSE, 0, 0);	// densities
					}

					if (p != -1)
					{
						glEnableVertexAttribArray(p);
						glBindBuffer(GL_ARRAY_BUFFER, colors);
						glVertexAttribIPointer(p, 1,  GL_INT, 0, 0);			// phases
					}*/

//					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

//					glDrawElements(GL_POINTS, n, GL_UNSIGNED_INT, (const void*)(offset*sizeof(int)));
//					glDrawArrays(GL_POINTS, 0, nb_pts);

	glBegin(GL_POINTS);
		const char* p2 = reinterpret_cast<const char*>(pts);
		udword nb_pts2 = nb_pts;
		while(nb_pts2--)
		{
			const float* f = reinterpret_cast<const float*>(p2);
			glVertex3fv(f);
			p2+=stride;
		}
	glEnd();


					glUseProgram(0);
//					glBindBuffer(GL_ARRAY_BUFFER, 0);
//					glDisableClientState(GL_VERTEX_ARRAY);	
/*					
					if (d != -1)
						glDisableVertexAttribArray(d);
					if (p != -1)
						glDisableVertexAttribArray(p);
					*/
					glDisable(GL_POINT_SPRITE);
					glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);		
				}
			}
	
	}


	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);

	const char* p = reinterpret_cast<const char*>(pts);

	glPointSize(2.0f);

//				glEnable(GL_BLEND);
//				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//				glEnable(GL_POINT_SPRITE);
//				glEnable(GL_POINT_SMOOTH);
//				glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glBegin(GL_POINTS);
		while(nb_pts--)
		{
			const float* f = reinterpret_cast<const float*>(p);
			glVertex3fv(f);

			p+=stride;
		}
	glEnd();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
}


/*
void BeginPoints(float size)
{
	glPointSize(size);

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	for (int i = 0; i < 8; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glDisable(GL_TEXTURE_2D);
	}

	glBegin(GL_POINTS);
}

void DrawPoint(const Vec3& p, const Vec4& color)
{
	glColor3fv(color);
	glVertex3fv(p);
}

void EndPoints()
{
	glEnd();
}
*/

#ifdef REMOVED

static GLuint mPositionVBO = 0;

void PointRenderer::Init()
{
	glGenBuffers(1, &mPositionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * numFluidParticles, 0, GL_DYNAMIC_DRAW);
}

void PointRenderer::Close()
{
	glDeleteBuffers(1, &mPositionVBO);
}


// vertex shader
const char *vertexPointShader = "#version 130\n" STRINGIFY(

uniform float pointRadius;  // point size in world space
uniform float pointScale;   // scale to calculate size in pixels

uniform mat4 lightTransform; 
uniform vec3 lightDir;
uniform vec3 lightDirView;

uniform vec4 colors[8];

uniform vec4 transmission;
uniform int mode;

//in int density;
in float density;
in int phase;
in vec4 velocity;

void main()
{
    // calculate window-space point size
	vec4 viewPos = gl_ModelViewMatrix*vec4(gl_Vertex.xyz, 1.0);

	gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
	gl_PointSize = -pointScale * (pointRadius / viewPos.z);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = lightTransform*vec4(gl_Vertex.xyz-lightDir*pointRadius*2.0, 1.0);
	gl_TexCoord[2] = gl_ModelViewMatrix*vec4(lightDir, 0.0);

	if (mode == 1)
	{
		// density visualization
		if (density < 0.0f)
			gl_TexCoord[3].xyz = mix(vec3(0.1, 0.1, 1.0), vec3(0.1, 1.0, 1.0), -density);
		else
			gl_TexCoord[3].xyz = mix(vec3(1.0, 1.0, 1.0), vec3(0.1, 0.2, 1.0), density);
	}
	else if (mode == 2)
	{
		gl_PointSize *= clamp(gl_Vertex.w*0.25, 0.0f, 1.0);

		gl_TexCoord[3].xyzw = vec4(clamp(gl_Vertex.w*0.05, 0.0f, 1.0));
	}
	else
	{
		gl_TexCoord[3].xyz = mix(colors[phase % 8].xyz*2.0, vec3(1.0), 0.1);
	}

	gl_TexCoord[4].xyz = gl_Vertex.xyz;
	gl_TexCoord[5].xyz = viewPos.xyz;
}
);

// pixel shader for rendering points as shaded spheres
const char *fragmentPointShader = STRINGIFY(

uniform vec3 lightDir;
uniform vec3 lightPos;
uniform float spotMin;
uniform float spotMax;
uniform int mode;

uniform sampler2DShadow shadowTex;
uniform vec2 shadowTaps[12];
uniform float pointRadius;  // point size in world space

// sample shadow map
float shadowSample()
{
	vec3 pos = vec3(gl_TexCoord[1].xyz/gl_TexCoord[1].w);
	vec3 uvw = (pos.xyz*0.5)+vec3(0.5);

	// user clip
	if (uvw.x  < 0.0 || uvw.x > 1.0)
		return 1.0;
	if (uvw.y < 0.0 || uvw.y > 1.0)
		return 1.0;
	
	float s = 0.0;
	float radius = 0.002;

	for (int i=0; i < 8; i++)
	{
		s += shadow2D(shadowTex, vec3(uvw.xy + shadowTaps[i]*radius, uvw.z)).r;
	}

	s /= 8.0;
	return s;
}

float sqr(float x) { return x*x; }

void main()
{
    // calculate normal from texture coordinates
    vec3 normal;
    normal.xy = gl_TexCoord[0].xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(normal.xy, normal.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
   	normal.z = sqrt(1.0-mag);

	if (mode == 2)
	{
		float alpha  = normal.z*gl_TexCoord[3].w;
		gl_FragColor.xyz = gl_TexCoord[3].xyz*alpha;
		gl_FragColor.w = alpha;
		return;
	}

    // calculate lighting
	float shadow = shadowSample();
	
	vec3 lVec = normalize(gl_TexCoord[4].xyz-(lightPos));
	vec3 lPos = vec3(gl_TexCoord[1].xyz/gl_TexCoord[1].w);
	float attenuation = max(smoothstep(spotMax, spotMin, dot(lPos.xy, lPos.xy)), 0.05);

	vec3 diffuse = vec3(0.9, 0.9, 0.9);
	vec3 reflectance =  gl_TexCoord[3].xyz;
	
	vec3 Lo = diffuse*reflectance*max(0.0, sqr(-dot(gl_TexCoord[2].xyz, normal)*0.5 + 0.5))*max(0.2,shadow)*attenuation;

	gl_FragColor = vec4(pow(Lo, vec3(1.0/2.2)), 1.0);

	vec3 eyePos = gl_TexCoord[5].xyz + normal*pointRadius;//*2.0;
	vec4 ndcPos = gl_ProjectionMatrix * vec4(eyePos, 1.0);
	ndcPos.z /= ndcPos.w;
	gl_FragDepth = ndcPos.z*0.5 + 0.5;
}
);

// vertex shader
const char *vertexShader = "#version 130\n" STRINGIFY(

uniform mat4 lightTransform; 
uniform vec3 lightDir;
uniform float bias;
uniform vec4 clipPlane;
uniform float expand;

uniform mat4 objectTransform;

void main()
{
	vec3 n = normalize((objectTransform*vec4(gl_Normal, 0.0)).xyz);
	vec3 p = (objectTransform*vec4(gl_Vertex.xyz, 1.0)).xyz;

    // calculate window-space point size
	gl_Position = gl_ModelViewProjectionMatrix * vec4(p + expand*n, 1.0);

	gl_TexCoord[0].xyz = n;
	gl_TexCoord[1] = lightTransform*vec4(p + n*bias, 1.0);
	gl_TexCoord[2] = gl_ModelViewMatrix*vec4(lightDir, 0.0);
	gl_TexCoord[3].xyz = p;
	gl_TexCoord[4] = gl_Color;
	gl_TexCoord[5] = gl_MultiTexCoord0;
	gl_TexCoord[6] = gl_SecondaryColor;
	gl_TexCoord[7] = gl_ModelViewMatrix*vec4(gl_Vertex.xyz, 1.0);

	gl_ClipDistance[0] = dot(clipPlane,vec4(gl_Vertex.xyz, 1.0));
}
);

const char *passThroughShader = STRINGIFY(

void main()
{
	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);

}
);

// pixel shader for rendering points as shaded spheres
const char *fragmentShader = STRINGIFY(

uniform vec3 lightDir;
uniform vec3 lightPos;
uniform float spotMin;
uniform float spotMax;
uniform vec3 color;
uniform vec4 fogColor;

uniform sampler2DShadow shadowTex;
uniform vec2 shadowTaps[12];

uniform sampler2D tex;
uniform bool sky;

uniform bool grid;
uniform bool texture;

float sqr(float x) { return x*x; }

// sample shadow map
float shadowSample()
{
	vec3 pos = vec3(gl_TexCoord[1].xyz/gl_TexCoord[1].w);
	vec3 uvw = (pos.xyz*0.5)+vec3(0.5);

	// user clip
	if (uvw.x  < 0.0 || uvw.x > 1.0)
		return 1.0;
	if (uvw.y < 0.0 || uvw.y > 1.0)
		return 1.0;
	
	float s = 0.0;
	float radius = 0.002;

	const int numTaps = 12;

	for (int i=0; i < numTaps; i++)
	{
		s += shadow2D(shadowTex, vec3(uvw.xy + shadowTaps[i]*radius, uvw.z)).r;
	}

	s /= numTaps;
	return s;
}

float filterwidth(vec2 v)
{
  vec2 fw = max(abs(dFdx(v)), abs(dFdy(v)));
  return max(fw.x, fw.y);
}

vec2 bump(vec2 x) 
{
	return (floor((x)/2) + 2.f * max(((x)/2) - floor((x)/2) - .5f, 0.f)); 
}

float checker(vec2 uv)
{
  float width = filterwidth(uv);
  vec2 p0 = uv - 0.5 * width;
  vec2 p1 = uv + 0.5 * width;
  
  vec2 i = (bump(p1) - bump(p0)) / width;
  return i.x * i.y + (1 - i.x) * (1 - i.y);
}

void main()
{
    // calculate lighting
	float shadow = max(shadowSample(), 0.5);

	vec3 lVec = normalize(gl_TexCoord[3].xyz-(lightPos));
	vec3 lPos = vec3(gl_TexCoord[1].xyz/gl_TexCoord[1].w);
	float attenuation = max(smoothstep(spotMax, spotMin, dot(lPos.xy, lPos.xy)), 0.05);
		
	vec3 n = gl_TexCoord[0].xyz;
	vec3 color = gl_TexCoord[4].xyz;

	if (!gl_FrontFacing)
	{
		color = gl_TexCoord[6].xyz;
		n *= -1.0f;
	}

	if (grid && (n.y >0.995))
	{
		color *= 1.0 - 0.25 * checker(vec2(gl_TexCoord[3].x, gl_TexCoord[3].z));
	}
	else if (grid && abs(n.z) > 0.995)
	{
		color *= 1.0 - 0.25 * checker(vec2(gl_TexCoord[3].y, gl_TexCoord[3].x));
	}

	if (texture)
	{
		color = texture2D(tex, gl_TexCoord[5].xy).xyz;
	}
	
	// direct light term
	float wrap = 0.0;
	vec3 diffuse = color*vec3(1.0, 1.0, 1.0)*max(0.0, (-dot(lightDir, n)+wrap)/(1.0+wrap)*shadow)*attenuation;
	
	// wrap ambient term aligned with light dir
	vec3 light = vec3(0.03, 0.025, 0.025)*1.5;
	vec3 dark = vec3(0.025, 0.025, 0.03);
	vec3 ambient = 4.0*color*mix(dark, light, -dot(lightDir, n)*0.5 + 0.5)*attenuation;

	vec3 fog = mix(vec3(fogColor), diffuse + ambient, exp(gl_TexCoord[7].z*fogColor.w));

	gl_FragColor = vec4(pow(fog, vec3(1.0/2.2)), 1.0);				
}
);

/*void ShadowApply(GLint sprogram, Vec3 lightPos, Vec3 lightTarget, Matrix44 lightTransform, GLuint shadowTex)
{
	GLint uLightTransform = glGetUniformLocation(sprogram, "lightTransform");
	glUniformMatrix4fv(uLightTransform, 1, false, lightTransform);

	GLint uLightPos = glGetUniformLocation(sprogram, "lightPos");
	glUniform3fv(uLightPos, 1, lightPos);
	
	GLint uLightDir = glGetUniformLocation(sprogram, "lightDir");
	glUniform3fv(uLightDir, 1, Normalize(lightTarget-lightPos));

	GLint uBias = glGetUniformLocation(sprogram, "bias");
	glUniform1f(uBias, g_shadowBias);

	const Vec2 taps[] = 
	{ 
		Vec2(-0.326212f,-0.40581f),Vec2(-0.840144f,-0.07358f),
		Vec2(-0.695914f,0.457137f),Vec2(-0.203345f,0.620716f),
		Vec2(0.96234f,-0.194983f),Vec2(0.473434f,-0.480026f),
		Vec2(0.519456f,0.767022f),Vec2(0.185461f,-0.893124f),
		Vec2(0.507431f,0.064425f),Vec2(0.89642f,0.412458f),
		Vec2(-0.32194f,-0.932615f),Vec2(-0.791559f,-0.59771f) 
	};
	
	GLint uShadowTaps = glGetUniformLocation(sprogram, "shadowTaps");
	glUniform2fv(uShadowTaps, 12, &taps[0].x);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
}*/



void PointRenderer::Draw(int n, int offset, float radius, float screenWidth, float screenAspect, float fov
						 //const Point& lightPos, const Point& lightTarget
						 //, Matrix44 lightTransform
//						 ShadowMap* shadowMap, bool showDensity
						 )
{
//	FluidRenderBuffersGL* buffers = reinterpret_cast<FluidRenderBuffersGL*>(buffersIn);
//	GLuint positions = buffers->mPositionVBO;
	GLuint positions = mPositionVBO;
//	GLuint colors = buffers->mDensityVBO;
//	GLuint indices = buffers->mIndices;

	static int sprogram = -1;
	if (sprogram == -1)
	{
		sprogram = CompileProgram(vertexPointShader, fragmentPointShader, null);
	}

	if (sprogram)
	{
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		//glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	
		int mode = 0;
//		if (showDensity)
//			mode = 1;
//		if (shadowMap == NULL)
			mode = 2;

		glUseProgram(sprogram);
		glUniform1f( glGetUniformLocation(sprogram, "pointRadius"), radius);
		glUniform1f( glGetUniformLocation(sprogram, "pointScale"), screenWidth/screenAspect * (1.0f / (tanf(fov*0.5f))));
//		glUniform1f( glGetUniformLocation(sprogram, "spotMin"), g_spotMin);
//		glUniform1f( glGetUniformLocation(sprogram, "spotMax"), g_spotMax);
		glUniform1i( glGetUniformLocation(sprogram, "mode"), mode);
//		glUniform4fv( glGetUniformLocation(sprogram, "colors"), 8, (float*)&g_colors[0].r);

		// set shadow parameters
//		ShadowApply(sprogram, lightPos, lightTarget, lightTransform, shadowMap->texture);

		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, positions);
		glVertexPointer(4, GL_FLOAT, 0, 0);

/*		int d = glGetAttribLocation(sprogram, "density");
		int p = glGetAttribLocation(sprogram, "phase");

		if (d != -1)
		{
			glEnableVertexAttribArray(d);
			glBindBuffer(GL_ARRAY_BUFFER, colors);
			glVertexAttribPointer(d, 1,  GL_FLOAT, GL_FALSE, 0, 0);	// densities
		}

		if (p != -1)
		{
			glEnableVertexAttribArray(p);
			glBindBuffer(GL_ARRAY_BUFFER, colors);
			glVertexAttribIPointer(p, 1,  GL_INT, 0, 0);			// phases
		}*/

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

		glDrawElements(GL_POINTS, n, GL_UNSIGNED_INT, (const void*)(offset*sizeof(int)));

		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableClientState(GL_VERTEX_ARRAY);	
		
		if (d != -1)
			glDisableVertexAttribArray(d);
		if (p != -1)
			glDisableVertexAttribArray(p);
		
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);		
	}
}

#endif