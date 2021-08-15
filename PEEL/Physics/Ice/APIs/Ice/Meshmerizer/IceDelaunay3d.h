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
#ifndef ICEDELAUNAY3D_H
#define ICEDELAUNAY3D_H

	class MESHMERIZER_API Delaunay3d : public Allocateable
	{
	public:
				Delaunay3d();
				~Delaunay3d();

		void	clear();

		// tetra mesh
		void	tetrahedralize(const Point* vertices, int numVerts, int byteStride, bool removeExtraVerts = true);

		// voronoi mesh, needs tetra mesh
		void	computeVoronoiMesh();

		struct Convex
		{
			int firstVert;
			int numVerts;
			int firstIndex;
			int numFaces;
			int firstNeighbor;
			int numNeighbors;
		};

		udword						GetNbConvexes()		const;
		const Delaunay3d::Convex*	GetConvexes()		const;

		const Point*				getConvexVerts()	const { return mConvexVerts.GetVertices(); }
		const udword*				getConvexIndices()	const { return mConvexIndices.GetEntries(); }

	//private:
		// ------------------------------------------------------
		struct Edge 
		{
			void init(int i0, int i1, int tetraNr, int neighborNr = -1)
			{
				this->tetraNr = tetraNr;
				this->neighborNr = neighborNr;
				if (i0 > i1) { this->i0 = i0; this->i1 = i1; }
				else { this->i0 = i1; this->i1 = i0; }
			}
			bool operator <(Edge &e) const
			{
				if (i0 < e.i0) return true;
				if (i0 > e.i0) return false;
				if (i1 < e.i1) return true;
				if (i1 > e.i1) return false;
				return (neighborNr < e.neighborNr);
			}
			bool operator ==(Edge &e) const
			{
				return i0 == e.i0 && i1 == e.i1;
			}
			int i0, i1;
			int tetraNr;
			int neighborNr;
		};

		// ------------------------------------------------------
		struct Tetra 
		{
			void init(int i0, int i1, int i2, int i3)
			{
				ids[0] = i0; ids[1] = i1; ids[2] = i2; ids[3] = i3;
				neighborNrs[0] = neighborNrs[1] = neighborNrs[2] = neighborNrs[3] = -1;
				circumsphereDirty = true;
				center.Zero();
				radiusSquared = 0.0f;
				deleted = false;
			}
			inline int& neighborOf(int i0, int i1, int i2)
			{
				if (ids[0] != i0 && ids[0] != i1 && ids[0] != i2) return neighborNrs[0]; 
				if (ids[1] != i0 && ids[1] != i1 && ids[1] != i2) return neighborNrs[1]; 
				if (ids[2] != i0 && ids[2] != i1 && ids[2] != i2) return neighborNrs[2]; 
				if (ids[3] != i0 && ids[3] != i1 && ids[3] != i2) return neighborNrs[3]; 
				return neighborNrs[0];
			}
			
			int ids[4];
			int neighborNrs[4];
			Point center;
			float radiusSquared;
			bool circumsphereDirty;
			bool deleted;

			static const int sideIndices[4][3];
		};

		// ------------------------------------------------------

		void				delaunayTetrahedralization();

		int					findSurroundingTetra(int startTetra, const Point& p) const;
		void				updateCircumSphere(Tetra& tetra);
		bool				pointInCircumSphere(Tetra& tetra, const Point& p);
		void				retriangulate(int tetraNr, int vertNr);
		void				compressTetrahedra(bool removeExtraVerts);

		int					mFirstFarVertex;
		int					mLastFarVertex;
		Vertices			mVertices;
		Container			mTetras;

		Vertices			mConvexVerts;
		Container			mConvexIndices;
		Container			mConvexes;
		Container			mConvexNeighbors;
	};

#endif	// ICEDELAUNAY3D_H

