///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Common.h"
#include "Camera.h"
#include "GLRenderHelpers.h"

static	Point		gEye(50.0f, 50.0f, 50.0f);
static	Point		gDir(-0.6f, -0.2f, -0.7f);
static	Point		gViewY(0.0f, 0.0f, 0.0f);
static	float		gFOV = 60.0f;
static	float		gSensititivy = 10.0f;
static	float		gNearClip = 0.0f;
static	float		gFarClip = 0.0f;
static	float		gCameraSpeed = 0.0f;
static	Frustum		gFrustum;
		bool		gInvalidPlanes = true;
		bool		gFreezeFrustum = false;

void GetModelViewMatrix(Matrix4x4& mat)
{
	float M[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, M);
	mat.Set(M);
}

void GetProjMatrix(Matrix4x4& mat)
{
	float M[16];
	glGetFloatv(GL_PROJECTION_MATRIX, M);
	mat.Set(M);
}

static void BuildFrustumPlanes()
{
	Matrix4x4 ViewMatrix;
	Matrix4x4 ProjMatrix;

	float VM[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, VM);
	ViewMatrix.Set(VM);

	float PM[16];
	glGetFloatv(GL_PROJECTION_MATRIX, PM);
	ProjMatrix.Set(PM);

	const Matrix4x4 Combo = ViewMatrix * ProjMatrix;

	// PT: TODO: check parameters here
	if(!gFreezeFrustum)
		gFrustum.ExtractPlanes(Combo, true, true);

	gInvalidPlanes = false;
}

void SetCamera(const Point& pos, const Point& dir)
{
	gEye = pos;
	gDir = dir;
}

Point GetCameraPos()
{
	return gEye;
}

Point GetCameraDir()
{
	return gDir;
}

void ResetCamera()
{
	gEye = Point(50.0f, 50.0f, 50.0f);
	gDir = Point(-0.6f, -0.2f, -0.7f);
}

void	SetCameraSpeed(float speed)		{ gCameraSpeed = speed;	}
float	GetCameraSpeed()				{ return gCameraSpeed;	}
void	SetCameraFOV(float fov)			{ gFOV = fov;			}
float	GetCameraFOV()					{ return gFOV;			}
void	SetCameraNearClip(float clip)	{ gNearClip = clip;		}
float	GetCameraNearClip()				{ return gNearClip;		}
void	SetCameraFarClip(float clip)	{ gFarClip = clip;		}
float	GetCameraFarClip()				{ return gFarClip;		}
void	SetCameraSensitivity(float s)	{ gSensititivy = s;		}
float	GetCameraSensitivity()			{ return gSensititivy;	}

void MoveCameraForward(float dt)
{
	gEye += gDir * gCameraSpeed * dt;
}

void MoveCameraBackward(float dt)
{
	gEye -= gDir * gCameraSpeed * dt;
}

void MoveCameraRight(float dt)
{
	gEye -= gViewY * gCameraSpeed * dt;
}

void MoveCameraLeft(float dt)
{
	gEye += gViewY * gCameraSpeed * dt;
}

class MyQuat
{
	public:

