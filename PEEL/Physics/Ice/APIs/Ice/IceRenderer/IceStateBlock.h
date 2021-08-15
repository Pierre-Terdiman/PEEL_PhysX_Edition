///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains state-blocks related code.
 *	\file		IceStateBlock.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTATEBLOCK_H
#define ICESTATEBLOCK_H

	// Recordable:								Recordable ID						Stage number		State / recorded
//	struct ICERENDERER_API BoolRS				{ BooleanRenderState	brs;		/* Undefined */		udword		state;		};
//	struct ICERENDERER_API DwordRS				{ DwordRenderState		drs;		/* Undefined */		udword		state;		};
//	struct ICERENDERER_API TextureRS			{ TextureRenderState	trs;		udword	stage;		udword		state;		};
//	struct ICERENDERER_API TextureTransformRS	{ /* Implicit ID*/					udword	stage;		Matrix4x4	transform;	};

	class ICERENDERER_API BoolRS : public Allocateable
	{
		public:
		//! Constructor
		inline_								BoolRS()	{}
		//! Destructor
		inline_								~BoolRS()	{}

		//! Operator for "if(BoolRS==BoolRS)"
					bool					operator==(const BoolRS& obj)	const;
		//! Operator for "if(BoolRS!=BoolRS)"
		inline_		bool					operator!=(const BoolRS& obj)	const	{ return !(*this==obj);	}

					BooleanRenderState		brs;
					udword					state;
	};

	class ICERENDERER_API DwordRS : public Allocateable
	{
		public:
		//! Constructor
		inline_								DwordRS()	{}
		//! Destructor
		inline_								~DwordRS()	{}

		//! Operator for "if(DwordRS==DwordRS)"
					bool					operator==(const DwordRS& obj)	const;
		//! Operator for "if(DwordRS!=DwordRS)"
		inline_		bool					operator!=(const DwordRS& obj)	const	{ return !(*this==obj);	}

					DwordRenderState		drs;
					udword					state;
	};

	class ICERENDERER_API TextureRS : public Allocateable
	{
		public:
		//! Constructor
		inline_								TextureRS()		{}
		//! Destructor
		inline_								~TextureRS()	{}

		//! Operator for "if(TextureRS==TextureRS)"
					bool					operator==(const TextureRS& obj)	const;
		//! Operator for "if(TextureRS!=TextureRS)"
		inline_		bool					operator!=(const TextureRS& obj)	const	{ return !(*this==obj);	}

					TextureRenderState		trs;
					udword					stage;
					udword					state;
	};

	class ICERENDERER_API TextureTransformRS : public Allocateable
	{
		public:
		//! Constructor
		inline_								TextureTransformRS()	{}
		//! Destructor
		inline_								~TextureTransformRS()	{}

		//! Operator for "if(TextureTransformRS==TextureTransformRS)"
					bool					operator==(const TextureTransformRS& obj)	const;
		//! Operator for "if(TextureTransformRS!=TextureTransformRS)"
		inline_		bool					operator!=(const TextureTransformRS& obj)	const	{ return !(*this==obj);	}

					udword					stage;
					Matrix4x4				transform;
	};



	class ICERENDERER_API BRSContainer : public Container
	{
		public:
		//! Constructor
		inline_								BRSContainer()									{}
		//! Destructor
		inline_								~BRSContainer()									{}

		#define SB_BRS_SIZE	(sizeof(BoolRS)/sizeof(udword))

		inline_		udword					GetNbRenderStates()		const					{ return GetNbEntries()/SB_BRS_SIZE;				}
		inline_		BoolRS*					GetRenderStates()		const					{ return (BoolRS*)GetEntries();						}

					void					AddRenderState(const BoolRS& rs)				{ Add(udword(rs.brs)).Add(udword(rs.state));		}
	};

	class ICERENDERER_API DRSContainer : public Container
	{
		public:
		//! Constructor
		inline_								DRSContainer()									{}
		//! Destructor
		inline_								~DRSContainer()									{}

		#define SB_DRS_SIZE	(sizeof(DwordRS)/sizeof(udword))

		inline_		udword					GetNbRenderStates()		const					{ return GetNbEntries()/SB_DRS_SIZE;				}
		inline_		DwordRS*				GetRenderStates()		const					{ return (DwordRS*)GetEntries();					}

					void					AddRenderState(const DwordRS& rs)				{ Add(udword(rs.drs)).Add(udword(rs.state));		}
	};

	class ICERENDERER_API TRSContainer : public Container
	{
		public:
		//! Constructor
		inline_								TRSContainer()									{}
		//! Destructor
		inline_								~TRSContainer()									{}

		#define SB_TRS_SIZE	(sizeof(TextureRS)/sizeof(udword))

		inline_		udword					GetNbRenderStates()		const					{ return GetNbEntries()/SB_TRS_SIZE;				}
		inline_		TextureRS*				GetRenderStates()		const					{ return (TextureRS*)GetEntries();					}

					void					AddRenderState(const TextureRS& rs)				{ Add(udword(rs.trs)).Add(udword(rs.stage)).Add(udword(rs.state));	}
	};

	class ICERENDERER_API TransformContainer : public MatrixPalette
	{
		public:
		//! Constructor
		inline_								TransformContainer()							{}
		//! Destructor
		inline_								~TransformContainer()							{}

		#define SB_TTRS_SIZE	(sizeof(TextureTransformRS)/sizeof(udword))

		inline_		udword					GetNbRenderStates()		const					{ return GetNbEntries()/SB_TTRS_SIZE;				}
		inline_		TextureTransformRS*		GetRenderStates()		const					{ return (TextureTransformRS*)GetEntries();			}

					void					AddRenderState(const TextureTransformRS& rs)	{ Add(udword(rs.stage));AddMatrix(rs.transform);	}
	};

	class ICERENDERER_API StateBlock : public Allocateable
	{
		private:
											StateBlock();
											~StateBlock();
		public:
		// Bool-controlled render states
					bool					RecordCall(BooleanRenderState brs, bool state);
					bool					RemoveCall(BooleanRenderState brs);
		// Dword-controlled render states
					bool					RecordCall(DwordRenderState drs, udword state);
					bool					RemoveCall(DwordRenderState drs);
		// Texture-stage render states
					bool					RecordCall(TextureRenderState trs, udword stage, udword state);
					bool					RemoveCall(TextureRenderState trs, udword stage);

					bool					RecordCall(udword stage, const Matrix4x4& transform);
		// Material properties
					bool					RecordCall(const MaterialProps& props);
		// Dynamic values
					bool					RecordCall(ValueType type, FunctionType func, float base, float amplitude, float phase, float frequency);

		inline_		bool					ApplyDynamicStates(RenderStateManager& rsm)
											{
												if(!mDSM || !mDSM->IsValid())	return false;

												const TimeInfo* Time = rsm.GetTimeInfo();
												if(!Time)	return false;

												mDSM->ComputeDynamicValues(*Time);
												return mDSM->ApplyDynamicStates(mMaterial, rsm);
											}
		private:
		// Hardwired ID
					udword					mHardID;				//!< Hardwired ID
		// Boolean caches
					udword					mRSFlagsSelectionMask;	//!< Bitmask: 1 for a managed render state, else 0.
					udword					mRSFlagsValuesMask;		//!< Bitmask: gives the wanted values for render states selected by mRSFlagsSelectionMask.
		// Containers
					BRSContainer			mRSBools;				//!< Captured boolean render states
					DRSContainer			mRSDwords;				//!< Captured dword render states
					TRSContainer			mRSTexture;				//!< Captured texture render states
					TransformContainer		mRSTextureTransform;	//!< Captured static texture transforms
					MaterialProps			mMaterial;				//!< The one and only "material" available in a single state block

					DynamicStateManager*	mDSM;					//!< Dynamic state manager
		// Management
					SBID					mID;
					uword					mRefCount;
		// Internal methods
					bool					Equals(StateBlock* sb)	const;
					bool					Reorganize();

		friend class RenderStateManager;
	};

#endif // ICESTATEBLOCK_H
