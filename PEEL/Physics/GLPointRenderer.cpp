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

