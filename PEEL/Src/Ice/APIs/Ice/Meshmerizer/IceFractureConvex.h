///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Fracture code.
 *	\file		IceDelaunay3d.h
 *	\author		Pierre Terdiman
 *	\date		October, 31, 2018
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFRACTURECONVEX_H
#define ICEFRACTURECONVEX_H

	class MESHMERIZER_API FractureConvex : public Allocateable
	{
		public:
								FractureConvex(const FractureConvex* convex);
								FractureConvex(udword numVertices, const Point* vertices, udword numFaces, const udword* indices, const Matrix4x4* trans = null);
								FractureConvex(udword nbPts, const Point* polyPoints, float height);
								~FractureConvex();

				void			Release();

				void			Init(udword numVertices, const Point* vertices, udword numFaces, const udword* indices, const Matrix4x4* trans);

				Point			centerAtZero();

				bool			cut(const Point& localPlaneN, float localPlaneD, FractureConvex*& c0, FractureConvex*& c1);
				bool			selfCut(const Point& localPlaneN, float localPlaneD, bool& selfCutEmpty);

				void			getPlanes(Planes& planes) const;

		inline_	udword			GetNbFaces()	const	{ return mFirstIds.GetNbEntries()-1;	}
		inline_	udword			GetNbVerts()	const	{ return mVertices.GetNbVertices();		}
		inline_	const Point*	GetVerts()		const	{ return mVertices.GetVertices();		}
		inline_	const udword*	GetFirstIds()	const	{ return mFirstIds.GetEntries();		}
		inline_	const udword*	GetIndices()	const	{ return mIndices.GetEntries();			}

		private:
				void			InitVertices(udword numVertices, const Point* vertices, const Matrix4x4* trans);
				void			InitFaces(udword numVertices, udword numFaces, const udword* indices);
				void			clear();

				bool			cut(const Point& localPlaneN, float localPlaneD, FractureConvex*& c0, FractureConvex*& c1, bool selfCut, bool& selfCutEmpty);
				bool			cut2(const Point& localPlaneN, float localPlaneD, FractureConvex*& c0, FractureConvex*& c1, bool selfCut, bool& selfCutEmpty);

				Container		mFirstIds;
				Container		mIndices;
				Vertices		mVertices;
		public:
				bool			mKinematic;
	};

#endif	// ICEFRACTURECONVEX_H