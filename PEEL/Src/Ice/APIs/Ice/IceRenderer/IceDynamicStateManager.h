///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains the dynamic state manager.
 *	\file		IceDynamicStateManager.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEDYNAMICSTATEMANAGER_H
#define ICEDYNAMICSTATEMANAGER_H

	enum ValueType
	{
		VAL_MATERIAL_POWER,

		VAL_MATERIAL_DIFFUSE_R,
		VAL_MATERIAL_DIFFUSE_G,
		VAL_MATERIAL_DIFFUSE_B,

		VAL_MATERIAL_AMBIENT_R,
		VAL_MATERIAL_AMBIENT_G,
		VAL_MATERIAL_AMBIENT_B,

		VAL_MATERIAL_SPECULAR_R,
		VAL_MATERIAL_SPECULAR_G,
		VAL_MATERIAL_SPECULAR_B,

		VAL_MATERIAL_EMISSIVE_R,
		VAL_MATERIAL_EMISSIVE_G,
		VAL_MATERIAL_EMISSIVE_B,

		VAL_MATERIAL_OPACITY,

		VAL_TEXTURE0_POSITION_X,
		VAL_TEXTURE0_POSITION_Y,
		VAL_TEXTURE0_ROTATION,
		VAL_TEXTURE0_ZOOM,

		VAL_TEXTURE0_MIPMAPLODBIAS,

		VAL_TEXTURE1_POSITION_X,
		VAL_TEXTURE1_POSITION_Y,
		VAL_TEXTURE1_ROTATION,
		VAL_TEXTURE1_ZOOM,

		VAL_ALPHAREF,
		VAL_TFACTOR_R,
		VAL_TFACTOR_G,
		VAL_TFACTOR_B,
		VAL_TFACTOR_A,
		VAL_TFACTOR_RGB,
		VAL_TFACTOR_RGBA,

		VAL_FORCE_DWORD	= 0x7fffffff
	};

	struct ICERENDERER_API DynamicValue : public Allocateable
	{
		ValueType	mType;
		Function	mFunction;
	};

	struct ICERENDERER_API ComputedValue : public Allocateable
	{
		ValueType	mType;
		float		mResult;
	};

	class ICERENDERER_API DynamicValueContainer : public Container
	{
		public:
		//! Constructor
		inline_							DynamicValueContainer() : mNeedsSorting(false)	{}
		//! Destructor
		inline_							~DynamicValueContainer()						{}

		#define SB_DYNAVAL_SIZE	(sizeof(DynamicValue)/sizeof(udword))

		inline_		udword				GetNbValues()	const		{ return GetNbEntries()/SB_DYNAVAL_SIZE;	}
		inline_		DynamicValue*		GetValues()		const		{ return (DynamicValue*)GetEntries();		}

					void				AddDynamicValue(ValueType type, const Function& func);
					bool				Sort();

					bool				mNeedsSorting;
	};

	// Forward declarations
	class RenderStateManager;

	class ICERENDERER_API DynamicStateManager : public Allocateable
	{
		public:
											DynamicStateManager();
											~DynamicStateManager();

		inline_		BOOL					IsValid()	const		{ return mVals.GetNbEntries();				}

					bool					RecordDynamicValue(ValueType type, const Function& func);

					bool					ComputeDynamicValues(const TimeInfo& time);
					bool					ApplyDynamicStates(MaterialProps& material, RenderStateManager& rsm);
		private:
					DynamicValueContainer	mVals;
					Container				mResults;

					Point					mPos0, mPos1;
					float					mRot0, mRot1;
					float					mZoom0, mZoom1;
					RGBAColor				mTFactor;

					float					mTimeStamp;

					bool					mUseMaterial;
					bool					mUseTexture0;
					bool					mUseTexture1;
					bool					mUseTFactor;

		// Internal methods
					void					OutputValue(float val, ValueType type, MaterialProps& mat, RenderStateManager& rsm);
	};

#endif // ICEDYNAMICSTATEMANAGER_H
