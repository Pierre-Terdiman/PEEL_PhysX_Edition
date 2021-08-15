///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for erosion layers.
 *	\file		IceErosionLayer.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEEROSIONLAYER_H
#define ICEEROSIONLAYER_H

	struct ICETERRAIN_API EROSIONLAYERCREATE : HEIGHTLAYERCREATE
	{
		EROSIONLAYERCREATE() :
			mNbIterHydraulicErosion		(0),
			mPerformHydraulicErosion	(false),
			mKc							(0.0f),
			mKd							(0.0f),
			mKs							(0.0f),
			mRainFrequency				(0),
			mNbIterThermalWeathering	(0),
			mPerformThermalWeathering	(false),
			mTalusValue					(0.0f),
			mCt							(0.0f)
		{}

		// Hydraulic erosion
		udword		mNbIterHydraulicErosion;	//!< Number of iterations for this erosion process
		bool		mPerformHydraulicErosion;	//!< Validate this erosion model
		float		mKc;						//!< Sediment capacity constant
		float		mKd;						//!< Deposition constant
		float		mKs;						//!< Soil softness constant
		udword		mRainFrequency;
		// Thermal weathering
		udword		mNbIterThermalWeathering;	//!< Number of iterations for this erosion process
		bool		mPerformThermalWeathering;	//!< Validate this erosion model
		float		mTalusValue;				//!< Maximum allowed height difference between two neighbor vertices
		float		mCt;						//!< Percentage of moved material
	};

	class ICETERRAIN_API ErosionLayer : public HeightLayer
	{
		public:
							ErosionLayer();
		virtual				~ErosionLayer();

		virtual	bool		Init(const EROSIONLAYERCREATE& create);
		virtual	bool		Update(float* field, udword width, udword height)	const;

		inline_	udword		GetNbIterHydraulicErosion()		const	{ return mNbIterHydraulicErosion;	}
		inline_	bool		GetPerformHydraulicErosion()	const	{ return mPerformHydraulicErosion;	}
		inline_	float		GetKc()							const	{ return mKc;						}
		inline_	float		GetKd()							const	{ return mKd;						}
		inline_	float		GetKs()							const	{ return mKs;						}
		inline_	udword		GetRainFrequency()				const	{ return mRainFrequency;			}
		inline_	udword		GetNbIterThermalWeathering()	const	{ return mNbIterThermalWeathering;	}
		inline_	bool		GetPerformThermalWeathering()	const	{ return mPerformThermalWeathering;	}
		inline_	float		GetTalusValue()					const	{ return mTalusValue;				}
		inline_	float		GetCt()							const	{ return mCt;						}

		inline_	void		SetNbIterHydraulicErosion(udword nb)	{ mNbIterHydraulicErosion = nb;		}
		inline_	void		SetPerformHydraulicErosion(bool b)		{ mPerformHydraulicErosion = b;		}
		inline_	void		SetKc(float kc)							{ mKc = kc;							}
		inline_	void		SetKd(float kd)							{ mKd = kd;							}
		inline_	void		SetKs(float ks)							{ mKs = ks;							}
		inline_	void		SetRainFrequency(udword freq)			{ mRainFrequency = freq;			}
		inline_	void		SetNbIterThermalWeathering(udword nb)	{ mNbIterThermalWeathering = nb;	}
		inline_	void		SetPerformThermalWeathering(bool b)		{ mPerformThermalWeathering = b;	}
		inline_	void		SetTalusValue(float value)				{ mTalusValue = value;				}
		inline_	void		SetCt(float ct)							{ mCt = ct;							}

		protected:
		// Erosion parameters

			// Hydraulic erosion
				udword		mNbIterHydraulicErosion;
				bool		mPerformHydraulicErosion;
				float		mKc;
				float		mKd;
				float		mKs;
				udword		mRainFrequency;

			// Thermal weathering
				udword		mNbIterThermalWeathering;
				bool		mPerformThermalWeathering;
				float		mTalusValue;
				float		mCt;
	};

#endif // ICEEROSIONLAYER_H
