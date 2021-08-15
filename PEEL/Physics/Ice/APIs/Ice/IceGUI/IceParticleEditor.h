///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPARTICLEEDITOR_H
#define ICEPARTICLEEDITOR_H

	enum SingleValueCode
	{
		SOURCE_LIFETIME		= 1,
		SOURCE_MIN_NOISE	= 2,
		SOURCE_MAX_NOISE	= 3,
	};

	enum GraphCode
	{
		GRAPH_LIFETIME,
		GRAPH_SIZE,
		GRAPH_MASS,
		GRAPH_INITROT,
		GRAPH_SPIN,

		GRAPH_UNDEFINED
	};

	struct ParticleSystemInfo
	{
					ParticleSystemInfo();

		const char*	mName;
		udword		mNbSources;
	};

	struct ParticleSourceInfo
	{
							ParticleSourceInfo();

		const char*			mName;
		float				mLifetime;
		bool				mInfiniteLifetime;

		KeyframedTrack*		mLifetimeTrack;
		float				mLifetimeMinNoise;
		float				mLifetimeMaxNoise;

		KeyframedTrack*		mSizeTrack;
		float				mSizeMinNoise;
		float				mSizeMaxNoise;

		KeyframedTrack*		mMassTrack;
		float				mMassMinNoise;
		float				mMassMaxNoise;

		KeyframedTrack*		mInitRotTrack;
		float				mInitRotMinNoise;
		float				mInitRotMaxNoise;

		KeyframedTrack*		mSpinTrack;
		float				mSpinMinNoise;
		float				mSpinMaxNoise;
	};

	class ParticleEditorInterface
	{
		public:
							ParticleEditorInterface()	{}
		virtual				~ParticleEditorInterface()	{}

		virtual		udword	GetNbSystems()																										{ return 0;	}
		virtual		void	GetSystemInfo(udword system_index, ParticleSystemInfo& info)														{}
		virtual		void	GetSourceInfo(udword system_index, udword source_index, ParticleSourceInfo& info)									{}

		virtual		void	OnChangedValue(SingleValueCode code, GraphCode graph_code, const scell& value)										{}
		virtual		void	OnChangedGraph(GraphCode graph_code, udword nb_keys, const Keyframe* keys, uword graph_width, uword graph_height)	{}
		virtual		void	OnChangedPalette(const RGBAPalette* palette)																		{}
	};

	FUNCTION ICEGUI_API IceWindow* CreateParticleEditor(
										IceWidget* parent, sdword x, sdword y, sdword width, sdword height,
										ParticleEditorInterface* pei
										);
	FUNCTION ICEGUI_API void LoadParticleEditor(IceWindow*);

#endif	// ICEPARTICLEEDITOR_H