///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLRenderHelpers.h"
#include "PEEL.h"

#include <xmmintrin.h>
#include <emmintrin.h>

void RenderVolumeLine(const Point& p1, const Point& p2, float size, udword color);


void GLRenderHelpers::DrawLine(const Point& p0, const Point& p1, const Point& color)
{
	if(0)
	{
		RenderVolumeLine(p0, p1, 0.1f, 0);
	}
	else
	{
	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);
	const Point Pts[] = { p0, p1 };
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Point), &Pts[0].x);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	}
}

void GLRenderHelpers::DrawLines(udword nb_lines, const Point* pts, const Point& color)
{
	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Point), &pts->x);
	glDrawArrays(GL_LINES, 0, 2*nb_lines);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
}

extern udword gScreenWidth;
extern udword gScreenHeight;

static void StartOrtho(udword screen_width, udword screen_height)
{
	if(screen_width==INVALID_ID || screen_height==INVALID_ID)
	{
		screen_width = gScreenWidth;
		screen_height = gScreenHeight;
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, screen_width, 0, screen_height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

static void EndOrtho()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void GLRenderHelpers::DrawLines2D(const float* vertices, udword nb_verts, const Point& color)
{
/*	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Point), &vertices->x);

	StartOrtho(INVALID_ID, INVALID_ID);

	glDrawArrays(GL_LINE_STRIP, 0, nb_verts);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	EndOrtho();*/

	udword NbToGo = nb_verts-1;
	while(NbToGo--)
	{
		float x0 = vertices[0];
		float y0 = vertices[1];
		float x1 = vertices[2];
		float y1 = vertices[3];
		DrawLine2D(x0, x1, y0, y1, color);
		vertices += 2;
	}
}

void GLRenderHelpers::DrawLine2D(float x_start, float x_end, float y_start, float y_end, const Point& color)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	StartOrtho(gScreenWidth, gScreenHeight);

	const float x0 = x_start * float(gScreenWidth);
	const float x1 = x_end * float(gScreenWidth);
	const float y0 = y_start * float(gScreenHeight);
	const float y1 = y_end * float(gScreenHeight);

	const Point p0(x0,	y0, 0.0f);
	const Point p2(x1,	y1, 0.0f);

	{
		glColor4f(color.x, color.y, color.z, 1.0f);
		glBegin(GL_LINES);
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p2.x, p2.y, p2.z);
		glEnd();
	}

	EndOrtho();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

static void DrawSegments(udword nb, const Point* segments, const Point& color)
{
	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Point), &segments->x);
	glDrawArrays(GL_LINES, 0, nb*2);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
}

void GLRenderHelpers::DrawCircle(udword nb_segments, const Matrix4x4& matrix, const Point& color, float radius, bool semi_circle)
{
	const float step = TWOPI / float(nb_segments);
	udword segs = nb_segments;
	if(semi_circle)
		segs /= 2;

	Point* tmp = (Point*)StackAlloc(sizeof(Point)*segs*2);
	for(udword i=0;i<segs;i++)
	{
		udword j=i+1;
		if(j==nb_segments)
			j=0;

		const float angle0 = float(i) * step;
		const float angle1 = float(j) * step;

		const Point p0 = Point(radius * sinf(angle0), radius * cosf(angle0), 0.0f) * matrix;
		const Point p1 = Point(radius * sinf(angle1), radius * cosf(angle1), 0.0f) * matrix;

//		DrawLine(p0, p1, color);
		tmp[i*2+0] = p0;
		tmp[i*2+1] = p1;
	}
	DrawSegments(segs, tmp, color);
}

void GLRenderHelpers::DrawTriangle(const Point& p0, const Point& p1, const Point& p2, const Point& color)
{
	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);
	const Point Pts[] = {p0, p1, p2};
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(Point), &Pts[0].x);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
}

