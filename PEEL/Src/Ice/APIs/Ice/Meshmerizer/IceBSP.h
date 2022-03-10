///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains BSP-related code.
 *	\file		IceBSP.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEBSP_H
#define ICEBSP_H

	enum BSPNodeCode
	{
		BSP_INSIDE_FACES	= (1<<0),		//!< Inside faces
		BSP_OUTSIDE_FACES	= (1<<1),		//!< Outside faces
		BSP_FORCE_DWORD		= 0x7fffffff
	};

	class MESHMERIZER_API BSPNode : public Allocateable
	{
		private:
							BSPNode();
							~BSPNode();
		public:
		// Builds a BSP from a tri-surface
				bool		Build(const TriSurface* surface, const Matrix4x4* world=null);
		// Builds a BSP from a indexed tri-surface
				bool		Build(const IndexedSurface* surface, const Matrix4x4* world=null);

		// Adds a triangle for CSG
				bool		_AddTriangle(Triangles& trilist, const Triangle& triangle, bool discard_positive);
		private:
		// Inner data
				Plane		mPlane;			//!< Split plane
				Triangles	mInnerList;		//!< Coplanar triangles
				BSPNode*	mInside;		//!< Left child
				BSPNode*	mOutside;		//!< Right child
				udword		mNodeType;		//!< Type

		// Builds a BSP from a list of triangles
				bool		_Build(const Triangles& trilist);
		// Collects all faces included so far in the BSP tree
				bool		_CollectFaces(Triangles& trilist, udword code) const;
		// Collects used ram
				void		_CollectUsedRam(udword& nbbytes) const;
		// Collects number of nodes
				void		_CollectNbNodes(udword& nbnodes) const;

		friend	class		BSPTree;
	};

	class MESHMERIZER_API BSPTree : public BSPNode
	{
		public:
							BSPTree();
							~BSPTree();

		// Collects all faces included so far in the BSP tree
				bool		CollectFaces(Triangles& trilist, udword code)	{ return _CollectFaces(trilist, code);	}
		// Collects used ram
				udword		GetUsedRam() const;
		// Gets number of nodes
				udword		GetNbNodes() const;
	};

#endif // ICEBSP_H
