///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains Phototracer's scanline
 *	\file		IceScanline.h
 *	\author		Pierre Terdiman
 *	\date		August 1998
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESCANLINE_H
#define ICESCANLINE_H

	// Shared edge structure:
	struct FaceEdge
	{
		float		dXOverdY;
		float		dZOverdY;
	};

	// Span structure:
	struct FaceSpan
	{
		short		Count;
		short		Type;
		scell		XR;				// Interpolated along Y
		scell		XL;				// Interpolated along Y
		scell		XM;				// Copied at slope-swap time
		scell		ZL;				// Interpolated along Y
		scell		ZM;				// Copied at slope-swap time
		scell		dZx;			// Constant over face, do not change at slope-swap time
		FaceEdge*	ELeft;
		FaceEdge*	ERight;
		FaceEdge*	EMiddle;		// Copied at slope-swap time
		//
		scell		AlphaL;			// Interpolated along Y
		scell		AlphaM;			// Copied at slope-swap time
		scell		dAlphax;		// Constant over face, do not change at slope-swap time
		scell		dAlphay;		// Only needed for some shading methods
		//
		scell		BetaL;			// Interpolated along Y
		scell		BetaM;			// Copied at slope-swap time
		scell		dBetax;			// Constant over face, do not change at slope-swap time
		scell		dBetay;			// Only needed for some shading methods
		//
		scell		GammaL;			// Interpolated along Y
		scell		GammaM;			// Copied at slope-swap time
		scell		dGammax;		// Constant over face, do not change at slope-swap time
		scell		dGammay;		// Only needed for some shading methods
		//
		float		dAlphaOverdY;	// Current increment
		float		dBetaOverdY;	// Current increment
		float		dGammaOverdY;	// Current increment
		float		dAlphaOverdYM;	// Copied at slope-swap time
		float		dBetaOverdYM;	// Copied at slope-swap time
		float		dGammaOverdYM;	// Copied at slope-swap time
	};

	// Generic triangle structure:
	struct TRIANGLE{
		scell	X0, Y0, Z0;
		scell	X1, Y1, Z1;
		scell	X2, Y2, Z2;
	};

	// Generic vertex structure:
	struct RfVERTEX{
		scell	X;
		scell	Y;
		scell	Z;
	};

	// Subpixel structure:
	struct ITEM{
		udword	FaceNumber;
		scell	Z;
		scell	Alpha;
		scell	Beta;
		scell	Gamma;
		scell	dAlphax;
		scell	dAlphay;
		scell	dBetax;
		scell	dBetay;
		scell	dGammax;
		scell	dGammay;
	};

	typedef void (*PIXEL_CALLBACK)	(const ITEM& pixeldesc, RGBAColor& color, void* user_data);

	// Edge structure for each face:
	struct EDGES_REF{
		udword	Ref1;				// RfEDGE reference
		udword	Ref2;				// RfEDGE reference
		udword	Ref3;				// RfEDGE reference
	};

	// Temp Y values for each face:
	struct Y_VALUES{
		udword	YTop;				// Top ceiled Y value
		udword	YBottom;			// Bottom ceiled Y value
	};

	enum AALevel
	{
		AA_NONE			= 1,		// No antialiasing
		AA_OVER2		= 2,		// A-Buffer with 2*2 masks
		AA_OVER4		= 4,		// A-Buffer with 4*4 masks
		AA_OVER8		= 8,		// A-Buffer with 8*8 masks
		AA_FORCE_DWORD	= 0x7fffffff
	};

	struct SCANLINECREATE
	{
						SCANLINECREATE() : Antialias(AA_NONE), Target(null)
						{
						}

		AALevel			Antialias;
		Picture*		Target;
	};

	class ICERENDERER_API Scanline
	{
		public:
									Scanline();
									~Scanline();

					bool			Init(const SCANLINECREATE& create);
					bool			Render(udword nbfaces, const udword* dfaces, const uword* wfaces, udword nbverts, const Point* verts);
					void			ReleaseMemory();
					void			ReleaseObjectMemory();

		// Callback control
		inline_		Scanline&		SetUserData(void* data)						{ mUserData	= data;			return *this;	}
		inline_		Scanline&		SetPixelCallback(PIXEL_CALLBACK callback)	{ mCallback	= callback;		return *this;	}

//		private:
		// Pixel callback
					void*			mUserData;
					PIXEL_CALLBACK	mCallback;

					udword			mRamUsed;				// Ram currently used for scanline structures
					udword			mHighWaterMark;			// scanline Ram: High Water Mark

					udword			mScreenWidth;			// render screen width in pixels
					udword			mScreenHeight;			// render screen height in pixels
					udword			mSubScreenWidth;		// render screen width in subpixels
					udword			mSubScreenHeight;		// render screen height in subpixels

					udword			mAAScale;				// antialiasing scale factor [1, 2, 4, 8]
					float			mAACoeff;				// mAAScale in float

					ubyte*			mScreen;				// Render scanline
					Picture*		mTarget;				// Render target

					udword*			mPixelCount;			// A-Buffer fragment-areas
					ITEM**			mPixelComponents;		// index in mZLine for (mAAScale*mAAScale) subpixels

					sdword*			mNbBirth;				// Number of new faces for each line
					sdword*			mNbDeath;				// Number of faces to kill for each line
					sdword*			mNbBirthCopy;			// Number of new faces for each line
					sdword*			mNbDeathCopy;			// Number of faces to kill for each line
					sdword*			mNbBirthOffsetsCopy;	// copy of the birth-offsets!
					sdword*			mNbDeathOffsetsCopy;	// copy of the death-offsets!

					EDGES_REF*		mEdgesRef;				// 3 vertices references for each face
					sdword*			mBirth;					// index for faces to add
					sdword*			mDeath;					// index for faces to kill
					sdword*			mFacesLocation;			// index in .mFacesSpan
					scell*			mFacesLocation2;		// index in the mActiveSpanTable
					ubyte*			mFaceIndex;				// index in mPixelComponents
					Y_VALUES*		mYSave;					// YTop & YBottom for each face
		// Object description
					udword			mNbFaces;				// current total #faces
					udword			mNbVerts;				// current total #vertices
					udword*			mFaces;					// Vertices references
					Point*			mVerts;					// array of vertices

					udword			mMaxNbFaces;			// Maximum #active faces per scanline

					udword			mNbEdges;				// #non redundant edges
					udword			mMaxNbEdges;			// Maximum #active edges per scanline
					Edge*			mEdgesList;				// list of non-redundant edges
					long*			mEdgesLocation;			// index in .mEdges

					ubyte*			mBigBuffer;				// view-dependent allocated ram
					long			mCurrentEdgeLocation;	// index in .mEdges
					long			mCurrentFaceLocation;	// index in .mFacesSpan
					long			mFreeEdgeLocation;		// index in .mEdges
					long			mFreeFaceLocation;		// index in .mFacesSpan
					long			mNbFreeFaceLocation;	// face stack current size
					long			mNbFreeEdgeLocation;	// edge stack current size
					long			mNbActiveSpans;			// #active spans for current scanline
					long*			mFreeFaceLocationList;	// stack for free faces locations
					long*			mFreeEdgeLocationList;	// stack for free edges locations

					long			mUsedSegment;			// longest rendered segment [in #subpixels]
					FaceSpan*		mFacesSpan;				// ram for RfFACESPANs
					FaceEdge*		mEdges;					// ram for RfFACEEDGEs
					ITEM*			mZLine;					// sub-zbuffer
					long*			mActiveSpanTable;		// index for active faces

					long			mXMin;					// Minimal x
					long			mXMax;					// Maximal x

					long			mFaceToAdd;				// index: face to add to SpanTable
					long			mFaceToRemove;			// index: face to remove from SpanTable

					bool			mBackColor;				// Use a background color or not
		// Methods
					bool			CreateEdgesList();
					bool			DrawObject();
					bool			InitForNewFrame();
					bool			AddObjectToDatabase();
					bool			BuildHelpers();
					bool			FreeHelpers();

					void			AddFaceToSpanTable();
					void			RemoveFaceFromSpanTable();

					void			DrawScanlineAA1();
					void			DrawScanlineAA2();
					void			DrawScanlineAA4();
					void			DrawScanlineAA8();
	};

#endif // ICESCANLINE_H