void GLRenderHelpers::DrawRectangle(float x_start, float x_end, float y_start, float y_end, const Point& color_top, const Point& color_bottom, float alpha, udword screen_width, udword screen_height, bool draw_outline, udword texture_flags)
{
	if(alpha!=1.0f)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	const bool texturing = texture_flags!=SQT_DISABLED;

	if(texturing)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	StartOrtho(screen_width, screen_height);

	const float x0 = x_start * float(screen_width);
	const float x1 = x_end * float(screen_width);
	const float y0 = y_start * float(screen_height);
	const float y1 = y_end * float(screen_height);

	const Point p0(x0,	y0, 0.0f);
	const Point p1(x0,	y1, 0.0f);
	const Point p2(x1,	y1, 0.0f);
	const Point p3(x1,	y0, 0.0f);

	const float u = (texture_flags & SQT_FLIP_U) ? 1.0f : 0.0f;
	const float v = (texture_flags & SQT_FLIP_V) ? 1.0f : 0.0f;

	const float UScale = 1.0f;
	const float VScale = 1.0f;

	const Point t0(u*UScale, v*VScale, 0.0f);
	const Point t1(u*UScale, (1.0f - v)*VScale, 0.0f);
	const Point t2((1.0f - u)*UScale, (1.0f - v)*VScale, 0.0f);
	const Point t3((1.0f - u)*UScale, v*VScale, 0.0f);

	glBegin(GL_TRIANGLES);
		glTexCoord2f(t0.x, t0.y);
		glColor4f(color_top.x, color_top.y, color_top.z, alpha);
		glVertex3f(p0.x, p0.y, p0.z);

		glTexCoord2f(t1.x, t1.y);
		glColor4f(color_bottom.x, color_bottom.y, color_bottom.z, alpha);
		glVertex3f(p1.x, p1.y, p1.z);

		glTexCoord2f(t2.x, t2.y);
		glColor4f(color_bottom.x, color_bottom.y, color_bottom.z, alpha);
		glVertex3f(p2.x, p2.y, p2.z);

		glTexCoord2f(t0.x, t0.y);
		glColor4f(color_top.x, color_top.y, color_top.z, alpha);
		glVertex3f(p0.x, p0.y, p0.z);

		glTexCoord2f(t2.x, t2.y);
		glColor4f(color_bottom.x, color_bottom.y, color_bottom.z, alpha);
		glVertex3f(p2.x, p2.y, p2.z);

		glTexCoord2f(t3.x, t3.y);
		glColor4f(color_top.x, color_top.y, color_top.z, alpha);
		glVertex3f(p3.x, p3.y, p3.z);
	glEnd();

	if(draw_outline)
	{
		glDisable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		glBegin(GL_LINES);
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p1.x, p1.y, p1.z);

			glVertex3f(p1.x, p1.y, p1.z);
			glVertex3f(p2.x, p2.y, p2.z);

			glVertex3f(p2.x, p2.y, p2.z);
			glVertex3f(p3.x, p3.y, p3.z);

			glVertex3f(p3.x, p3.y, p3.z);
			glVertex3f(p0.x, p0.y, p0.z);
		glEnd();
	}

	EndOrtho();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

static __declspec(align(16)) const float minus1w[4] = { 0.0f, 0.0f, 0.0f, -1.0f };
static __declspec(align(16)) const udword gMaskXYZ[4] = { 0xffffffff, 0xffffffff, 0xffffffff, 0 };

static inline_ __m128 _mm_mul_add_ps(const __m128 a, const __m128 b, const __m128 c)
{
	return _mm_add_ps(_mm_mul_ps(a, b), c);
}

static inline_ __m128 _mm_neg_ps(const __m128 f)
{
	return _mm_sub_ps(_mm_setzero_ps(), f);
}

static inline_ __m128 _mm_clearw_ps(const __m128 v)
{
	return _mm_and_ps(v, (__m128&)gMaskXYZ);
}

static inline_ __m128 _mm_getw_ps(const __m128 f)
{
	return _mm_shuffle_ps(f, f, _MM_SHUFFLE(3, 3, 3, 3));
}

static inline_ __m128 _mm_getx_ps(const __m128 f)
{
	return _mm_shuffle_ps(f, f, _MM_SHUFFLE(0, 0, 0, 0));
}

static inline_ __m128 _mm_gety_ps(const __m128 f)
{
	return _mm_shuffle_ps(f, f, _MM_SHUFFLE(1, 1, 1, 1));
}

static inline_ __m128 _mm_getz_ps(const __m128 f)
{
	return _mm_shuffle_ps(f, f, _MM_SHUFFLE(2, 2, 2, 2));
}

static inline_ void QuatGetMat33V(const __m128 q, __m128& column0, __m128& column1, __m128& column2)
{
    const __m128 q2 = _mm_add_ps(q, q);
    const __m128 qw2 = _mm_mul_add_ps(q2, _mm_getw_ps(q), _mm_load_ps(minus1w));
    const __m128 nw2 = _mm_clearw_ps(_mm_neg_ps(qw2));
    const __m128 v = _mm_clearw_ps(q);

    const __m128 a0 = _mm_shuffle_ps(qw2, nw2, _MM_SHUFFLE(3, 1, 2, 3));
    column0 = _mm_mul_add_ps(v, _mm_getx_ps(q2), a0);

    const __m128 a1 = _mm_shuffle_ps(qw2, nw2, _MM_SHUFFLE(3, 2, 0, 3));
    column1 = _mm_mul_add_ps(v, _mm_gety_ps(q2), _mm_shuffle_ps(a1, a1, _MM_SHUFFLE(3, 1, 0, 2)));

    const __m128 a2 = _mm_shuffle_ps(qw2, nw2, _MM_SHUFFLE(3, 0, 1, 3));
    column2 = _mm_mul_add_ps(v, _mm_getz_ps(q2), _mm_shuffle_ps(a2, a2, _MM_SHUFFLE(3, 0, 2, 1)));
}

