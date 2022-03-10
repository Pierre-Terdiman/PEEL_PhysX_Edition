///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RevolutionShape.h"
#include "Loader_ShapeProfile.h"
#include "PintShapeRenderer.h"

static const float gScale		= 0.01f;
static const float gBrickHeight	= 9.6f * gScale;
static const float gBrickLength	= 8.0f * gScale;

RevolutionShape::RevolutionShape() : mNbSlices(0), mVertsCopy(null), mShapes(null)
{
}

RevolutionShape::~RevolutionShape()
{
	for(udword s=0;s<mNbSlices;s++)
	{
		DELETESINGLE(mShapes[s]);
		DELETEARRAY(mVertsCopy[s]);
	}
	DELETEARRAY(mShapes);
	DELETEARRAY(mVertsCopy);
}

bool RevolutionShape::Init(const char* filename, const Point& offset, float scale)
{
	//###TODO:
	// - check that the outline is convex - we create *convex* parts
	//- reorient poly in CW or CCW => refactor shape loading code

	// Load shape - to refactor/share later
	udword NbShapeVerts = 0;
	Point* ShapeVerts = null;
	{
		FILE* fp = fopen(filename, "rb");
		if(!fp)
			return false;

		struct FileHeader
		{
			udword	mIdentifier;
			udword	mVersion;
			sdword	mGridSpacing;
		};

		// 
		FileHeader Header;
		fread(&Header, 1, sizeof(Header), fp);
		ASSERT(Header.mIdentifier == 'SHAP');
		ASSERT(Header.mVersion == 1);
		const sdword gGridSpacingY = Header.mGridSpacing;
		//const sdword gGridSpacingX = gGridSpacingY*3;

		// 
		udword NbShapes;
		fread(&NbShapes, 1, sizeof(udword), fp);
		ASSERT(NbShapes==1);

		for(udword i=0;i<NbShapes;i++)
		{
			udword NbPts;
			fread(&NbPts, 1, sizeof(udword), fp);

			ShapeProfile S(NbPts, null);
			fread(S.mPts, 1, sizeof(Pt)*NbPts, fp);
			fread(S.mSegments, 1, sizeof(Seg)*NbPts, fp);

			{
				sdword MinX = MAX_SDWORD;
				sdword MaxX = MIN_SDWORD;
				sdword MinY = MAX_SDWORD;
				sdword MaxY = MIN_SDWORD;
				for(udword j=0;j<NbPts;j++)
				{
					if(S.mPts[j].mX<MinX)
						MinX = S.mPts[j].mX;
					if(S.mPts[j].mX>MaxX)
						MaxX = S.mPts[j].mX;
					if(S.mPts[j].mY<MinY)
						MinY = S.mPts[j].mY;
					if(S.mPts[j].mY>MaxY)
						MaxY = S.mPts[j].mY;
				}
				for(udword j=0;j<NbPts;j++)
				{
					S.mPts[j].mY = -S.mPts[j].mY;
				}
			}

			if(1)
			{
	//			const udword NbPts = S.mNbPts;
				NbShapeVerts = NbPts;
				ShapeVerts = ICE_NEW(Point)[NbShapeVerts];

				float MinX = MAX_FLOAT;
				float MaxX = MIN_FLOAT;
				float MinY = MAX_FLOAT;
				float MaxY = MIN_FLOAT;

				const float ScaleX = gBrickLength/(3.0f*float(gGridSpacingY));
				const float ScaleY = gBrickHeight/(3.0f*float(gGridSpacingY));
				for(udword j=0;j<NbPts;j++)
				{
					const Pt& p0 = S.mPts[j];

					ShapeVerts[j] = Point(float(p0.mX)*ScaleX, float(p0.mY)*ScaleY, 0.0f);

					if(ShapeVerts[j].x<MinX)
						MinX = ShapeVerts[j].x;
					if(ShapeVerts[j].x>MaxX)
						MaxX = ShapeVerts[j].x;
					if(ShapeVerts[j].y<MinY)
						MinY = ShapeVerts[j].y;
					if(ShapeVerts[j].y>MaxY)
						MaxY = ShapeVerts[j].y;
				}

				for(udword j=0;j<NbPts;j++)
				{
					ShapeVerts[j].x -= (MinX + MaxX)*0.5f;
					ShapeVerts[j].y -= (MinY + MaxY)*0.5f;

					const float x = ShapeVerts[j].x;
					const float y = ShapeVerts[j].y;
					ShapeVerts[j].x = 0.0f;
					ShapeVerts[j].y = x;
					ShapeVerts[j].z = y;
				}
			}
		}
		fclose(fp);
	}

	if(0)
		return false;

	const Quat torus_rot(Idt);

	// This version is a bit silly and super expensive for a wheel arch but we can always revisit this later.
	// Based on GenerateTireCompound() and maybe we could refactor these two.

	// Generate torus. We generate a torus by sweeping a small circle along a large circle, taking N samples (slices) along the way.
	const float BigRadius = 2.0f * 0.25f;// * scale;
	const udword NbSlices = 32;
//	const udword NbSlices = 4;

	// First we generate a small vertical template circle
	const udword NbPtsSmallCircle = NbShapeVerts;
	const Point* SmallCirclePts = ShapeVerts;

	// We'll be sweeping this initial circle along a curve (a larger circle), taking N slices along the way.
	// The final torus will use these vertices exclusively so the total #verts is:
	const udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
	Point* Verts = ICE_NEW(Point)[TotalNbVerts];

	// Now we do the sweep along the larger circle.
	Point SliceCenters[NbSlices];
	{
		const Matrix3x3 TRot = torus_rot;

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
//			const float Coeff = float(j)/float(NbSlices-1);	//###modif
			const float Coeff = float(j)/float(NbSlices);

			// We rotate and translate the template circle to position it along the larger circle.
			Matrix3x3 Rot;
			Rot.RotZ(Coeff * TWOPI);
//			Rot.RotZ(Coeff * PI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = (Trans + SmallCirclePts[i]*Rot)*TRot;

			SliceCenters[j] = Trans*TRot;
		}
		ASSERT(Index==TotalNbVerts);
	}
	// Here we have generated all the vertices.

	// Next, we generate a convex object for each part of the torus. A part is a section connecting two of the previous slices.
	mNbSlices = NbSlices/2;
	mVertsCopy = new Point*[mNbSlices];
	mShapes = new PINT_CONVEX_CREATE*[mNbSlices];

	const udword SliceOffset = 24;	//###TODO: expose this
	for(udword s=0;s<mNbSlices;s++)
	{
		const udword SliceIndex0 = (s+SliceOffset)%NbSlices;
		const udword SliceIndex1 = (s+SliceOffset+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		
		// We copy the vertices because we want to recenter them for each part.
		Point* ConvexPts = ICE_NEW(Point)[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i] * scale;
			ConvexPts[i+NbPtsSmallCircle] = V1[i] * scale;
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}

		// Now we create the convex object itself
		mVertsCopy[s] = ConvexPts;

		PINT_CONVEX_CREATE* ConvexCreate = ICE_NEW(PINT_CONVEX_CREATE)(NbPtsSmallCircle*2, ConvexPts);
		mShapes[s] = ConvexCreate;
		ConvexCreate->mRenderer	= CreateConvexRenderer(ConvexCreate->mNbVerts, ConvexPts);
		ConvexCreate->mLocalPos	= offset + Center;
	}

	DELETEARRAY(Verts);
	DELETEARRAY(ShapeVerts);

	return true;
}
