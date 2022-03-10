///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for the skin.
 *	\file		IceSkin.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESKIN_H
#define ICESKIN_H

	//! Skin flags
	enum SkinFlag
	{
		SKIN_ONE_BONE_PER_VERTEX	= (1<<0),		//!< READ-ONLY: the skin has one bone/vertex only

		SKIN_READONLY				= SKIN_ONE_BONE_PER_VERTEX,

		SKIN_FORCE_DWORD			= 0x7fffffff
	};

	//! Creation structure
	struct ICECHARACTER_API SKINCREATE : public Allocateable
	{
								SKINCREATE();

				udword			mNbVerts;			//!< Number of vertices in the skin
				const udword*	mBonesID;			//!< List of bones ID
				const udword*	mBonesNb;			//!< List of bones number (or null)
				const Point*	mOffsetVectors;		//!< List of offset vectors
				const float*	mWeights;			//!< List of weights (or null)
				bool			mFixData;			//!< true: let the engine use premultiplied offset vectors, else already done
	};

	class ICECHARACTER_API BoneToVertices : public Allocateable
	{
		public:
								BoneToVertices();
								~BoneToVertices();

				bool			Create(udword nb_verts, const udword* bonesid, const udword* bonesnb);

		// Data access
		inline_	const udword*	GetCounts()			const	{ return mNbVertsForBone;	}
		inline_	const udword*	GetOffsets()		const	{ return mOffsets;			}
		inline_	const udword*	GetVertexIndices()	const	{ return mVertexIndices;	}

		private:
				udword*			mNbVertsForBone;
				udword*			mOffsets;
				udword*			mVertexIndices;
	};

	class ICECHARACTER_API Skin
	{
		public:
		// Constructo/Destructor
								Skin();
		virtual					~Skin();

								DECLARE_FLAGS(SkinFlag, mSkinFlags, SKIN_READONLY)

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the skin.
		 *	\param		create		[in] creation structure
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool			Init(const SKINCREATE& create);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the skin according to current skeleton/matrix palette.
		 *	- the destination buffer should not directly be a vertex buffer, since you usually don't need to do the skinning on vertices
		 *	replicated by a consolidation process. Hence you may perform the skinning on the base mesh, then replicate skinned vertices
		 *	within the vertex buffer. Depends on your app, actually.
		 *	- the normals are not computed here anymore, since the final number of vertices may be quite different from the number of
		 *	vertices in the original skin (notably when it's used as the control net of a subdivision surface)
		 *
		 *	\param		skeleton	[in] underlying skeleton (could be shared among multiple skins)
		 *	\param		skin		[out] destination buffer
		 *	\param		stride		[in] destination buffer's stride (in number of floats)
		 *	\param		aabb		[out] new skin's AABB
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool			Compute(Skeleton* skeleton, float* skin, udword stride, AABB& aabb);

				BoneToVertices*	GetBoneToVertices()
								{
									if(!mBTV)
									{
										mBTV = ICE_NEW(BoneToVertices);
										if(!mBTV)	return null;
										mBTV->Create(mNbVerts, mBonesID, mBonesNb);
									}
									return mBTV;
								}

		// Accessors
		inline_	Point*			GetOffsetVectors()	const	{ return mOffsetVectors;	}
		inline_	float*			GetWeights()		const	{ return mWeights;			}
		inline_	udword*			GetBonesID()		const	{ return mBonesID;			}
		inline_	udword*			GetBonesNb()		const	{ return mBonesNb;			}
		inline_	udword			GetNbVerts()		const	{ return mNbVerts;			}
		inline_	udword			GetNbBonesID()		const	{ return mNbBonesID;		}

								PREVENT_COPY(Skin)
		private:
				Point*			mOffsetVectors;		//!< Skin's offset vectors
				udword			mNbBonesID;			//!< Total number of BonesID
				udword			mNbVerts;			//!< Total number of vertices in the character's skin
				udword*			mBonesID;			//!< A list of NbBonesID IDs. There can be one or many BonesID for each vertex of the skin. (==> links to underlying bones)
				float*			mWeights;			//!< A list of NbBonesID weights. One weight maps one BonesID in the previous list.
				udword*			mBonesNb;			//!< A list of NbVerts counts = a number of related bones for each one of the skin's vertex
		//
				BoneToVertices*	mBTV;				//!< Possible bone-to-vertices mapping
		// Internal methods
				void			FreeUsedRam();
	};

#endif // ICESKIN_H