void GLRenderHelpers::SetupGLMatrix(const PR& pose)
{
	if(0)
	{
		const Matrix4x4 M = pose;
		glMultMatrixf(&M.m[0][0]);

		// 90 lines
/*
010285F0  push        ebp  
010285F1  mov         ebp,esp 
010285F3  and         esp,0FFFFFFF0h 
010285F6  sub         esp,0C0h 
010285FC  mov         eax,dword ptr [pose] 
010285FF  movss       xmm0,dword ptr [eax+0Ch] 
01028604  movss       xmm3,dword ptr [eax+10h] 
01028609  movss       xmm2,dword ptr [__real@40000000 (1127994h)] 
01028611  movss       xmm6,dword ptr [eax+14h] 
01028616  movaps      xmm5,xmm0 
01028619  mulss       xmm5,xmm2 
0102861D  movaps      xmm1,xmm6 
01028620  mulss       xmm1,xmm2 
01028624  movaps      xmm4,xmm3 
01028627  mulss       xmm4,xmm2 
0102862B  movss       xmm2,dword ptr [eax+18h] 
01028630  movaps      xmm7,xmm2 
01028633  mulss       xmm7,xmm5 
01028637  movss       dword ptr [esp+8],xmm7 
0102863D  movaps      xmm7,xmm2 
01028640  mulss       xmm2,xmm1 
01028644  movss       dword ptr [esp+0Ch],xmm2 
0102864A  mulss       xmm7,xmm4 
0102864E  mulss       xmm6,xmm1 
01028652  movaps      xmm2,xmm0 
01028655  mulss       xmm2,xmm5 
01028659  movss       dword ptr [esp+10h],xmm2 
0102865F  movaps      xmm2,xmm0 
01028662  mulss       xmm2,xmm4 
01028666  mulss       xmm0,xmm1 
0102866A  movaps      xmm5,xmm3 
0102866D  mulss       xmm5,xmm4 
01028671  movss       xmm4,dword ptr [__real@3f800000 (11273E0h)] 
01028679  mulss       xmm3,xmm1 
0102867D  subss       xmm4,xmm5 
01028681  subss       xmm4,xmm6 
01028685  movaps      xmm1,xmm2 
01028688  movss       dword ptr [esp+14h],xmm6 
0102868E  movss       xmm6,dword ptr [esp+0Ch] 
01028694  addss       xmm1,xmm6 
01028698  movss       dword ptr [esp+60h],xmm1 
0102869E  movaps      xmm1,xmm0 
010286A1  subss       xmm1,xmm7 
010286A5  subss       xmm2,xmm6 
010286A9  movss       xmm6,dword ptr [esp+8] 
010286AF  movss       dword ptr [esp+64h],xmm1 
010286B5  movss       xmm1,dword ptr [__real@3f800000 (11273E0h)] 
010286BD  subss       xmm1,dword ptr [esp+10h] 
010286C3  movss       dword ptr [esp+68h],xmm2 
010286C9  addss       xmm0,xmm7 
010286CD  movss       xmm7,dword ptr [eax+8] 
010286D2  movss       dword ptr [esp+18h],xmm4 
010286D8  movss       xmm4,dword ptr [esp+60h] 
010286DE  movaps      xmm2,xmm1 
010286E1  subss       xmm2,dword ptr [esp+14h] 
010286E7  movss       dword ptr [esp+6Ch],xmm2 
010286ED  movss       dword ptr [esp+0B8h],xmm7 
010286F6  movss       xmm7,dword ptr [esp+68h] 
010286FC  movss       dword ptr [esp+1Ch],xmm4 
01028702  movss       xmm4,dword ptr [esp+64h] 
01028708  movss       dword ptr [esp+38h],xmm0 
0102870E  movss       xmm0,dword ptr [esp+0B8h] 
01028717  movaps      xmm2,xmm3 
0102871A  addss       xmm2,xmm6 
0102871E  subss       xmm3,xmm6 
01028722  movss       xmm6,dword ptr [eax+4] 
01028727  subss       xmm1,xmm5 
0102872B  movss       xmm5,dword ptr [eax] 
0102872F  movss       dword ptr [esp+20h],xmm4 
01028735  xorps       xmm4,xmm4 
01028738  movss       dword ptr [esp+28h],xmm7 
0102873E  movss       xmm7,dword ptr [esp+6Ch] 
01028744  movss       dword ptr [esp+50h],xmm0 
0102874A  movss       xmm0,dword ptr [__real@3f800000 (11273E0h)] 
01028752  movss       dword ptr [esp+24h],xmm4 
01028758  movss       dword ptr [esp+2Ch],xmm7 
0102875E  movss       dword ptr [esp+30h],xmm2 
01028764  movss       dword ptr [esp+34h],xmm4 
0102876A  movss       dword ptr [esp+3Ch],xmm3 
01028770  movss       dword ptr [esp+40h],xmm1 
01028776  movss       dword ptr [esp+44h],xmm4 
0102877C  movss       dword ptr [esp+48h],xmm5 
01028782  movss       dword ptr [esp+4Ch],xmm6 
01028788  movss       dword ptr [esp+54h],xmm0 
0102878E  lea         eax,[esp+18h] 
01028792  push        eax  
01028793  call        dword ptr [__imp__glMultMatrixf@4 (1126B3Ch)] 
01028799  mov         esp,ebp 
0102879B  pop         ebp  
0102879C  ret              
*/
	}
	else
	{
		__declspec(align(16)) float m[16];

		const __m128 posV = _mm_loadu_ps(&pose.mPos.x);
		_mm_storeu_ps(&m[12], posV);

		const __m128 rotV = _mm_loadu_ps(&pose.mRot.p.x);
		QuatGetMat33V(rotV, *(__m128*)&m[0], *(__m128*)&m[4], *(__m128*)&m[8]);

		m[15] = 1.0f;

		glMultMatrixf(m);

		void SetGlobalPoseRenderModel(const float*);
		SetGlobalPoseRenderModel(m);

		// 50 lines
/*
009A8930  push        ebp  
009A8931  mov         ebp,esp 
009A8933  and         esp,0FFFFFFF0h 
009A8936  sub         esp,40h 
009A8939  mov         eax,dword ptr [pose] 
009A893C  movups      xmm3,xmmword ptr [eax+0Ch] 
009A8940  movups      xmm0,xmmword ptr [eax] 
009A8943  movaps      xmm4,xmmword ptr [gMaskXYZ (0AADD60h)] 
009A894A  movups      xmmword ptr [esp+30h],xmm0 
009A894F  movaps      xmm2,xmmword ptr [minus1w (0AADD50h)] 
009A8956  movaps      xmm1,xmm3 
009A8959  addps       xmm1,xmm3 
009A895C  movaps      xmm0,xmm3 
009A895F  shufps      xmm0,xmm3,0FFh 
009A8963  mulps       xmm0,xmm1 
009A8966  addps       xmm0,xmm2 
009A8969  xorps       xmm2,xmm2 
009A896C  subps       xmm2,xmm0 
009A896F  andps       xmm2,xmm4 
009A8972  andps       xmm4,xmm3 
009A8975  movaps      xmm5,xmm0 
009A8978  shufps      xmm5,xmm2,0DBh 
009A897C  movaps      xmm3,xmm1 
009A897F  shufps      xmm3,xmm1,0 
009A8983  mulps       xmm3,xmm4 
009A8986  addps       xmm3,xmm5 
009A8989  movaps      xmmword ptr [esp],xmm3 
009A898D  movaps      xmm3,xmm0 
009A8990  shufps      xmm0,xmm2,0C7h 
009A8994  shufps      xmm3,xmm2,0E3h 
009A8998  movaps      xmm2,xmm1 
009A899B  shufps      xmm2,xmm1,0AAh 
009A899F  movaps      xmm5,xmm1 
009A89A2  shufps      xmm5,xmm1,55h 
009A89A6  shufps      xmm0,xmm0,0C9h 
009A89AA  mulps       xmm2,xmm4 
009A89AD  addps       xmm2,xmm0 
009A89B0  movss       xmm0,dword ptr [__real@3f800000 (0AA73E0h)] 
009A89B8  lea         eax,[esp] 
009A89BB  mulps       xmm5,xmm4 
009A89BE  shufps      xmm3,xmm3,0D2h 
009A89C2  addps       xmm5,xmm3 
009A89C5  push        eax  
009A89C6  movaps      xmmword ptr [esp+14h],xmm5 
009A89CB  movaps      xmmword ptr [esp+24h],xmm2 
009A89D0  movss       dword ptr [esp+40h],xmm0 
009A89D6  call        dword ptr [__imp__glMultMatrixf@4 (0AA6B3Ch)] 
009A89DC  mov         esp,ebp 
009A89DE  pop         ebp  
009A89DF  ret              
*/
	}
}