	inline_	MyQuat()													{}
	inline_	MyQuat(const MyQuat& q) : x(q.x), y(q.y), z(q.z), w(q.w)	{}
	inline_	MyQuat(const Point& v, float s)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = s;
	}

	inline_ float magnitudeSquared() const
	{
		return x*x + y*y + z*z + w*w;
	}

	inline_ float dot(const MyQuat& v) const
	{
		return x * v.x + y * v.y + z * v.z  + w * v.w;
	}

	inline_ float magnitude() const
	{
		return sqrtf(magnitudeSquared());
	}

	inline_ float normalize()
	{
		const float mag = magnitude();
		if(mag)
		{
			const float imag = float(1) / mag;

			x *= imag;
			y *= imag;
			z *= imag;
			w *= imag;
		}
		return mag;
	}

	inline_	void	fromAngleAxis(float Angle, const Point& axis)			// set the Quat by Angle-axis (see AA constructor)
	{
		x = axis.x;
		y = axis.y;
		z = axis.z;

		// required: Normalize the axis

		const float i_length =  float(1.0) / sqrtf( x*x + y*y + z*z );
		
		x = x * i_length;
		y = y * i_length;
		z = z * i_length;

		// now make a clQuaternionernion out of it
		float Half = degToRad(Angle * float(0.5));

		w = cosf(Half);//this used to be w/o deg to rad.
		const float sin_theta_over_two = sinf(Half );
		x = x * sin_theta_over_two;
		y = y * sin_theta_over_two;
		z = z * sin_theta_over_two;
	}

	inline_	MyQuat(const float angle, const Point& axis)				// creates a Quat from an Angle axis -- note that if Angle > 360 the resulting rotation is Angle mod 360
	{
		fromAngleAxis(angle,axis);
	}

	inline_ void	multiply(const MyQuat& left, const Point& right)		// this = a * b
	{
		float a,b,c,d;

		a = - left.x*right.x - left.y*right.y - left.z *right.z;
		b =   left.w*right.x + left.y*right.z - right.y*left.z;
		c =   left.w*right.y + left.z*right.x - right.z*left.x;
		d =   left.w*right.z + left.x*right.y - right.x*left.y;

		w = a;
		x = b;
		y = c;
		z = d;
	}

	inline_	void	rotate(Point& v) const						//rotates passed vec by rot expressed by quaternion.  overwrites arg ith the result.
	{
		//float msq = float(1.0)/magnitudeSquared();	//assume unit quat!
		MyQuat myInverse;
		myInverse.x = -x;//*msq;
		myInverse.y = -y;//*msq;
		myInverse.z = -z;//*msq;
		myInverse.w =  w;//*msq;

		//v = ((*this) * v) ^ myInverse;

		MyQuat left;
		left.multiply(*this,v);
		v.x =left.w*myInverse.x + myInverse.w*left.x + left.y*myInverse.z - myInverse.y*left.z;
		v.y =left.w*myInverse.y + myInverse.w*left.y + left.z*myInverse.x - myInverse.z*left.x;
		v.z =left.w*myInverse.z + myInverse.w*left.z + left.x*myInverse.y - myInverse.x*left.y;
	}

    float x,y,z,w;
};

void RotateCamera(int dx, int dy)
{
	const Point Up(0.0f, 1.0f, 0.0f);
	gDir.Normalize();
	gViewY = gDir^Up;

    const float Sensitivity = PI * gSensititivy / 180.0f;
	// #### TODO: replicate this using Ice quats
	const MyQuat qx(dx * Sensitivity, Up);
	qx.rotate(gDir);
	const MyQuat qy(dy * Sensitivity, gViewY);
	qy.rotate(gDir);
}

extern udword gScreenWidth;
extern udword gScreenHeight;

void SetupProjectionMatrix()
{
//	z_far = 20.0f;

	// This is not needed for the projection matrix but we need to do that once per frame so that the left/right "move"
	// commands work. So we do that here, just because we know it's called at least once per frame.
	{
		const Point Up(0.0f, 1.0f, 0.0f);
		gDir.Normalize();
		gViewY = gDir^Up;
	}

//	const float Width	= float(gScreenWidth);
//	const float Height	= float(gScreenHeight);

	glMatrixMode(GL_PROJECTION);
/*	glLoadIdentity();
	gluPerspective(gFOV, Width/Height, gNearClip, gFarClip);
//	gluPerspective(gFOV, 1.0f, z_near, z_far);
//	gluPerspective(gFOV, ((float)glutxGet(GLUT_WINDOW_WIDTH))/((float)glutxGet(GLUT_WINDOW_HEIGHT)), z_near, z_far);
*/
	{
		const float nearZ = gNearClip;
		const float farZ = gFarClip;
		const float frange = farZ / (nearZ - farZ);

		const float fov = 0.5f * degToRad(gFOV);
		const float tanFov = tanf(fov);

		const float invAspectRatio = float(gScreenHeight) / float(gScreenWidth);
		const float height = 1.0f / tanFov;
		const float width = height * invAspectRatio;

		float projMat[16];
		projMat[0] = width;
		projMat[1] = 0.0f;
		projMat[2] = 0.0f;
		projMat[3] = 0.0f;

		projMat[4] = 0.0f;
		projMat[5] = height;
		projMat[6] = 0.0f;
		projMat[7] = 0.0f;

		projMat[8] = 0.0f;
		projMat[9] = 0.0f;
		projMat[10] = frange;
		projMat[11] = -1.0f;

		projMat[12] = 0.0f;
		projMat[13] = 0.0f;
//		projMat[14] = nearZ == INFINITY ? farZ : frange * nearZ;
		projMat[14] = frange * nearZ;
		projMat[15] = 0.0f;
		glLoadMatrixf(projMat);
	}


	gInvalidPlanes = true;
}




// PT: TODO: use the ICE version
static void ComputeViewVectors(Point& right, Point& up)
{
	if(fabsf(gDir.y)>0.9999f)
	{
		right = Point(1.0f, 0.0f, 0.0f);
	}
	else
	{
		right = Point(0.0f, 1.0f, 0.0f)^gDir;
		right.Normalize();
	}

	up = gDir^right;
	up.Normalize();
}

