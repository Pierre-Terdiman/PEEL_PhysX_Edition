///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLShader.h"
#include "GLRenderHelpers.h"
#include "Camera.h"
#include "GLPointRenderer.h"
#include "PEEL.h"

// Random experiments / obsolete code

extern udword gScreenWidth;
extern udword gScreenHeight;

void Experiments()
{
	if(0)	// Modern GL test (instancing)
	{
		void ModernTest();
		ModernTest();
	}

	// Screen quad stuff
	if(0)
	{
		static const char* vs = "#version 130\n" STRINGIFY(

			varying vec3 varyPos;

			void main()
			{
				varyPos = gl_Normal;

				gl_FrontColor = gl_Color;
				gl_Position = gl_Vertex;
//					gl_Position = ftransform();
			}
		);

		static const char* ps = "#version 130\n" STRINGIFY(
			uniform vec3 camPos;
			varying vec3 varyPos;

			vec2 intersectRayPlane(vec3 orig, vec3 dir)
			{
			//	const float dn = dir.dot(plane.n);
				float dn = dir.y;
			    
			//	if(-1E-7f < dn && dn < 1E-7f)
			//		return false; // parallel

			//	float distanceAlongLine = -plane.distance(orig)/dn;
				float distanceAlongLine = -orig.y/dn;

			//			*pointOnPlane = orig + distanceAlongLine * dir;
				vec3 pointOnPlane = orig + distanceAlongLine * dir;

				return vec2(distanceAlongLine, pointOnPlane.y);
			}

			// http://iquilezles.org/www/articles/checkerfiltering/checkerfiltering.htm
			float checkersGradBox( in vec2 p )
			{
				// filter kernel
				vec2 w = 1.0*fwidth(p) + 0.001;
				// analytical integral (box filter)
				vec2 i = 2.0*(abs(fract((p-0.5*w)*0.5)-0.5)-abs(fract((p+0.5*w)*0.5)-0.5))/w;
				// xor pattern
				return 0.5 - 0.5*i.x*i.y;                  
			}

			vec3 render( in vec3 ro, in vec3 rd )
			{ 
			//    vec3 col = vec3(0.7, 0.9, 1.0) +rd.y*0.8;
			    
				vec3 col = vec3(228.0/255.0, 207.0/255.0, 160.0/255.0) - rd.y*0.8;
				vec3 col2b = vec3(210.0/255.0, 176.0/255.0, 98.0/255.0) + rd.y*0.8;
			    
			    
				vec3 col2 = col;
			//    col2 = vec3(0.75, 0.75, 0.75);// +rd.y*0.8;
			//    vec3 col3 = vec3(0.075, 0.5, 0.75);
			//    vec2 res = castRay(ro,rd);
				vec2 res = intersectRayPlane(ro,rd);

				float t = res.x;
				float m = res.y;
			//    if( m>-0.5 )
			//    if( m>100000.5 )
			//    if(t<50.0)
				if(t>0.0)
				{
					vec3 pos = ro + t*rd;
			        
					// material        
			//		col = 0.45 + 0.35*sin( vec3(0.05,0.08,0.10)*(m-1.0) );
			//        if( m<1.5 )
					{
			            
						float f = checkersGradBox( 5.0*pos.xz );	//5.0
						float maxDist = 20.0;
						float coeff = t/maxDist;
			//            coeff *= coeff;
						coeff = clamp(coeff, 0.0, 1.0);
			//            col = 0.3 + f*vec3(0.1);            
						col = col2b + 0.2*vec3(f);
			            
			            
			//            col = col2;
			//            col *= 1.0 - coeff;
						col = mix(col, col2, coeff);
			//            col = mix(col3, col, coeff);
			            
			//		    vec3 col2 = vec3(0.7, 0.9, 1.0) + t*0.8;
			 //           col.y = col2.y;
					}


				}

				return vec3( clamp(col,0.0,1.0) );
			}

			void main()
			{
//					vec3 rd = normalize(gl_Color.xyz);
//					vec3 rd = varyPos.xyz;
				vec3 rd = normalize(varyPos.xyz);

//					gl_FragColor = vec4(rd, 1.0);

				vec3 col = render( camPos, rd );
				col = pow( col, vec3(0.4545) );
				gl_FragColor = vec4( col, 1.0 );

				gl_FragDepth = 0.95;
			}
		);

		static bool InitDone = false;
		static GLuint Program = 0;
		if(!InitDone)
		{
			InitDone = true;
			Program = GLShader::CompileProgram(vs, ps, null);
		}

		SetupProjectionMatrix();
		SetupModelViewMatrix();

		Matrix4x4 ViewMatrix;
		GetModelViewMatrix(ViewMatrix);

		Matrix4x4 ProjMatrix;
		GetProjMatrix(ProjMatrix);

		const Point p0b(-1.0f, -1.0f, 0.0f);
		const Point p1b(-1.0f,  1.0f, 0.0f);
		const Point p2b( 1.0f,  1.0f, 0.0f);
		const Point p3b( 1.0f, -1.0f, 0.0f);

		// ICE: Extract the camera's up and right vector
		// TODO: move to camera file
		Point cy, cx;
		{
			cx.x = ViewMatrix.m[0][0]; cx.y = ViewMatrix.m[1][0]; cx.z = ViewMatrix.m[2][0]; // Right
			cy.x = ViewMatrix.m[0][1]; cy.y = ViewMatrix.m[1][1]; cy.z = ViewMatrix.m[2][1]; // Up
		}

		const Point Pos(0.0f, 0.0f, 0.0f);

//			Point vp0 = Pos * ViewMatrix;
//			Point vp1 = (Pos + Point(1.0f, 0.0f, 0.0f)) * ViewMatrix;
//			printf("%f\n", (vp1-vp0).Magnitude());


		const float radius = 2.0f;
		const Point Ex = cx*radius;
		const Point Ey = cy*radius;

		const Point p0 = Pos - Ex - Ey;
		const Point p1 = Pos - Ex + Ey;
		const Point p2 = Pos + Ex + Ey;
		const Point p3 = Pos + Ex - Ey;

//			const Point c0(1.0f, 0.0f, 0.0f);
//			const Point c1(0.0f, 1.0f, 0.0f);
//			const Point c2(1.0f, 1.0f, 0.0f);
//			const Point c3(0.0f, 0.0f, 1.0f);

		const Point c0 = ComputeWorldRay(0, gScreenHeight-1);
		const Point c1 = ComputeWorldRay(0, 0);
		const Point c2 = ComputeWorldRay(gScreenWidth-1, 0);
		const Point c3 = ComputeWorldRay(gScreenWidth-1, gScreenHeight-1);

		const Point CamPos = GetCameraPos();
//			Point c0 = p0 - CamPos;	c0.Normalize();
//			Point c1 = p1 - CamPos;	c1.Normalize();
//			Point c2 = p2 - CamPos;	c2.Normalize();
//			Point c3 = p3 - CamPos;	c3.Normalize();

		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glUseProgram(Program);
		GLShader::SetUniform3f(Program, "camPos", CamPos.x, CamPos.y, CamPos.z);

		glBegin(GL_TRIANGLES);
			glNormal3f(c0.x, c0.y, c0.z);
//				glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p0b.x, p0b.y, p0b.z);

			glNormal3f(c1.x, c1.y, c1.z);
//				glVertex3f(p1.x, p1.y, p1.z);
			glVertex3f(p1b.x, p1b.y, p1b.z);

			glNormal3f(c2.x, c2.y, c2.z);
//				glVertex3f(p2.x, p2.y, p2.z);
			glVertex3f(p2b.x, p2b.y, p2b.z);

			glNormal3f(c0.x, c0.y, c0.z);
//				glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p0b.x, p0b.y, p0b.z);

			glNormal3f(c3.x, c3.y, c3.z);
//				glVertex3f(p3.x, p3.y, p3.z);
			glVertex3f(p3b.x, p3b.y, p3b.z);

			glNormal3f(c2.x, c2.y, c2.z);
//				glVertex3f(p2.x, p2.y, p2.z);
			glVertex3f(p2b.x, p2b.y, p2b.z);
		glEnd();

/*			glBegin(GL_TRIANGLE_STRIP);
			glNormal3f(c0.x, c0.y, c0.z);
			glVertex3f(p0.x, p0.y, p0.z);

			glNormal3f(c1.x, c1.y, c1.z);
			glVertex3f(p1.x, p1.y, p1.z);

			glColor3f(c3.x, c3.y, c3.z);
			glVertex3f(p3.x, p3.y, p3.z);

			glNormal3f(c2.x, c2.y, c2.z);
			glVertex3f(p2.x, p2.y, p2.z);
		glEnd();*/

		glUseProgram(0);

		glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);
		glEnable(GL_LIGHTING);
	}

	// trying a real raytraced version
	if(0)
	{
		static const char* vs = "#version 130\n" STRINGIFY(

			varying vec3 varyPos;

			void main()
			{
//					gl_FrontColor = gl_Color;
//					gl_FrontColor = vec4(gl_Normal.x, gl_Normal.y, gl_Normal.z);
//					gl_BackColor = gl_Color;

//					vec3 N = gl_Normal.xyz;
//					gl_FrontColor = vec4(gl_Normal.xyz, 1.0);

//					varyPos = gl_Normal;

				varyPos = gl_Vertex;

				gl_Position = ftransform();
			}
		);

		static const char* ps = "#version 130\n" STRINGIFY(

			uniform vec3 spherePos;
			uniform vec3 camPos;
			uniform float sphereRadius;
//				uniform float constantScale;

			varying vec3 varyPos;

			float intersectRaySphereBasic(const vec3 origin, const vec3 dir, float length, const vec3 center, float radius)
			{
				const vec3 offset = center - origin;

				const float ray_dist = dot(dir, offset);

				const float off2 = dot(offset, offset);
				float rad_2 = radius * radius;
				if(off2 <= rad_2)
					return 0.0;

				if(ray_dist <= 0 || (ray_dist - length) > radius)
					return -1.0;

				float d = rad_2 - (off2 - ray_dist * ray_dist);
				if(d<0.0)
					return -1.0;

				float dist = ray_dist - sqrt(d);
				if(dist > length)
					return -1.0;

//					*hit = origin + dir * dist;
				return dist;
			}

			void main()
			{
/*					const float Width	= 768.0;
				const float Height	= 768.0;

				float xs = gl_FragCoord.x;
				float ys = gl_FragCoord.y;

				float u = ((xs - Width*0.5)/Width)*2.0;
				float v = -((ys - Height*0.5)/Height)*2.0;

				float gFOV = 60.0;
				float HTan = tan(0.25 * (0.01745329251994329577 * gFOV * 2.0));
				float VTan = HTan*(Width/Height);

//					const Point CamRay(VTan*u, HTan*v, 1.0f);
				gl_FragColor = vec4(abs(VTan*u), abs(HTan*v), 0.0, 1.0);*/

//					vec3 dir = normalize(vec3(gl_Normal.x, gl_Normal.y, gl_Normal.z));
//					vec3 dir = normalize(vec3(gl_Color.x, gl_Color.y, gl_Color.z));
//					vec3 dir = normalize(varyPos);
				vec3 dir = normalize(varyPos - camPos);
//					vec3 dir = vec3(gl_Color.x, gl_Color.y, gl_Color.z);
				

				float dist = intersectRaySphereBasic(camPos, dir, 10000.0, spherePos, sphereRadius);
//
				if(dist>=0.0)
				{
					vec3 hit = camPos + dir * dist;

					vec3 sphereNormal = normalize(hit - spherePos);

//						gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);

						vec3 L = vec3(0.5, 0.5, 0.5);
						normalize(L);

						float d = dot(L, sphereNormal);
						gl_FragColor = vec4(d, d, d, 1.0);

					vec4 ndcPos = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(hit.xyz, 1.0);
					ndcPos.z /= ndcPos.w;
					float depth = ndcPos.z*0.5 + 0.5;
					gl_FragDepth = depth;
				}
				else
					discard;
//						gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
//						gl_FragColor = vec4(dir.x, dir.y, dir.z, 1.0);

//					gl_FragColor = vec4(dir.x, dir.y, dir.z, 1.0);

//					gl_FragColor = vec4(gl_FragCoord.x/768.0, 0.0, 0.0, 1.0);
//					gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
//					gl_FragColor = gl_Color;
//					gl_FragColor = vec4(gl_Color.x, gl_Color.y, 0.0, 1.0);
			}
		);

		static bool InitDone = false;
		static GLuint Program = 0;
		if(!InitDone)
		{
			InitDone = true;
			Program = GLShader::CompileProgram(vs, ps, null);
		}

		SetupProjectionMatrix();
		SetupModelViewMatrix();

		Matrix4x4 ViewMatrix;
		GetModelViewMatrix(ViewMatrix);

		Matrix4x4 ProjMatrix;
		GetProjMatrix(ProjMatrix);

		// ICE: Extract the camera's up and right vector
		// TODO: move to camera file
		Point cy, cx;
		{
			cx.x = ViewMatrix.m[0][0]; cx.y = ViewMatrix.m[1][0]; cx.z = ViewMatrix.m[2][0]; // Right
			cy.x = ViewMatrix.m[0][1]; cy.y = ViewMatrix.m[1][1]; cy.z = ViewMatrix.m[2][1]; // Up
		}

		const Point Pos(0.0f, 0.0f, 0.0f);

//			Point vp0 = Pos * ViewMatrix;
//			Point vp1 = (Pos + Point(1.0f, 0.0f, 0.0f)) * ViewMatrix;
//			printf("%f\n", (vp1-vp0).Magnitude());


		const float radius = 1.0f;
		const Point Ex = cx*radius;
		const Point Ey = cy*radius;

		const Point p0 = Pos - Ex - Ey;
		const Point p1 = Pos - Ex + Ey;
		const Point p2 = Pos + Ex + Ey;
		const Point p3 = Pos + Ex - Ey;

		const Point CamPos = GetCameraPos();
		Point d0 = p0 - CamPos;	d0.Normalize();
		Point d1 = p1 - CamPos;	d1.Normalize();
		Point d2 = p2 - CamPos;	d2.Normalize();
		Point d3 = p3 - CamPos;	d3.Normalize();

		glColor3f(0.0f, 1.0f, 0.0f);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glUseProgram(Program);
		GLShader::SetUniform3f(Program, "camPos", CamPos.x, CamPos.y, CamPos.z);
		GLShader::SetUniform3f(Program, "spherePos", Pos.x, Pos.y, Pos.z);
		GLShader::SetUniform1f(Program, "sphereRadius", radius);

		// TODO: technically we don't need to pass all the zs, they're the same
		// TODO: use color x/y for something else, it's free right now
		glBegin(GL_TRIANGLES);
//				glColor3f(1.0f, 0.0f, 0.0f);
//				glColor3f(0.0f, 0.0f, 0.0f);
//				glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
			glColor3f(d0.x, d0.y, d0.z);
			glNormal3f(d0.x, d0.y, d0.z);
			glVertex3f(p0.x, p0.y, p0.z);

//				glColor3f(0.0f, 1.0f, 0.0f);
//				glColor3f(0.0f, 1.0f, 0.0f);
//				glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			glColor3f(d1.x, d1.y, d1.z);
			glNormal3f(d1.x, d1.y, d1.z);
			glVertex3f(p1.x, p1.y, p1.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 1.0f, 0.0f);
//				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glColor3f(d2.x, d2.y, d2.z);
			glNormal3f(d2.x, d2.y, d2.z);
			glVertex3f(p2.x, p2.y, p2.z);

//				glColor3f(1.0f, 0.0f, 0.0f);
//				glColor3f(0.0f, 0.0f, 0.0f);
//				glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
			glColor3f(d0.x, d0.y, d0.z);
			glNormal3f(d0.x, d0.y, d0.z);
			glVertex3f(p0.x, p0.y, p0.z);

//				glColor3f(0.0f, 1.0f, 0.0f);
//				glColor3f(1.0f, 1.0f, 0.0f);
//				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glColor3f(d2.x, d2.y, d2.z);
			glNormal3f(d2.x, d2.y, d2.z);
			glVertex3f(p2.x, p2.y, p2.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 0.0f, 0.0f);
//				glColor4f(1.0f, 0.0f, 1.0f, 0.0f);
			glColor3f(d3.x, d3.y, d3.z);
			glNormal3f(d3.x, d3.y, d3.z);
			glVertex3f(p3.x, p3.y, p3.z);
		glEnd();
		glUseProgram(0);


		GLRenderHelpers::DrawLine(p0, p1, Point(1.0f, 1.0f, 1.0f));
		GLRenderHelpers::DrawLine(p1, p2, Point(1.0f, 1.0f, 1.0f));
		GLRenderHelpers::DrawLine(p2, p3, Point(1.0f, 1.0f, 1.0f));
		GLRenderHelpers::DrawLine(p3, p0, Point(1.0f, 1.0f, 1.0f));


		glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);
		glEnable(GL_LIGHTING);
	}


	// Basic shader tests
	if(0)
	{
		// Command line to test this:
		// -p PINT_PhysX35_New.dll -t EmptyScene
		static const char* vs = "#version 130\n" STRINGIFY(
			void main()
			{
				gl_FrontColor = gl_Color;
//					gl_BackColor = gl_Color;

				gl_Position = ftransform();
			}
		);

		static const char* ps = "#version 130\n" STRINGIFY(

			uniform vec3 spherePos;
			uniform float sphereRadius;
			uniform float constantScale;

			void main()
			{
//					gl_FragColor = vec4(gl_FragCoord.x/768.0, 0.0, 0.0, 1.0);
//					gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
//					gl_FragColor = gl_Color;
//					gl_FragColor = vec4(gl_Color.x, gl_Color.y, 0.0, 1.0);

				// gl_Color.x = u between 0.0 and 1.0
				// gl_Color.y = v between 0.0 and 1.0
//					vec3 normal = vec3(gl_Color.x - 0.5, gl_Color.y - 0.5, 0.0f);
//					float mag = dot(normal.xy, normal.xy);
//					if(mag > 1.0)
//						discard;

				vec3 normal;
//					normal.xy = gl_Color.xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
				normal.xy = gl_Color.zw*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
				float mag = dot(normal.xy, normal.xy);
				if(mag > 1.0)
				{
//						gl_FragColor = vec4(1.0, 0, 0, 1.0);
					discard;   // kill pixels outside circle
				}
				else
				{
					normal.z = sqrt(1.0-mag);

					float radius = sphereRadius;
					vec4 viewPos = gl_ModelViewMatrix*vec4(spherePos.xyz, 1.0);
					vec3 eyePos = viewPos.xyz + normal*radius;
					vec4 ndcPos = gl_ProjectionMatrix * vec4(eyePos, 1.0);

//						float coeff = 1.0f * ndcPos.z;
//						float coeff = 1.0f / viewPos.z;

					ndcPos.z /= ndcPos.w;
					float depth = ndcPos.z*0.5 + 0.5;
					gl_FragDepth = depth;

					// depth = 0 => 0.95
					// depth = 1000 => 0.9
//						float limit = 1.0 - depth*0.1;
//						float limit = 1.0 - depth*0.04;
//						float limit = 1.0 - 1.0/depth;

					//limit = a*depth+b;
					//depth = 0 => b = 0.95
					//depth = 1000 => limit = 0.5 => 0.5 = a*1000 + 0.95 => a = (0.5 - 0.95)/1000
					float lowLimit = 0.95;
					float highLimit = 0.05;
					float a = (highLimit - lowLimit)/100;
					float b = lowLimit;
					float limit = a*depth+b;

//						limit = 0.9*0.001f/constantScale;
					limit = 10.0;

//						float coeff = 1.0f * ndcPos.z;
					if(mag > limit)
//						if(mag > 0.95)
//						if(mag > 0.9*coeff)
					{
						gl_FragColor = vec4(1.0, 0, 0, 1.0);
					}
					else
					{
						vec3 L = vec3(0.5, 0.5, 0.5);
						normalize(L);

						float d = dot(L, normal);
						gl_FragColor = vec4(d, d, d, 1.0);
//							gl_FragColor = vec4(depth, depth, depth, 1.0);
					}

				}
			}
		);

		static bool InitDone = false;
		static GLuint Program = 0;
		if(!InitDone)
		{
			InitDone = true;
			Program = GLShader::CompileProgram(vs, ps, null);
		}

		SetupProjectionMatrix();
		SetupModelViewMatrix();

		Matrix4x4 ViewMatrix;
		GetModelViewMatrix(ViewMatrix);

		Matrix4x4 ProjMatrix;
		GetProjMatrix(ProjMatrix);

		// ICE: Extract the camera's up and right vector
		// TODO: move to camera file
		Point cy, cx;
		{
			cx.x = ViewMatrix.m[0][0]; cx.y = ViewMatrix.m[1][0]; cx.z = ViewMatrix.m[2][0]; // Right
			cy.x = ViewMatrix.m[0][1]; cy.y = ViewMatrix.m[1][1]; cy.z = ViewMatrix.m[2][1]; // Up
		}

		const Point Pos(0.0f, 0.0f, 0.0f);

//			const float radius = 0.5f;
		const float radius = 1.0f;
//			const float radius = 10.0f;
//			const float radius = 0.05f;
		float Scale = ComputeConstantScale(Pos, ViewMatrix, ProjMatrix);
//			float Scale = 100.0f*ComputeConstantScale(Pos, ViewMatrix, ProjMatrix);


//			const Point Ex(1.0f, 0.0f, 0.0f);
//			const Point Ey(0.0f, 1.0f, 0.0f);
		const Point Ex = cx*radius;
		const Point Ey = cy*radius;

		const Point p0 = Pos - Ex - Ey;
		const Point p1 = Pos - Ex + Ey;
		const Point p2 = Pos + Ex + Ey;
		const Point p3 = Pos + Ex - Ey;

		glColor3f(0.0f, 1.0f, 0.0f);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glUseProgram(Program);
		GLShader::SetUniform3f(Program, "spherePos", Pos.x, Pos.y, Pos.z);
		GLShader::SetUniform1f(Program, "sphereRadius", radius);
		GLShader::SetUniform1f(Program, "constantScale", Scale);

		// TODO: technically we don't need to pass all the zs, they're the same
		// TODO: use color x/y for something else, it's free right now
/*			glBegin(GL_TRIANGLES);
//				glColor3f(1.0f, 0.0f, 0.0f);
//				glColor3f(0.0f, 0.0f, 0.0f);
			glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
			glVertex3f(p0.x, p0.y, p0.z);

//				glColor3f(0.0f, 1.0f, 0.0f);
//				glColor3f(0.0f, 1.0f, 0.0f);
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			glVertex3f(p1.x, p1.y, p1.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 1.0f, 0.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex3f(p2.x, p2.y, p2.z);

//				glColor3f(1.0f, 0.0f, 0.0f);
//				glColor3f(0.0f, 0.0f, 0.0f);
			glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
			glVertex3f(p0.x, p0.y, p0.z);

//				glColor3f(0.0f, 1.0f, 0.0f);
//				glColor3f(1.0f, 1.0f, 0.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex3f(p2.x, p2.y, p2.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 0.0f, 0.0f);
			glColor4f(1.0f, 0.0f, 1.0f, 0.0f);
			glVertex3f(p3.x, p3.y, p3.z);*/

/*			glBegin(GL_TRIANGLE_FAN);
//				glColor3f(1.0f, 0.0f, 0.0f);
//				glColor3f(0.0f, 0.0f, 0.0f);
			glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
			glVertex3f(p0.x, p0.y, p0.z);

//				glColor3f(0.0f, 1.0f, 0.0f);
//				glColor3f(0.0f, 1.0f, 0.0f);
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			glVertex3f(p1.x, p1.y, p1.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 1.0f, 0.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex3f(p2.x, p2.y, p2.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 0.0f, 0.0f);
			glColor4f(1.0f, 0.0f, 1.0f, 0.0f);
			glVertex3f(p3.x, p3.y, p3.z);
		glEnd();*/

		glBegin(GL_TRIANGLE_STRIP);
//				glColor3f(1.0f, 0.0f, 0.0f);
//				glColor3f(0.0f, 0.0f, 0.0f);
			glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
			glVertex3f(p0.x, p0.y, p0.z);

//				glColor3f(0.0f, 1.0f, 0.0f);
//				glColor3f(0.0f, 1.0f, 0.0f);
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			glVertex3f(p1.x, p1.y, p1.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 0.0f, 0.0f);
			glColor4f(1.0f, 0.0f, 1.0f, 0.0f);
			glVertex3f(p3.x, p3.y, p3.z);

//				glColor3f(0.0f, 0.0f, 1.0f);
//				glColor3f(1.0f, 1.0f, 0.0f);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex3f(p2.x, p2.y, p2.z);
		glEnd();

		glUseProgram(0);
		glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);
		glEnable(GL_LIGHTING);
	}


	if(0)
	{
		const Point color(1.0f, 1.0f, 1.0f);
		udword nb_pts = 1;
		const Point pt(0.0f, 0.0f, 0.0f);
		udword stride = sizeof(Point);
		GLPointRenderer::Draw(color, nb_pts, &pt, stride);
	}


	if(0)
	{
		const Point color(1.0f, 0.0f, 0.0f);
		const Point p(0.0f, 1.0f, 0.0f);
//			GLRenderHelpers::DrawLine(p, p, color);

		glDisable(GL_LIGHTING);
		glColor4f(color.x, color.y, color.z, 1.0f);
//			const Point Pts[] = { p };
//			glEnableClientState(GL_VERTEX_ARRAY);
//			glVertexPointer(3, GL_FLOAT, sizeof(Point), &Pts[0].x);
//			glDrawArrays(GL_LINES, 0, 2);
//			glDisableClientState(GL_VERTEX_ARRAY);

			glBegin(GL_POINTS);
			glVertex3f(p.x, p.y, p.z);
			glVertex3f(p.x+10.0f, p.y, p.z);
			glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_LIGHTING);
	

#ifdef REMOVED
		float size = 1.0f;
		{
			glPointSize(size);

/*				glUseProgram(0);
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
			}*/

			glBegin(GL_POINTS);
		}

		//void DrawPoint(const Vec3& p, const Vec4& color)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 1.0f, 0.0f);
		}

		{
			glEnd();
		}
#endif
	}
}

