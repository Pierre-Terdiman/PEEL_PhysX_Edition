///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for hull-related Gauss maps
 *	\file		IceHullGaussMaps.h
 *	\author		Pierre Terdiman
 *	\date		September, 12, 2004
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEHULLGAUSSMAPS_H
#define ICEHULLGAUSSMAPS_H

	class MESHMERIZER_API ClosestPolygonMap : public GaussMap
	{
		public:
								ClosestPolygonMap(const ConvexHull* hull);
		virtual					~ClosestPolygonMap();

		// Implements GaussMap
		virtual	bool			Initialize();
		virtual	bool			PrecomputeSample(udword offset, const Point& dir);
		virtual	void			PostInit();
				ubyte*			mSamples;
		private:
		const	ConvexHull*		mHull;
	};

	class MESHMERIZER_API SupportVertexMap : public GaussMap
	{
		public:
								SupportVertexMap(const ConvexHull* hull);
		virtual					~SupportVertexMap();

		// Implements GaussMap
		virtual	bool			Initialize();
		virtual	bool			PrecomputeSample(udword offset, const Point& dir);
		virtual	void			PostInit();

				ubyte*			mSamples;
				ubyte*			mSamples2;
		private:
		const	ConvexHull*		mHull;
	};

	class MESHMERIZER_API RaycastMap : public GaussMap
	{
		public:
								RaycastMap(const ConvexHull* hull);
		virtual					~RaycastMap();

		// Implements GaussMap
		virtual	bool			Initialize();
		virtual	bool			PrecomputeSample(udword offset, const Point& dir);
		virtual	void			PostInit();

		// Experimental
				bool			FastPointCulling(const Point& p)	const;
				bool			PointInHull(const Point& p)			const;

				ubyte*			mSamples;
		private:
		const	ConvexHull*		mHull;
	};

#ifdef MSH_SUPPORT_AERO_MAP
	struct MESHMERIZER_API AeroData
	{
		float	mArea;
		Point	mDir;
	};

	class MESHMERIZER_API AeroMap : public GaussMap
	{
		public:
								AeroMap(const ConvexHull* hull);
		virtual					~AeroMap();

		// Implements GaussMap
		virtual	bool			Initialize();
		virtual	bool			PrecomputeSample(udword offset, const Point& dir);

				AeroData*		mSamples;
		private:
		const	ConvexHull*		mHull;
	};
#endif

#ifdef MSH_SUPPORT_PROJ_MAP
	class MESHMERIZER_API ProjectionMap : public GaussMap
	{
		public:
								ProjectionMap(const CullingHull* hull);
		virtual					~ProjectionMap();

		// Implements GaussMap
		virtual	bool			Initialize();
		virtual	bool			PrecomputeSample(udword offset, const Point& dir);

				float*			mMinSamples;
				float*			mMaxSamples;
		private:
		const	CullingHull*	mHull;
	};
#endif

#endif // ICEHULLGAUSSMAPS_H
