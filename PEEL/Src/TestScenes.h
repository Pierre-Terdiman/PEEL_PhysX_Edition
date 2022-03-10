///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_SCENES_H
#define TEST_SCENES_H

#include "Common.h"
#include "Pint.h"
#include "SurfaceManager.h"
#include "CameraManager.h"
#include "ControlInterface.h"
#include "Gamepad.h"

	class Pint;
	class PintRender;
	class PINT_WORLD_CREATE;
	class GLFontRenderer;
	class ZCB2Factory;
	class ZB2CreationCallback;
	class ZB2CustomizeCallback;

	// Update CategoryText[] if you modify this
	enum TestCategory
	{
		CATEGORY_UNDEFINED,
		CATEGORY_API,
		CATEGORY_COOKING,
		CATEGORY_BEHAVIOR,
		CATEGORY_CONTACT_GENERATION,
		CATEGORY_JOINTS_BASICS,
		CATEGORY_JOINTS,
		CATEGORY_JOINTS_PORTALS,
		CATEGORY_ARTICULATIONS,
		CATEGORY_MCARTICULATIONS,
		CATEGORY_RCARTICULATIONS,
		CATEGORY_PERFORMANCE,
		CATEGORY_FRACTURE,
		CATEGORY_KINEMATICS,
		CATEGORY_CCD,
		CATEGORY_VEHICLES,
		CATEGORY_CCT,
		CATEGORY_RAYCAST,
		CATEGORY_SWEEP,
		CATEGORY_OVERLAP,
		CATEGORY_MIDPHASE,
		CATEGORY_MIDPHASE_MESH_MESH,
		CATEGORY_STATIC_SCENE,
		CATEGORY_WIP,
		CATEGORY_DISABLED,
		CATEGORY_INTERNAL,
	};

	enum TestFlags
	{
		TEST_FLAGS_DEFAULT			= 0,
		TEST_FLAGS_USE_CURSOR_KEYS	= (1<<0),
	};

	enum TestProfilingFlags
	{
		PROFILING_SIM_UPDATE		= (1<<0),
		PROFILING_TEST_UPDATE		= (1<<1),
		PROFILING_TEST_POST_UPDATE	= (1<<2),
	};

	class PhysicsTest : public PEEL::ControlInterface, public GamepadInterface
	{
		public:
								PhysicsTest() : mMustResetTest(false)		{}
		virtual					~PhysicsTest()								{}

		virtual	const char*		GetName()						const		= 0;
		virtual	const char*		GetDescription()				const		= 0;
		virtual	TestCategory	GetCategory()					const		= 0;

		virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)		= 0;

		virtual	bool			CommonSetup()								= 0;
		virtual	void			CommonRelease()								= 0;
		virtual	void			CommonPreSimUpdate(float dt)				= 0;
		virtual	void			CommonUpdate(float dt)						= 0;
		virtual	void			CommonDebugRender(PintRender&)				= 0;
		virtual	void			CommonRender(PintRender&, PintRenderPass)	= 0;

		virtual	float			GetRenderData(Point& center)	const
								{
									center = Point(0.0f, 1.0f, 0.0f);
									return 200.0f;
								}

		virtual	bool			Init(Pint& pint, bool create_def_env)		= 0;
		virtual	void			Close(Pint& pint)							= 0;

		virtual	void			PreUpdate(Pint& pint, float dt)				{			}
		virtual	udword			Update(Pint& pint, float dt)				{ return 0;	}
		virtual	udword			PostUpdate(Pint& pint, float dt)			{ return 0;	}

		// Defines if the system should profile the PhysicsTest::Update() function.
		// If false, the system profiles the physics engine's update function.
		// If true, the system profiles the test's update function.
		// Regular tests return false. "SQ tests" return true.
		//
		// When returning true, the generic UI can actually be used to define what the
		// system should profile (see "SQ Profiling Mode" in the user manual).
		//virtual	bool			ProfileUpdate()								{ return false;					}

		virtual	udword			GetProfilingFlags()					const	{ return PROFILING_SIM_UPDATE;	}

		// Some tests use private customer assets that can't be included in the public version.
		// These tests are unfortunately disabled in that case.
		virtual	bool			IsPrivate()							const	{ return false;	}

		// Experimental support for per-test controls
		virtual	udword			GetFlags()									const				{ return TEST_FLAGS_DEFAULT;	}
		virtual	bool			SpecialKeyCallback(int key, int x, int y, bool down)			{ return false;	}
		virtual	bool			KeyboardCallback(unsigned char key, int x, int y, bool down)	{ return false;	}
		virtual	void			LostFocus()														{}
		virtual	bool			RepositionMouse()							const				{ return false;	}
		virtual	void			OnObjectReleased(Pint& pint, PintActorHandle removed_object)	{}

		// GamepadInterface
		virtual	void			OnButtonEvent(udword button_id, bool down)								override	{}
		virtual	void			OnAnalogButtonEvent(udword button_id, ubyte old_value, ubyte new_value)	override	{}
		virtual	void			OnAxisEvent(udword axis_id, float value)								override	{}
		//~GamepadInterface

		// Configurable tests can have their own dialog. They are created and deleted in these dedicated functions.
		// That way we can reset the scene without deleting the dialog window (thus keeping current settings alive).
		virtual	IceTabControl*	InitUI(PintGUIHelper& helper)			{ return null;	}
		virtual	void			CloseUI();
		// Returns optional sub-name for presets (will be used in saved Excel files)
		virtual	const char*		GetSubName()					const	{ return null;	}

		// Let tests draw some information on screen if needed. Experimental design.
		virtual	float			DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)	{ return y;	}
		virtual	void			DrawDebugInfo(Pint& pint, PintRender& render)	{}

		virtual	void			SetDefaultSubscene(const char* sub_scene)	{}

		// ### purely experimental
		virtual	bool			OnBulletHit(Pint& pint, const PintRaycastHit& hit, const Ray& world_ray)	{ return false;	}

		inline_	Widgets&		GetUIElements()							{ return mUIElems;				}
		inline_	void			RegisterUIElement(IceWidget* widget)	{ mUIElems.Register(widget);	}
				void			AddResetButton(IceWindow* parent, sdword x, sdword y, sdword width);
		private:
				Widgets			mUIElems;
		public:
		// This is used to let the per-test GUI (from "configurable tests") reset the test after users have
		// tweaked the test parameters.
				bool			mMustResetTest;
	};

	void			InitTests();
	udword			GetNbTests();
	PhysicsTest*	GetTest(udword i);

	// Admittedly this became a bit of a kitchen-sink class
	class TestBase : public SurfaceManager, /*public ObjectsManager,*/ public PhysicsTest
	{
		class PlanarMeshHelper
		{
			public:
								PlanarMeshHelper();
								~PlanarMeshHelper();

			void				Release();

			void				Generate(udword nb, float scale, float sine_freq=0.0f, float sine_amplitude=0.0f);
			void				Generate(udword nb_x, udword nb_z, const Point& scale, float sine_freq=0.0f, float sine_amplitude=0.0f);
			PintActorHandle		CreatePlanarMesh(Pint& pint, float altitude, const PINT_MATERIAL_CREATE* material);

			private:
			IndexedSurface*		mSurface;
			PintShapeRenderer*	mRenderer;
		};

		public:
										TestBase();
		virtual							~TestBase();

		class TestData : public Allocateable
		{
			public:
						TestData()	{}
			virtual		~TestData()	{}
		};

		// PhysicsTest
		virtual	void					GetSceneParams(PINT_WORLD_CREATE& params)	override;

		virtual	bool					CommonSetup()								override	{ return true;			}
		virtual	void					CommonRelease()								override;
		virtual	void					CommonPreSimUpdate(float dt)				override	{ mCurrentTime += dt;	}
		virtual	void					CommonUpdate(float dt)						override	{ /*mCurrentTime += dt;*/	}
		virtual	void					CommonRender(PintRender&, PintRenderPass)	override	{						}
		virtual	void					CommonDebugRender(PintRender&)				override	{						}

		virtual	bool					Init(Pint& pint, bool create_def_env)		override;
		virtual	void					Close(Pint& pint)							override;
		//~PhysicsTest

		virtual	bool					Setup(Pint& pint, const PintCaps& caps)	= 0;

				void					RegisterAABB(const AABB& aabb);
				void					RenderAllAABBs(PintRender& renderer);
		inline_	udword					GetNbAABBs()	const	{ return mAABBs.GetNbEntries()/(sizeof(AABB)/sizeof(udword));	}
		inline_	const AABB*				GetAABBs()		const	{ return (const AABB*)mAABBs.GetEntries();						}

				void					RegisterRaycast(const Point& origin, const Point& dir, float max_dist);
				udword					GetNbRegisteredRaycasts()	const;
				PintRaycastData*		GetRegisteredRaycasts()		const;

				void					RegisterBoxSweep(const OBB& box, const Point& dir, float max_dist);
				udword					GetNbRegisteredBoxSweeps()	const;
				PintBoxSweepData*		GetRegisteredBoxSweeps()	const;

				void					RegisterSphereSweep(const Sphere& sphere, const Point& dir, float max_dist);
				udword					GetNbRegisteredSphereSweeps()	const;
				PintSphereSweepData*	GetRegisteredSphereSweeps()		const;

				void					RegisterCapsuleSweep(const LSS& capsule, const Point& dir, float max_dist);
				udword					GetNbRegisteredCapsuleSweeps()	const;
				PintCapsuleSweepData*	GetRegisteredCapsuleSweeps()	const;

				void					RegisterConvexSweep(const udword convex_object_index, PintShapeRenderer* renderer, const PR& pr, const Point& dir, float max_dist);
				udword					GetNbRegisteredConvexSweeps()	const;
				PintConvexSweepData*	GetRegisteredConvexSweeps()		const;

				void					RegisterSphereOverlap(const Sphere& sphere);
				udword					GetNbRegisteredSphereOverlaps()	const;
				PintSphereOverlapData*	GetRegisteredSphereOverlaps()	const;

				void					RegisterBoxOverlap(const OBB& box);
				udword					GetNbRegisteredBoxOverlaps()	const;
				PintBoxOverlapData*		GetRegisteredBoxOverlaps()		const;

				void					RegisterCapsuleOverlap(const LSS& capsule);
				udword					GetNbRegisteredCapsuleOverlaps()	const;
				PintCapsuleOverlapData*	GetRegisteredCapsuleOverlaps()		const;

				void					RegisterConvexOverlap(const udword convex_object_index, PintShapeRenderer* renderer, const PR& pr);
				udword					GetNbRegisteredConvexOverlaps()		const;
				PintConvexOverlapData*	GetRegisteredConvexOverlaps()		const;

				void					RegisterRenderer(PintShapeRenderer* renderer);
				udword					GetNbRegisteredRenderers()			const;
				PintShapeRenderer**		GetRegisteredRenderers()			const;

				void					RegisterTestData(TestData* data)
										{
											mTestData.AddPtr(data);
										}

				bool					ImportZB2File(PINT_WORLD_CREATE& desc, const char* filename, ZB2CustomizeCallback* callback=null);
				bool					CreateZB2Scene(Pint& pint, const PintCaps& caps, ZB2CreationCallback* callback=null);

				CameraManager			mCameraManager;
				PlanarMeshHelper		mPlanarMeshHelper;

		const	PINT_MATERIAL_CREATE	mHighFrictionMaterial;
		const	PINT_MATERIAL_CREATE	mZeroFrictionMaterial;

		protected:
				void*					mRepX;				// Built-in support for one RepX scene
				ZCB2Factory*			mZB2Factory;		// Built-in support for one ZB2 scene
				float					mCurrentTime;
				Container				mAABBs;
				Container				mRaycastData;

				Container				mBoxSweepData;
				Container				mSphereSweepData;
				Container				mCapsuleSweepData;
				Container				mConvexSweepData;

				Container				mSphereOverlapData;
				Container				mBoxOverlapData;
				Container				mCapsuleOverlapData;
				Container				mConvexOverlapData;

				PtrContainer			mRenderers;
				PtrContainer			mTestData;
		public:
		inline_	void					SetDefEnv(PINT_WORLD_CREATE& desc, bool b)
										{
											desc.mCreateDefaultEnvironment = b;
										}
		protected:
				IceTabControl*			CreateOverrideTabControl(const char* name, udword extra_size);
	};

	#define START_TEST(name, category, desc)												\
		class name : public TestBase														\
		{																					\
			public:																			\
									name()						{						}	\
			virtual					~name()						{						}	\
			virtual	const char*		GetName()			const	{ return #name;			}	\
			virtual	const char*		GetDescription()	const	{ return desc;			}	\
			virtual	TestCategory	GetCategory()		const	{ return category;		}

	#define START_SQ_TEST(name, category, desc)												\
		class name : public TestBase														\
		{																					\
			public:																			\
									name()						{						}	\
			virtual					~name()						{						}	\
			virtual	const char*		GetName()			const	{ return #name;			}	\
			virtual	const char*		GetDescription()	const	{ return desc;			}	\
			virtual	TestCategory	GetCategory()		const	{ return category;		}	\
			virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;	}

	#define START_SQ_RAYCAST_TEST(name, category, desc)										\
		class name : public TestBase														\
		{																					\
			public:																			\
									name()						{						}	\
			virtual					~name()						{						}	\
			virtual	const char*		GetName()			const	{ return #name;			}	\
			virtual	const char*		GetDescription()	const	{ return desc;			}	\
			virtual	TestCategory	GetCategory()		const	{ return category;		}	\
			virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;			}\
			virtual	udword			Update(Pint& pint, float dt){ return DoBatchRaycasts(*this, pint);	}

	#define END_TEST(name)	}name;

#endif