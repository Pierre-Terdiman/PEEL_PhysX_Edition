///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to compute normals.
 *	\file		IceNormals.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICENORMALS_H
#define ICENORMALS_H

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Computes vertex-normals for a patch of nbu*nbv points.
	 *	\param		normals		[out] destination array, large enough for nbu*nbv normals
	 *	\param		nbu			[in] number of vertices along U
	 *	\param		nbv			[in] number of vertices along V
	 *	\param		verts		[in] nbu*nbv source vertices
	 *	\param		mode		[in] normal computation mode
	 *	\param		normalize	[in] true to normalize the vectors
	 *	\return		true if success.
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION MESHMERIZER_API bool ComputePatchNormals(Point* normals, udword nbu, udword nbv, const Point* verts, udword mode, bool normalize=true);

	// Normal packing/unpacking
	FUNCTION MESHMERIZER_API void QuantizeNormal(float nx, float ny, float nz, long& sextant, long& octant, long& qphi, long& qtheta, unsigned char nb_bits=5);
	FUNCTION MESHMERIZER_API void DequantizeNormal(long sextant, long octant, long qphi, long qtheta, float& nx, float& ny, float& nz, unsigned char nb_bits=5);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Encodes a normal vector to a 16-bits word.
	 *	\param		normal	[in] (unit) normal vector to encode
	 *	\return		encoded normal. It could be used as in index in a 64K-entries normal map to speed up decoding.
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline_ uword EncodeNormal(const Point& normal)
	{
		long Sextant;
		long Octant;
		long QPhi;
		long QTheta;
		QuantizeNormal(normal.x, normal.y, normal.z, Sextant, Octant, QPhi, QTheta, 5);
		ASSERT(QPhi<32);
		ASSERT(QTheta<32);
		ASSERT(QPhi>=0);
		ASSERT(QTheta>=0);
		return uword(Sextant|(Octant<<3)|(QPhi<<6)|(QTheta<<11));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Decodes a normal vector from a 16-bits word.
	 *	\param		normal	[out] decoded normal vector
	 *	\param		code	[in] encoded word
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline_ void DecodeNormal(Point& normal, uword code)
	{
		long QTheta		= code>>11;
		long QPhi		= (code>>6) & 31;
		long Octant		= (code>>3) & 7;
		long Sextant	= code & 7;
		DequantizeNormal(Sextant, Octant, QPhi, QTheta, normal.x, normal.y, normal.z);
	}

	MESHMERIZER_API	bool BuildSmoothNormals(udword nb_tris, udword nb_verts, const Point* verts, const udword* dfaces, const uword* wfaces, Point* normals, bool flip);

	//! Normals creation structure
	struct MESHMERIZER_API NORMALSCREATE{
		//! Constructor
						NORMALSCREATE()	{ ZeroMemory(this, SIZEOFOBJECT); }
		// In
		udword			NbVerts;		//!< Number of vertices
		const Point*	Verts;			//!< List of vertices
		udword			NbFaces;		//!< Number of faces
		const udword*	dFaces;			//!< List of faces (triangle list) or null
		const uword*	wFaces;			//!< List of faces (triangle list) or null
		bool			UseAngles;		//!< Take edge angles into account
		// Out
		Point*			FNormals;		//!< Face-normals. If available, normals are written here, else kept in the class.
		Point*			VNormals;		//!< Vertex-normals. If available, normals are written here, else kept in the class.
	};

	class MESHMERIZER_API SmoothNormals
	{
		public:
						SmoothNormals();
						~SmoothNormals();

		// Computes normals.
				bool	Compute(const NORMALSCREATE& nc);

		// Data access
		inline_	Point*	GetFaceNormals()	const	{ return mFNormals; }
		inline_	Point*	GetVertexNormals()	const	{ return mVNormals; }

		private:
				Point*	mFNormals;
				Point*	mVNormals;
	};

#endif // ICENORMALS_H
