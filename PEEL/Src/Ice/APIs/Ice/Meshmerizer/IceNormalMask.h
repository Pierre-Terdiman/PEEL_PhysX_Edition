///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for normal masks.
 *	\file		IceNormalMask.h
 *	\author		Pierre Terdiman, original code by Hansong Zhang
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICENORMALMASK_H
#define ICENORMALMASK_H

//-------------------------------------------------------------
// nmask.h
//
// Hansong Zhang
// Department of Computer Science
// UNC-Chapel
// 1996
//
// Refer to "Fast Backface Culling Using Normal Masks" in
// ACM Symposium of Interactive 3D Graphics, 1997, for more
// details on the algorithm
//--------------------------------------------------------------

//--------------------------------------------------------------
// Defines 2 classes: NormalMask and PolygonNormalMask
//
// The NormalMask is used as the frontface mask in which every
// normal cluster has a bit to indicate whether it is
// front-facing (when the bit is 1) or backfacing (when
// the bit is 0). 
//
// Note that In my 1997 I3D paper I described the algorithm 
// in terms of a backface mask, which is the bitwise NOT of 
// the front-face mask used in this implementation - just to
// test if you really understand it :^)
//
// The PolygonNormalMask is the per-polygon normal mask,
// stored in the compact form (byteOffset, bitMask) since
// only one bit can be ON. ByteOffset and bitMask are both
// unsigned bytes.
//
// So the whole thing normally works this way:
//
// As a preprocessing (or during model loading) each polygon's
// 2-byte normal code is computed and stored in 
// PolygonNormalMask;
// Then for each frame, a FrontfaceMask (of type NormalMask) 
// is computed given the viewing direction and FOV, using
// the FillBits() function in NormalMask (you can do 
// this multiple time, each for a sub-frustum... see the 
// paper);
// For each polygon, FrontfaceMask[byteOffset] & bitMask tells
// you whether the polygon's considered front-facing. The macro
// FRONTFACING(VMSK, PMSK) is a shorthand for this.
//--------------------------------------------------------------

	// The backface test; used per polygon.
	#define FRONTFACING(VMSK,PMSK) ((VMSK)[(PMSK).byteOffset]&(PMSK).bitMask)

	#define MAX_NUM_SUBDIV 20

	static int Masks[8] = {1, 2, 4, 8, 16, 32, 64, 128};

	class MESHMERIZER_API NormalMask : public Allocateable
	{
		public:
								NormalMask(udword subdiv);
								~NormalMask();

				bool			Init(udword subdiv);
				void			Release();

		// Called in the constructor
				void			InitNormals();
    
		// This is called for each frame to form the viewing masking
		// representing all the "active" normal clusters. The main thing.
				void			FillBits(const Point& dir, float fov, bool silmode=false);

		inline_	void			TurnOn(int bitoffset)							{ mMask[bitoffset>>3] |= Masks[bitoffset & 0x7];		}
		inline_	void			TurnOff(int bitoffset)							{ mMask[bitoffset>>3] &= ~Masks[bitoffset & 0x7];		}
		inline_	void			TurnOn(int byteoffset, int bitoffset)			{ mMask[byteoffset] |= Masks[bitoffset & 0x7];			}
		inline_	void			TurnOff(int byteoffset, int bitoffset)			{ mMask[byteoffset] &= ~Masks[bitoffset & 0x7];			}
		inline_	int				IsOn(int bitoffset)						const	{ return mMask[bitoffset>>3] & Masks[bitoffset & 0x7];	}

		inline_	ubyte			operator[] (int byteoffset)				const	{ return mMask[byteoffset];								}

		inline_	udword			GetSubdivNumber()						const	{ return mSubdiv;										}
		inline_	udword			GetSize()								const	{ return mSize;											}
		inline_	const ubyte*	GetMask()								const	{ return mMask;											}

		private:
				udword			mSubdiv;	//!< Number of subdivisions
				udword			mSize;		//!< Size of the mask (mMask) in bytes
				ubyte*			mMask;		//!< Normal mask
	};

	struct MESHMERIZER_API PolygonNormalMask : public Allocateable
	{
		ubyte	byteOffset, bitMask;
//		udword	BitOffset;
		udword	Encode(const Point& normal, int subdiv);
	};

#endif // ICENORMALMASK_H