void GLRenderHelpers::DrawSphere(float radius, const PR& pose)
{
//	return;
	glPushMatrix();
		SetupGLMatrix(pose);
		//glScalef(radius, radius, radius);
		//glutxSolidSphere(1.0f, 12, 12);
		glutxSolidSphere(radius, 12, 12);
	glPopMatrix();
}

void GLRenderHelpers::DrawSphereWireframe(float radius, const PR& pose, const Point& color)
{
	const udword NbSegments = 14;

	const Matrix3x3 M = pose.mRot;
	const Point R0 = M[0];
	const Point R1 = M[1];
	const Point R2 = M[2];

	Matrix4x4 Rot;
	Rot.SetTrans(pose.mPos);

	glPushMatrix();
	Rot.SetRow(0, R0);
	Rot.SetRow(1, R1);
	Rot.SetRow(2, R2);
	GLRenderHelpers::DrawCircle(NbSegments, Rot, color, radius);

	Rot.SetRow(0, R1);
	Rot.SetRow(1, R2);
	Rot.SetRow(2, R0);
	GLRenderHelpers::DrawCircle(NbSegments, Rot, color, radius);

	Rot.SetRow(0, R2);
	Rot.SetRow(1, R0);
	Rot.SetRow(2, R1);
	GLRenderHelpers::DrawCircle(NbSegments, Rot, color, radius);
	glPopMatrix();
}

void GLRenderHelpers::DrawBox(const Point& extents, const PR& pose)
{
//	return;
	glPushMatrix();
		SetupGLMatrix(pose);
		glScalef(extents.x, extents.y, extents.z);
		glutxSolidCube(2.0f);
	glPopMatrix();
}

