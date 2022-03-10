///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains consolidation data.
 *	\file		IceConsolidation.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECONSOLIDATION_H
#define ICECONSOLIDATION_H

	struct ICERENDERER_API MtlDesc : public Allocateable
	{
		inline_	MtlDesc() : mMatID(0), mNbFaces(0), mNbVerts(0)	{}

		udword	mMatID;
		udword	mNbFaces;
		udword	mNbVerts;
	};

	class ICERENDERER_API Consolidation : public Allocateable
	{
		public:
									Consolidation();
									~Consolidation();	// MUST NOT BE VIRTUAL

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes everything.
		 *	\param		consolidated_mesh	[in] result of consolidation process
		 *	\param		index_size			[in] 16 or 32 bits, or undefined to let the system choose
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				Init(const MBResult& consolidated_mesh, IndexSize index_size);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Frees everything.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void				Reset();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Generic component accessor.
		 *
		 *	\param		component	[in] component code
		 *	\param		i			[in] component index
		 *	\return		address of component
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				ubyte*				GetComponent(FVFComponent component, udword i)	const;
				ubyte*				GetBase(FVFComponent component)					const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Rebuilds vertex normals on-the-fly, after the vertex cloud has been modified.
		 *	This is a generic method, working for skinning, morphing, etc. It can be slower than a dedicated method (for example
		 *	you can skin your normals as well as vertices for characters)
		 *
		 *	\param		source		[in] new vertex cloud
		 *	\param		dest		[in] destination buffer
		 *	\param		stride		[in] destination stride in bytes
		 *	\param		normalize	[in] true to normalize normals, else leave it to the renderer
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				RebuildVertexNormals(const Point* source, ubyte* dest, udword stride, bool normalize) const;

				bool				ColorSmoothing(udword nb_passes, udword nb_colors, Point* colors, const bool* preserved=null)	const;

		// Data access
		inline_	udword				GetNbGeomPts()					const	{ return mNbGeomPts;				}
		inline_	udword				GetNbVerts()					const	{ return mNbVerts;					}
		inline_	const udword*		GetVertsRefs()					const	{ return mVertsRefs;				}
		inline_	udword				GetFVF()						const	{ return mFVF;						}
		inline_	udword				GetFVFSize()					const	{ return mFVFSize;					}
		inline_	udword				GetVBSize()						const	{ return mVBSize;					}
		inline_	const ubyte*		GetVBCopy()						const	{ return mVBCopy;					}

		inline_	IndexSize			GetIndexSize()					const	{ return mIndexSize;				}
		inline_	udword				GetNbSoftFaces()				const	{ return mNbSoftFaces;				}
		inline_	udword				GetNbHardFaces()				const	{ return mNbHardFaces;				}
		inline_	const void*			GetHardFaces()					const	{ return mHardFaces;				}
		inline_	const udword*		GetHardToSoft()					const	{ return mHardToSoft;				}
		inline_	udword				HardToSoft(udword hard_index)	const	{ return mHardToSoft[hard_index];	}

		inline_	udword				GetNbMaterials()				const	{ return mNbMaterials;				}
		inline_	const MtlDesc*		GetMaterials()					const	{ return mMaterials;				}

		inline_	const udword*		GetNormalInfo()					const	{ return mNormalInfo;				}

		// Lazy evaluated elements

		inline_	const Matrix3x3*	GetTangentSpaceBasis()	const
									{
										// Lazy-evaluate tangent space basis
										if(!mBasis)	const_cast<Consolidation* const>(this)->BuildTangentSpace2();	// "mutable method"
										return mBasis;
									}

		inline_	const udword*		GetReverseMap()			const
									{
										// Lazy-evaluate reverse map
										if(!mSoftToHard)	const_cast<Consolidation* const>(this)->BuildReverseMap();	// "mutable method"
										return mSoftToHard;
									}

		// Stats
				udword				GetObjectSize()		const;
				udword				GetOwnedSize()		const;
				udword				GetUsedRam()		const;

		private:
		// Possible local VB copy
				udword				mNbGeomPts;		//!< Number of vertices before replication
				udword				mNbVerts;		//!< Number of vertices after replication = stored in the VB
				udword*				mVertsRefs;		//!< Post-replication to pre-replication mapping
				udword				mFVF;			//!< The FVF used to fill mVBCopy
				udword				mFVFSize;		//!< The FVF's size (saved result of ComputeFVFSize())
				udword				mVBSize;		//!< Total size of VB in bytes = mNbVerts * mFVFSize
				ubyte*				mVBCopy;		//!< Local VB copy in system memory
		// Possible local IB copy
				IndexSize			mIndexSize;		//!< 16 or 32-bits indices (mFaces)
				udword				mNbSoftFaces;	//!< Number of faces after replication, in the hardware buffer
				udword				mNbHardFaces;	//!< Number of faces after replication, in the hardware buffer
				void*				mHardFaces;		//!< List of face indices. This is an exact copy of the hardwired index buffer.
				udword*				mHardToSoft;	//!< Consolidated list to original list mapping
		// Possible materials copy
				udword				mNbMaterials;	//!< Number of materials
				MtlDesc*			mMaterials;		//!< List of material descriptors
		// Possible normal information
				udword*				mNormalInfo;	//!< Normal information table

		// Tangent space stuff
		private:
				udword*				mSoftToHard;	//!< Original list to consolidated list mapping
		// Possible tangent-space basis
				Matrix3x3*			mBasis;			//!< List of tangent-space basis (one/vertex, i.e. mNbVerts)

		// Internal methods
				bool				BuildTangentSpace();
				bool				BuildTangentSpace2();
				bool				BuildReverseMap();

		friend class ConsolidatedSurface;	// ### Should be removed in the end
	};

#endif // ICECONSOLIDATION_H
