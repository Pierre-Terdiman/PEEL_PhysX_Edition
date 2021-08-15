///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for triangle containers.
 *	\file		IceTriangleContainers.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETRIANGLECONTAINERS_H
#define ICETRIANGLECONTAINERS_H

	class ICEMATHS_API Triangles : public Container
	{
		public:
							Triangles()					{}
							~Triangles()				{}

		inline_	udword		GetNbTriangles()	const	{ return GetNbEntries()/9;			}
		inline_	Triangle*	GetTriangles()		const	{ return (Triangle*)GetEntries();	}

				void		AddTri(const Triangle& tri)
							{
								Add(tri.mVerts[0].x).Add(tri.mVerts[0].y).Add(tri.mVerts[0].z);
								Add(tri.mVerts[1].x).Add(tri.mVerts[1].y).Add(tri.mVerts[1].z);
								Add(tri.mVerts[2].x).Add(tri.mVerts[2].y).Add(tri.mVerts[2].z);
							}

				void		AddTri(const Point& p0, const Point& p1, const Point& p2)
							{
								Add(p0.x).Add(p0.y).Add(p0.z);
								Add(p1.x).Add(p1.y).Add(p1.z);
								Add(p2.x).Add(p2.y).Add(p2.z);
							}
	};

	CHECK_CONTAINER_ITEM(Triangle)

	class ICEMATHS_API IndexedTriangles : public Container
	{
		public:
									IndexedTriangles()			{}
									~IndexedTriangles()			{}

		inline_	udword				GetNbTriangles()	const	{ return GetNbEntries()/3;				}
		inline_	IndexedTriangle*	GetTriangles()		const	{ return (IndexedTriangle*)GetEntries();}

				void				AddTriangle(const IndexedTriangle& tri)
									{
										Add(tri.mRef[0]).Add(tri.mRef[1]).Add(tri.mRef[2]);
									}

				void				AddTriangle(udword vref0, udword vref1, udword vref2)
									{
										Add(vref0).Add(vref1).Add(vref2);
									}
	};

	CHECK_CONTAINER_ITEM(IndexedTriangle)

#endif // ICETRIANGLECONTAINERS_H