static float gCylinderData[]={
	1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,
	0.866025f,0.500000f,1.0f,0.866025f,0.500000f,1.0f,0.866025f,0.500000f,0.0f,0.866025f,0.500000f,0.0f,
	0.500000f,0.866025f,1.0f,0.500000f,0.866025f,1.0f,0.500000f,0.866025f,0.0f,0.500000f,0.866025f,0.0f,
	-0.0f,1.0f,1.0f,-0.0f,1.0f,1.0f,-0.0f,1.0f,0.0f,-0.0f,1.0f,0.0f,
	-0.500000f,0.866025f,1.0f,-0.500000f,0.866025f,1.0f,-0.500000f,0.866025f,0.0f,-0.500000f,0.866025f,0.0f,
	-0.866025f,0.500000f,1.0f,-0.866025f,0.500000f,1.0f,-0.866025f,0.500000f,0.0f,-0.866025f,0.500000f,0.0f,
	-1.0f,-0.0f,1.0f,-1.0f,-0.0f,1.0f,-1.0f,-0.0f,0.0f,-1.0f,-0.0f,0.0f,
	-0.866025f,-0.500000f,1.0f,-0.866025f,-0.500000f,1.0f,-0.866025f,-0.500000f,0.0f,-0.866025f,-0.500000f,0.0f,
	-0.500000f,-0.866025f,1.0f,-0.500000f,-0.866025f,1.0f,-0.500000f,-0.866025f,0.0f,-0.500000f,-0.866025f,0.0f,
	0.0f,-1.0f,1.0f,0.0f,-1.0f,1.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,
	0.500000f,-0.866025f,1.0f,0.500000f,-0.866025f,1.0f,0.500000f,-0.866025f,0.0f,0.500000f,-0.866025f,0.0f,
	0.866026f,-0.500000f,1.0f,0.866026f,-0.500000f,1.0f,0.866026f,-0.500000f,0.0f,0.866026f,-0.500000f,0.0f,
	1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f
};

static float gCylinderDataCapsTop[]={
	0.866026f,-0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.500000f,-0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.500000f,-0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,-1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,-1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.500000f,-0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.500000f,-0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.866025f,-0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.866025f,-0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-1.000000f,-0.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-1.000000f,-0.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.866025f,0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.866025f,0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.500000f,0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.500000f,0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	-0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.500000f,0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.500000f,0.866025f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.866025f,0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.866025f,0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	1.000000f,0.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	1.000000f,0.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.000000f,1.000000f,1.000000f,0.000000f,1.000000f,1.000000f,
	0.866026f,-0.500000f,1.000000f,0.000000f,1.000000f,1.000000f,
};

static float gCylinderDataCapsBottom[]={
	1.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.866025f,0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.866025f,0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.500000f,0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.500000f,0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.000000f,1.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.000000f,1.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.500000f,0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.500000f,0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.866025f,0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.866025f,0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-1.000000f,-0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-1.000000f,-0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.866025f,-0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.866025f,-0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.500000f,-0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	-0.500000f,-0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,-1.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,-1.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.500000f,-0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.500000f,-0.866025f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.866026f,-0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.866026f,-0.500000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	0.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
	1.000000f,0.000000f,0.000000f,0.000000f,-1.000000f,0.000000f,
};

static void DrawCylinder(bool draw_caps)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gCylinderData);
    glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gCylinderData+3);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 13*2);

	if(draw_caps)
	{
		glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gCylinderDataCapsTop);
		glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gCylinderDataCapsTop+3);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glVertexPointer(3, GL_FLOAT, 2*3*sizeof(float), gCylinderDataCapsBottom);
		glNormalPointer(GL_FLOAT, 2*3*sizeof(float), gCylinderDataCapsBottom+3);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void DrawCylinderInternal()
{
	::DrawCylinder(true);
}

void GLRenderHelpers::DrawCapsule(float r, float h)
{
	glPushMatrix();
		glTranslatef(0.0f, h*0.5f, 0.0f);
		glScalef(r, r, r);
		glutxSolidSphere(1.0f, 12, 12);  // doesn't include texcoords
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0f, -h*0.5f, 0.0f);
		glScalef(r, r, r);
		glutxSolidSphere(1.0f, 12, 12);  // doesn't include texcoords
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0f, h*0.5f, 0.0f);
		glScalef(r, h, r);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		::DrawCylinder(false);
	glPopMatrix();
}

void GLRenderHelpers::DrawCapsule(float r, float h, const PR& pose)
{
/*	glPushMatrix();
		SetupGLMatrix(pose);

		unsigned num = 12;
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		gluSphere(GetGLUQuadric(), 
				  r, 
				  num, num);
		gluCylinder(GetGLUQuadric(), r, r, h, num, num);

		glTranslatef(0.0f, 0.0f, h);
		gluSphere(GetGLUQuadric(), 
				  r,
				  num, num);

	glPopMatrix();
	return;*/

	glPushMatrix();
		SetupGLMatrix(pose);
		GLRenderHelpers::DrawCapsule(r, h);
	glPopMatrix();
}

