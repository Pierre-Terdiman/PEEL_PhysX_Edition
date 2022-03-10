///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_DL_SHAPE_RENDERER_H
#define PINT_DL_SHAPE_RENDERER_H

#include "PintShapeRenderer.h"

	class PintDLShapeRenderer : public PintShapeRenderer
	{
//				Matrix4x4	mCached;
//				PR			mLastPR;
		public:
							PintDLShapeRenderer();
		virtual				~PintDLShapeRenderer();

		// PintShapeRenderer
		virtual	const char*	GetClassName()			const	override	{ return "PintDLShapeRenderer";	}
		virtual	void		_Render(const PR& pose)	const	override;
		//~PintShapeRenderer

		protected:
				GLuint		mDisplayListNum;
//				udword		mNbVerts;
	};

#endif