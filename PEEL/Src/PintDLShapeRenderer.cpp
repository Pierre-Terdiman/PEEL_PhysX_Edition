///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintDLShapeRenderer.h"
#include "GLRenderHelpers.h"

//udword gNbRenderedVerts = 0;

PintDLShapeRenderer::PintDLShapeRenderer()
{
	mDisplayListNum = glGenLists(1);
//	mLastPR.mPos.SetNotUsed();
//	mLastPR.mRot.SetNotUsed();
//	mNbVerts = 0;
}

PintDLShapeRenderer::~PintDLShapeRenderer()
{
	glDeleteLists(mDisplayListNum, 1);
}

void PintDLShapeRenderer::_Render(const PR& pose) const
{
//	gNbRenderedVerts += mNbVerts;
//	return;

	glPushMatrix();
	{
/*		if(0)
		{
			if(pose!=mLastPR)
			{
				mLastPR = pose;
				mCached = pose;
			}

			{
				const Matrix4x4& M = mCached;

				float glmat[16];	//4x4 column major matrix for OpenGL.
				glmat[0] = M.m[0][0];
				glmat[1] = M.m[0][1];
				glmat[2] = M.m[0][2];
				glmat[3] = M.m[0][3];

				glmat[4] = M.m[1][0];
				glmat[5] = M.m[1][1];
				glmat[6] = M.m[1][2];
				glmat[7] = M.m[1][3];

				glmat[8] = M.m[2][0];
				glmat[9] = M.m[2][1];
				glmat[10] = M.m[2][2];
				glmat[11] = M.m[2][3];

				glmat[12] = M.m[3][0];
				glmat[13] = M.m[3][1];
				glmat[14] = M.m[3][2];
				glmat[15] = M.m[3][3];

				glMultMatrixf(&(glmat[0]));
			//	glLoadMatrixf(&(glmat[0]));
			}
		}
		else*/
		{
			GLRenderHelpers::SetupGLMatrix(pose);
		}

		//glScalef(0.2f, 2.0f, 0.2f);

//		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
//		glColor4f(mColor.x, mColor.y, mColor.z, 1.0f);
//		glColor4f(UnitRandomFloat(), UnitRandomFloat(), UnitRandomFloat(), 1.0f);
		glCallList(mDisplayListNum);
	}
	glPopMatrix();

/*	if(0 && mShadows)
	{
		glPushMatrix();
		{
			const static float shadowMat[]={ 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
			glMultMatrixf(shadowMat);
//				glMultMatrixf(glMat);
			SetupGLMatrix(pose);
			glDisable(GL_LIGHTING);
			glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
			//glutxSolidCube(float(size_t(actor->userData))*2.0f);
			glCallList(mDisplayListNum);
			glEnable(GL_LIGHTING);
		}
		glPopMatrix();
	}*/
}