void GLRenderHelpers::DrawCapsuleWireframe(float r, float h, const PR& pose, const Point& color)
{
	const udword NbSegments = 14;

	const Matrix3x3 M = pose.mRot;

	Point p0, p1;

	p1 = M[1];
	p1 *= 0.5f*h;
	p0 = -p1;
	p0 += pose.mPos;
	p1 += pose.mPos;

	const Point c0 = M[0];
	const Point c1 = M[1];
	const Point c2 = M[2];
	GLRenderHelpers::DrawLine(p0 + c0*r, p1 + c0*r, color);
	GLRenderHelpers::DrawLine(p0 - c0*r, p1 - c0*r, color);
	GLRenderHelpers::DrawLine(p0 + c2*r, p1 + c2*r, color);
	GLRenderHelpers::DrawLine(p0 - c2*r, p1 - c2*r, color);

	Matrix4x4 MM;
	MM.SetRow(0, -c1);
	MM.SetRow(1, -c0);
	MM.SetRow(2, c2);
	MM.SetTrans(p0);
	GLRenderHelpers::DrawCircle(NbSegments, MM, color, r, true);	//halfcircle -- flipped

	MM.SetRow(0, c1);
	MM.SetRow(1, -c0);
	MM.SetRow(2, c2);
	MM.SetTrans(p1);
	GLRenderHelpers::DrawCircle(NbSegments, MM, color, r, true);

	MM.SetRow(0, -c1);
	MM.SetRow(1, c2);
	MM.SetRow(2, c0);
	MM.SetTrans(p0);
	GLRenderHelpers::DrawCircle(NbSegments, MM, color, r, true);//halfcircle -- good

	MM.SetRow(0, c1);
	MM.SetRow(1, c2);
	MM.SetRow(2, c0);
	MM.SetTrans(p1);
	GLRenderHelpers::DrawCircle(NbSegments, MM, color, r, true);

	MM.SetRow(0, c2);
	MM.SetRow(1, c0);
	MM.SetRow(2, c1);
	MM.SetTrans(p0);
	GLRenderHelpers::DrawCircle(NbSegments, MM, color, r);	//full circle
	MM.SetTrans(p1);
	GLRenderHelpers::DrawCircle(NbSegments, MM, color, r);
}

void GLRenderHelpers::DrawCylinder(float r, float h)
{
	glPushMatrix();
		glTranslatef(0.0f, h*0.5f, 0.0f);
		glScalef(r, h, r);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		::DrawCylinder(true);
	glPopMatrix();
}

void GLRenderHelpers::DrawCylinder(float r, float h, const PR& pose)
{
	glPushMatrix();
		SetupGLMatrix(pose);
		GLRenderHelpers::DrawCylinder(r, h);
	glPopMatrix();
}

void GLRenderHelpers::DrawBoxCorners(const AABB& bounds, const Point& color, float size)
{
	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);
	glEnableClientState(GL_VERTEX_ARRAY);

	Point Pts[8];
	bounds.ComputePoints(Pts);

	Point Verts[12*4];
	Point* v = Verts;

	const udword* Edges = bounds.GetEdges();
	for(udword i=0;i<12;i++)
	{
//		GLRenderHelpers::DrawLine(Pts[Edges[i*2]], Pts[Edges[i*2+1]], Color);

		// ICE style
		{
			const Point& p0 = Pts[Edges[i*2]];
			const Point& p1 = Pts[Edges[i*2+1]];
			const Point Delta = (p1-p0)*size;

			*v++ = p0;
			*v++ = p0 + Delta;
			*v++ = p1;
			*v++ = p1 - Delta;
		}
	}
	glVertexPointer(3, GL_FLOAT, sizeof(Point), &Verts[0].x);
	glDrawArrays(GL_LINES, 0, 2*12*2);

	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
}

#include "PEEL.h"
#include "Camera.h"

/*void GLRenderHelpers::DrawFrame(const Point& pt, float scale)
{
	if(UseFatFrames())
	{
		DrawFatFrame(pt, scale);
	}
	else
	{
		DrawLine(pt, pt + Point(scale, 0.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		DrawLine(pt, pt + Point(0.0f, scale, 0.0f), Point(0.0f, 1.0f, 0.0f));
		DrawLine(pt, pt + Point(0.0f, 0.0f, scale), Point(0.0f, 0.0f, 1.0f));
	}
}*/