void BuildFrustumPts(Point* frustum)
{
	int WindowWidth, WindowHeight;
	glutxGetWindowSize(WindowWidth, WindowHeight);

	const float Ratio = float(WindowWidth)/float(WindowHeight);
	const float Tan = tanf(0.5f * DEGTORAD * gFOV) / Ratio;

	const float NearCoeff	= gNearClip * Tan;
	const float FarCoeff	= gFarClip * Tan;

	const float RightCoeff = Ratio;
	const float UpCoeff	= 1.0f;

	const Point Forward	= gDir;
		Point Right, Up;
		ComputeViewVectors(Right, Up);

	frustum[0] = gEye + Forward*gNearClip - Right*NearCoeff*RightCoeff + Up*NearCoeff*UpCoeff;
	frustum[1] = gEye + Forward*gNearClip - Right*NearCoeff*RightCoeff - Up*NearCoeff*UpCoeff;
	frustum[2] = gEye + Forward*gNearClip + Right*NearCoeff*RightCoeff - Up*NearCoeff*UpCoeff;
	frustum[3] = gEye + Forward*gNearClip + Right*NearCoeff*RightCoeff + Up*NearCoeff*UpCoeff;

	frustum[4] = gEye + Forward*gFarClip - Right*FarCoeff*RightCoeff + Up*FarCoeff*UpCoeff;
	frustum[5] = gEye + Forward*gFarClip - Right*FarCoeff*RightCoeff - Up*FarCoeff*UpCoeff;
	frustum[6] = gEye + Forward*gFarClip + Right*FarCoeff*RightCoeff - Up*FarCoeff*UpCoeff;
	frustum[7] = gEye + Forward*gFarClip + Right*FarCoeff*RightCoeff + Up*FarCoeff*UpCoeff;
}

static Point gCapturedFrustum[8];
void CaptureFrustum()
{
	BuildFrustumPts(gCapturedFrustum);
}

void DrawCapturedFrustum()
{
	const Point* pts = gCapturedFrustum;
	const Point color(1.0f, 1.0f, 1.0f);
	GLRenderHelpers::DrawLine(pts[0], pts[1], color);
	GLRenderHelpers::DrawLine(pts[1], pts[2], color);
	GLRenderHelpers::DrawLine(pts[2], pts[3], color);
	GLRenderHelpers::DrawLine(pts[3], pts[0], color);
	GLRenderHelpers::DrawLine(pts[4], pts[5], color);
	GLRenderHelpers::DrawLine(pts[5], pts[6], color);
	GLRenderHelpers::DrawLine(pts[6], pts[7], color);
	GLRenderHelpers::DrawLine(pts[7], pts[4], color);
	GLRenderHelpers::DrawLine(pts[0], pts[4], color);
	GLRenderHelpers::DrawLine(pts[1], pts[5], color);
	GLRenderHelpers::DrawLine(pts[2], pts[6], color);
	GLRenderHelpers::DrawLine(pts[3], pts[7], color);
}

const Plane* GetFrustumPlanes()
{
	if(gInvalidPlanes)
		BuildFrustumPlanes();

	return gFrustum.GetPlanes();
}

#include "PxTransform.h"
using namespace physx;
bool gMirror = false;
static void lookAt()
{
//	gluLookAt(gEye.x, gEye.y, gEye.z, gEye.x + gDir.x, gEye.y + gDir.y, gEye.z + gDir.z, 0.0f, 1.0f, 0.0f);

	Point right, up;
	ComputeViewVectors(right, up);

	float viewMat[16];
	viewMat[0] = -right.x;
	viewMat[1] = up.x;
	viewMat[2] = -gDir.x;
	viewMat[3] = 0.0f;

	viewMat[4] = -right.y;
	viewMat[5] = up.y;
	viewMat[6] = -gDir.y;
	viewMat[7] = 0.0f;

	viewMat[8] = -right.z;
	viewMat[9] = up.z;
	viewMat[10] = -gDir.z;
	viewMat[11] = 0.0f;

	viewMat[12] = gEye|right;
	viewMat[13] = -gEye|up;
	viewMat[14] = gEye|gDir;
	viewMat[15] = 1.0f;
//	glMultMatrixf(viewMat);
	glLoadMatrixf(viewMat);

	if(gMirror)
	{

static float	gGroundY = 0.0f;
		PxTransform trans;

		PxVec3 planeN(0.0f, 1.0f, 0.0f);
		PxVec3 planeP(0.0f, gGroundY, 0.0f);
		float np2 = 2.0f*planeN.dot(planeP);

		PxVec3 r0 = PxVec3(1.0f, 0.0f, 0.0f) - 2 * planeN.x*planeN;
		PxVec3 r1 = PxVec3(0.0f, 1.0f, 0.0f) - 2 * planeN.y*planeN;
		PxVec3 r2 = PxVec3(0.0f, 0.0f, 1.0f) - 2 * planeN.z*planeN;
		PxVec3 t = np2*planeN;

		float matGL[16] = {
			r0.x, r1.x, r2.x, 0.0f,
			r0.y, r1.y, r2.y, 0.0f,
			r0.z, r1.z, r2.z, 0.0f,
			t.x, t.y, t.z, 1.0f };

		glMultMatrixf(matGL);
	}

}

