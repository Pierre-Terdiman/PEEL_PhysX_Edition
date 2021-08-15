///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a bunny primitive.
 *	\file		IceBunny.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEBUNNY_H
#define ICEBUNNY_H

	class Bunny
	{
		public:
		// Constructor / Destructor
						Bunny();
						~Bunny();
		// Data access
		inline_	udword	GetNbVerts()		const	{ return mNbVerts;		}
		inline_	udword	GetNbFaces()		const	{ return mNbFaces;		}
		inline_	Point*	GetVerts()			const	{ return mVerts;		}
		inline_	uword*	GetFaces()			const	{ return mFaces;		}
		inline_	Point*	GetVertexNormals()	const
						{
							if(!mVertexNormals)	((Bunny*)this)->ComputeVertexNormals();
							return mVertexNormals;
						}

		inline_	SurfaceInterface	GetSurface()	const
									{
										return SurfaceInterface(mNbVerts, mVerts, mNbFaces, null, mFaces);
									}
		protected:
				udword	mNbVerts;		//!< Number of vertices
				udword	mNbFaces;		//!< Number of faces
				Point*	mVerts;			//!< List of vertices
				uword*	mFaces;			//!< List of faces
				Point*	mVertexNormals;	//!< List of vertex normals

				bool	ComputeVertexNormals();
	};

	struct BUNNYPRIMITIVECREATE
	{
	};

	bool MakeBunny(const BUNNYPRIMITIVECREATE& create, IndexedSurface* surface);

#endif // ICEBUNNY_H