void GLRenderHelpers::DrawFrame(const PR& pose, float scale, bool symmetricFrames)
{
	if(UseFatFrames())
	{

/*		glDepthFunc(GL_ALWAYS);
		const Point& pt = pose.mPos;
		const Matrix3x3 Rot = pose.mRot;
		const Point dx = Rot[0]*scale;
		const Point dy = Rot[1]*scale;
		const Point dz = Rot[2]*scale;
		if(symmetricFrames)
		{
			DrawLine(pt - dx, pt + dx, Point(1.0f, 0.0f, 0.0f));
			DrawLine(pt - dy, pt + dy, Point(0.0f, 1.0f, 0.0f));
			DrawLine(pt - dz, pt + dz, Point(0.0f, 0.0f, 1.0f));
		}
		else
		{
			DrawLine(pt, pt + dx, Point(1.0f, 0.0f, 0.0f));
			DrawLine(pt, pt + dy, Point(0.0f, 1.0f, 0.0f));
			DrawLine(pt, pt + dz, Point(0.0f, 0.0f, 1.0f));
		}
		glDepthFunc(GL_LESS);*/


		DrawFatFrame(pose, scale, symmetricFrames);
	}
	else
	{
		const Point& pt = pose.mPos;
		const Matrix3x3 Rot = pose.mRot;
		const Point dx = Rot[0]*scale;
		const Point dy = Rot[1]*scale;
		const Point dz = Rot[2]*scale;
		if(symmetricFrames)
		{
			DrawLine(pt - dx, pt + dx, Point(1.0f, 0.0f, 0.0f));
			DrawLine(pt - dy, pt + dy, Point(0.0f, 1.0f, 0.0f));
			DrawLine(pt - dz, pt + dz, Point(0.0f, 0.0f, 1.0f));
		}
		else
		{
			DrawLine(pt, pt + dx, Point(1.0f, 0.0f, 0.0f));
			DrawLine(pt, pt + dy, Point(0.0f, 1.0f, 0.0f));
			DrawLine(pt, pt + dz, Point(0.0f, 0.0f, 1.0f));
		}
	}
}

void GLRenderHelpers::DrawFrame(const Point& pt, float scale)
{
	PR Pose;
	Pose.mPos = pt;
	Pose.mRot.Identity();
	GLRenderHelpers::DrawFrame(Pose, scale, GetSymmetricFrames());
}


/*void GLRenderHelpers::DrawFatFrame(const Point& pos, float scale)
{
	PR pose;
	pose.Identity();
	pose.mPos = pos;

	Matrix3x3 Rot;

	const float r = 0.01f;
	const float h = 1.0f * scale;
	const float offset = scale * 0.5f;

	glPushMatrix();
		SetupGLMatrix(pose);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			glTranslatef(0.0f, h*0.5f+offset, 0.0f);
			glScalef(r, h, r);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			::DrawCylinder(true);
	glPopMatrix();

	glPushMatrix();
		Rot.RotZ(HALFPI);
		pose.mRot = Rot;
		SetupGLMatrix(pose);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			glTranslatef(0.0f, h*0.5f-offset, 0.0f);
			glScalef(r, h, r);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			::DrawCylinder(true);
	glPopMatrix();

	glPushMatrix();
		Rot.RotX(HALFPI);
		pose.mRot = Rot;
		SetupGLMatrix(pose);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
			glTranslatef(0.0f, h*0.5f+offset, 0.0f);
			glScalef(r, h, r);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			::DrawCylinder(true);
	glPopMatrix();
}*/

void RenderScaledCylinder(const PR& pose, const Point& trans, const Point& scale, const Point& color);

static const Point gRed(1.0f, 0.0f, 0.0f);
static const Point gGreen(0.0f, 1.0f, 0.0f);
static const Point gBlue(0.0f, 0.0f, 1.0f);

void GLRenderHelpers::DrawFatFrame(const PR& _pose, float scale, bool symmetricFrames)
{
	// ### slow test

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

		float Scale = ComputeConstantScale(_pose.mPos, ViewMatrix, ProjMatrix);



	PR pose = _pose;

	const float r = 0.01f * Scale * 300.0f;
	const float h = 1.0f * scale;
	const float coeff = symmetricFrames ? 2.0f : 1.0f;
	const float offset = scale * 0.5f;
	const Point gScale(r, h*coeff, r);

	RenderScaledCylinder(pose, Point(0.0f, h*0.5f+offset, 0.0f), gScale, gGreen);

	{
		Matrix3x3 Rot;
		Rot.RotZ(HALFPI);
		pose.mRot = _pose.mRot * Rot;
		if(symmetricFrames)
			RenderScaledCylinder(pose, Point(0.0f, h*0.5f+offset, 0.0f), gScale, gRed);
		else
			RenderScaledCylinder(pose, Point(0.0f, h*0.5f-offset, 0.0f), gScale, gRed);
	}

	{
		Matrix3x3 Rot;
		Rot.RotX(HALFPI);
		pose.mRot = _pose.mRot * Rot;
		RenderScaledCylinder(pose, Point(0.0f, h*0.5f+offset, 0.0f), gScale, gBlue);
	}
}

void GLRenderHelpers::DrawFatFrame(const Point& pt, float scale)
{
	PR Pose;
	Pose.mPos = pt;
	Pose.mRot.Identity();
	GLRenderHelpers::DrawFatFrame(Pose, scale, GetSymmetricFrames());
}







/////

static inline_ void ComputePoint(Point& dest, float x, float y, const Matrix4x4& rot, const Point& trans)
{
	dest.x = trans.x + x * rot.m[0][0] + y * rot.m[1][0];
	dest.y = trans.y + x * rot.m[0][1] + y * rot.m[1][1];
	dest.z = trans.z + x * rot.m[0][2] + y * rot.m[1][2];
}