void SetupModelViewMatrix()
{
	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	gluLookAt(gEye.x, gEye.y, gEye.z, gEye.x + gDir.x, gEye.y + gDir.y, gEye.z + gDir.z, 0.0f, 1.0f, 0.0f);
	lookAt();

	gInvalidPlanes = true;
}

/*void SetupCameraMatrix(float z_near, float z_far)
{
	const Point Up(0.0f, 1.0f, 0.0f);
	gDir.Normalize();
	gViewY = gDir^Up;

	const float Width	= float(gScreenWidth);
	const float Height	= float(gScreenHeight);

	glLoadIdentity();
	gluPerspective(gFOV, Width/Height, z_near, z_far);
//	gluPerspective(gFOV, 1.0f, z_near, z_far);
//	gluPerspective(gFOV, ((float)glutxGet(GLUT_WINDOW_WIDTH))/((float)glutxGet(GLUT_WINDOW_HEIGHT)), z_near, z_far);
//	gluLookAt(gEye.x, gEye.y, gEye.z, gEye.x + gDir.x, gEye.y + gDir.y, gEye.z + gDir.z, 0.0f, 1.0f, 0.0f);
	lookAt();
}*/


WorldRayComputer::WorldRayComputer()
{
	mWidth	= float(gScreenWidth);
	mHeight	= float(gScreenHeight);

	mHTanCoeff = tanf(0.25f * fabsf(DEGTORAD * gFOV * 2.0f));
	mVTanCoeff = mHTanCoeff*(mWidth/mHeight);

	mHTanCoeff *= 2.0f / mWidth;
	mVTanCoeff *= 2.0f / mHeight;

	mWidth *= 0.5f;
	mHeight *= 0.5f;

	Point Right, Up;
	ComputeBasis(gDir, Right, Up);

	mInvView.SetCol(0, -Right);
	mInvView.SetCol(1, Up);
	mInvView.SetCol(2, gDir);
}



#define NEW_VERSION
#ifdef NEW_VERSION
// Fetched from the old ICE renderer. More accurate and MUCH faster than the previous crazy gluUnProject-based version...
Point ComputeWorldRay(int xs, int ys)
{
	// Catch width & height
	const float Width	= float(gScreenWidth);
	const float Height	= float(gScreenHeight);

	// Recenter coordinates in camera space ([-1, 1])
	const float u = ((xs - Width*0.5f)/Width)*2.0f;
	const float v = -((ys - Height*0.5f)/Height)*2.0f;

	// Adjust coordinates according to camera aspect ratio
	const float HTan = tanf(0.25f * fabsf(DEGTORAD * gFOV * 2.0f));
	const float VTan = HTan*(Width/Height);

	// Ray in camera space
	const Point CamRay(VTan*u, HTan*v, 1.0f);

	// Compute ray in world space
	Point Right, Up;
	ComputeBasis(gDir, Right, Up);

	Matrix3x3 InvView;
	InvView.SetCol(0, -Right);
	InvView.SetCol(1, Up);
	InvView.SetCol(2, gDir);

	return (InvView * CamRay).Normalize();
}
#else
Point ComputeWorldRay(int xs, int ys)
{
	GLint viewPort[4];
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	ys = viewPort[3] - ys - 1;
	GLdouble wx0, wy0, wz0;
	gluUnProject((GLdouble) xs, (GLdouble) ys, 0.0, modelMatrix, projMatrix, viewPort, &wx0, &wy0, &wz0);
	GLdouble wx1, wy1, wz1;
	int ret = gluUnProject((GLdouble) xs, (GLdouble) ys, 1.0, modelMatrix, projMatrix, viewPort, &wx1, &wy1, &wz1);
	if(!ret)
		printf("gluUnProject failed\n");
	Point tmp(float(wx1-wx0), float(wy1-wy0), float(wz1-wz0));
	tmp.Normalize();

	if(tmp.Dot(gDir)<0.0f)
		tmp = -tmp;

	return tmp;
}
#endif