static float vline_texCoord[8][3]={{1.0f, 0.0f, 0.0f},
							{1.0f, 1.0f, 0.0f},
							{0.5f, 0.0f, 0.0f},
							{0.5f, 1.0f, 0.0f},
							{0.5f, 0.0f, 0.0f},
							{0.5f, 1.0f, 0.0f},
							{0.0f, 0.0f, 0.0f},
							{0.0f, 1.0f, 0.0f}
};

static float vline_vertexOffset[8][2]={{ 1.0f,	 1.0f},
								{ 1.0f,	-1.0f},
								{ 0.0f,	 1.0f},
								{ 0.0f,	-1.0f},
								{ 0.0f,	-1.0f},
								{ 0.0f,	 1.0f},
								{ 1.0f,	-1.0f},
								{ 1.0f,	 1.0f}
};

static void ComputeVolumePoint2(PDT_Vertex& dest, const Matrix4x4& view_proj, const Matrix4x4& inv_view, const float* normal, const float* uvs, const Point& o, const Point& p, float size, udword color)
{
	dest.u		= uvs[0];
	dest.v		= uvs[1];
	dest.color	= color;

	const HPoint vMVP		= HPoint(p.x, p.y, p.z, 1.0f) * view_proj;
	const HPoint otherMVP	= HPoint(o.x, o.y, o.z, 1.0f) * view_proj;

	const float OneOverMVP = 1.0f / vMVP.w;
	const float OneOverOtherMVP = 1.0f / otherMVP.w;
	float xw = (vMVP.x*OneOverMVP) - (otherMVP.x*OneOverOtherMVP);
	float yw = (vMVP.y*OneOverMVP) - (otherMVP.y*OneOverOtherMVP);
	const float Norm2D = sqrtf(xw*xw + yw*yw);
	xw /= Norm2D;
	yw /= Norm2D;
	xw *= size;
	yw *= size;

	if((otherMVP.w * vMVP.w)< 0.0f)
	{
		xw = -xw;
		yw = -yw;
	}
	ComputePoint(dest.p, xw*normal[0] + yw*normal[1], yw*normal[0] - xw*normal[1], inv_view, p);
}

void RenderVolumeLine(/*RMScene* scene,*/
//		const Matrix4x4& view,
//		const Matrix4x4& proj,
//		const Matrix4x4& inv_view,
//		const Matrix4x4& view_proj,
					  const Point& p1, const Point& p2, float size, udword color)
{
		// Catch camera
//		RMCamera* Cam = scene->GetCurrentCamera();
//		if(!Cam)	return;

		// Catch relevant matrices
//		const ViewMatrix& view = Cam->GetViewMatrix();
//		const ProjMatrix& proj = Cam->GetProjMatrix();
//		const Matrix4x4& inv_view = Cam->GetInverseViewMatrix();
//		Matrix4x4 view_proj = view*proj;

		Matrix4x4 view;
		GetModelViewMatrix(view);

		Matrix4x4 proj;
		GetProjMatrix(proj);

		//const Matrix4x4& inv_view = Cam->GetInverseViewMatrix();
		Matrix4x4 inv_view;
		InvertPRMatrix(inv_view, view);

		Matrix4x4 view_proj = view*proj;



	// 8 vertices for each "line"
	PDT_Vertex Vertices[8];
	ComputeVolumePoint2(Vertices[0], view_proj, inv_view, vline_vertexOffset[0], vline_texCoord[0], p2, p1, size, color);
	ComputeVolumePoint2(Vertices[1], view_proj, inv_view, vline_vertexOffset[1], vline_texCoord[1], p2, p1, size, color);
	ComputeVolumePoint2(Vertices[2], view_proj, inv_view, vline_vertexOffset[2], vline_texCoord[2], p2, p1, size, color);
	ComputeVolumePoint2(Vertices[3], view_proj, inv_view, vline_vertexOffset[3], vline_texCoord[3], p2, p1, size, color);
	ComputeVolumePoint2(Vertices[4], view_proj, inv_view, vline_vertexOffset[4], vline_texCoord[4], p1, p2, size, color);
	ComputeVolumePoint2(Vertices[5], view_proj, inv_view, vline_vertexOffset[5], vline_texCoord[5], p1, p2, size, color);
	ComputeVolumePoint2(Vertices[6], view_proj, inv_view, vline_vertexOffset[6], vline_texCoord[6], p1, p2, size, color);
	ComputeVolumePoint2(Vertices[7], view_proj, inv_view, vline_vertexOffset[7], vline_texCoord[7], p1, p2, size, color);

//	scene->mRS->SetWorldIdentity();
//	scene->mRenderer->DrawPrimitive(PRIMTYPE_TRISTRIP, VF_PDT, Vertices, 8);


	glDisable(GL_LIGHTING);
	//glColor4f(color.x, color.y, color.z, 1.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	//glVertexPointer(3, GL_FLOAT, sizeof(Point), &pts->x);
	glVertexPointer(3, GL_FLOAT, sizeof(PDT_Vertex), &Vertices[0].p.x);
	//glDrawArrays(GL_LINES, 0, 2*nb_lines);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
	glDisableClientState(GL_VERTEX_ARRAY);
	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);


}

/////
