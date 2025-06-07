///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "GLFontRenderer.h"
#include "Loader_Bin.h"
#include "PEEL.h"
#include "PintVisibilityManager.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_VisibilityAndSQFlag = "This test checks that support for renderable & scene-query flags has been properly implemented by each engine. If the test works, \
the bottom cube should periodically become invisible, and scene-queries should then ignore it. Note however that the cube should remain part of the rigid body simulation, i.e. the top \
cube shouldn't fall! This is not about removing the object from the scene, this is about deactivating it from the editor's point of view.";

START_TEST(VisibilityAndSQFlag, CATEGORY_API, gDesc_VisibilityAndSQFlag)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.86f, 4.02f, 4.92f), Point(-0.64f, -0.45f, -0.62f));
		SetDefEnv(desc, true);
	}

	virtual bool	CommonSetup()
	{
		RegisterRaycast(Point(3.0f, 0.5, 0.0f), Point(-1.0f, 0.0f, 0.0f), 100.0f);
		RegisterSphereSweep(Sphere(Point(0.0f, 0.5f, 3.0f), 0.25f), Point(0.0f, 0.0f, -1.0f), 100.0f);
		RegisterSphereOverlap(Sphere(Point(0.5f, 0.5f, 0.5f), 0.25f));

		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportRaycasts || !caps.mSupportSphereSweeps || !caps.mSupportSphereOverlaps)
			return false;

		const Point Extents(0.5f, 0.5, 0.5);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass			= 1.0f;
		ObjectDesc.mPosition		= Point(0.0f, Extents.y, 0.0f);
		const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		pint.mUserData = Handle;

		ObjectDesc.mPosition.y		+= Extents.y*2.0f;
		CreatePintObject(pint, ObjectDesc);

		return true;
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)
	{
		if(pint.mUserData==removed_object)
			pint.mUserData = null;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		if(pint.mVisHelper && pint.mUserData)
		{
			const float s = sinf(mCurrentTime*10.0f);
			pint.mVisHelper->SetRenderable(PintActorHandle(pint.mUserData), s>0.0f);
		}

		DoBatchRaycasts(*this, pint);
		DoBatchSphereSweeps(*this, pint);
		DoBatchSphereOverlaps(*this, pint, OVERLAP_ANY);
		return TestBase::Update(pint, dt);
	}

END_TEST(VisibilityAndSQFlag)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CollisionGroups = "Simple filtering test, replicating the old collision groups from the NovodeX SDK. If the test works, \
the falling spheres should not collide with the spheres below them. The groups will mainly be used to disable collision detection between jointed \
objects, for engines that do not have a built-in feature for doing so.";

START_TEST(CollisionGroups, CATEGORY_API, gDesc_CollisionGroups)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(8.63f, 5.94f, -9.43f), Point(-0.56f, -0.55f, 0.61f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCollisionGroups || !caps.mSupportRigidBodySimulation)
			return false;

		// Group 0 is already used by the default static environment so we start with group 1
		const PintCollisionGroup Layer0_Group = 1;
		const PintCollisionGroup Layer1_Group = 2;

		const PintDisabledGroups DG(Layer0_Group, Layer1_Group);
		pint.SetDisabledGroups(1, &DG);

		const float Radius = 1.0f;
		const udword NbX = 4;
		const udword NbY = 4;
		GenerateArrayOfSpheres(pint, Radius, NbX, NbY, Radius, 4.0f, 4.0f, 1.0f, Layer0_Group);
		GenerateArrayOfSpheres(pint, Radius, NbX, NbY, Radius*4.0f, 4.0f, 4.0f, 1.0f, Layer1_Group);
		return true;
	}

END_TEST(CollisionGroups)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_InactiveShape = "Test that inactive shapes are properly handled. Inactive shapes should not have collisions and should not contribute to the object's inertia. This is mainly used for pure visual shapes. \
This test creates two compunds made of a box and a sphere. In one of the compounds the box is set as inactive. That object should not rotate when the test starts.";

START_TEST(InactiveShape, CATEGORY_API, gDesc_InactiveShape)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.46f, 6.16f, 11.04f), Point(-0.67f, -0.35f, -0.66f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;

		for(udword i=0;i<2;i++)
		{
			PINT_SPHERE_CREATE SphereDesc(1.0);
			SphereDesc.mRenderer = CreateRenderer(SphereDesc);

			Matrix3x3 Rot;
			Rot.RotZ(PI/4.0f);

			const Point Extents(0.1f, 5.0f, 0.1f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateRenderer(BoxDesc);
			BoxDesc.mLocalPos = Point(-3.0f, 4.0f, 0.0f);
			BoxDesc.mLocalRot = Rot;
			BoxDesc.mFlags	= i ? SHAPE_FLAGS_INACTIVE : SHAPE_FLAGS_DEFAULT;
			SphereDesc.SetNext(&BoxDesc);

			PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Point(0.0f, 1.0f, float(i)*5.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(InactiveShape)

///////////////////////////////////////////////////////////////////////////////

static const Point gCOMLocalOffset(0.0f, 0.0f, 1.5f);

class InertiaTest : public TestBase
{
	const bool	mUseExplicitInertia;
	public:
							InertiaTest(bool explicit_inertia) : mUseExplicitInertia(explicit_inertia)	{}
	virtual					~InertiaTest()																{}
	virtual	TestCategory	GetCategory()		const	override	{ return CATEGORY_API;		}

	virtual	float	GetRenderData(Point& center)	const	override	{ return 20.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.40f, 2.92f, 5.12f), Point(-0.69f, -0.31f, -0.65f));
//		desc.mCamera[0] = PintCameraPose(Point(6.58f, -6.10f, 4.08f), Point(-0.90f, 0.20f, -0.38f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)	override
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const Point Extents(1.5f, 0.5f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(0.0f, Extents.y, 0.0f);

		if(mUseExplicitInertia)
		{
			ObjectDesc.mExplicitInertiaTensor		= Point(0.416666687f, 1.08333337f, 0.833333373f);
			ObjectDesc.mExplicitMassLocalPose.mPos	= gCOMLocalOffset;
		}
		else
		{
			ObjectDesc.mCOMLocalOffset	= gCOMLocalOffset;
		}

		pint.mUserData = CreatePintObject(pint, ObjectDesc);
		return true;
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)	override
	{
		if(pint.mUserData==removed_object)
			pint.mUserData = null;
	}

	virtual	void	DrawDebugInfo(Pint& pint, PintRender& render)	override
	{
		if(!pint.mUserData)
			return;

		const PintActorHandle Handle = PintActorHandle(pint.mUserData);
		const Matrix4x4 GlobalPose = pint.GetWorldTransform(Handle);

		const Point p = gCOMLocalOffset * GlobalPose;

		render.DrawLine(p, p + GlobalPose[0], Point(1.0f, 0.0f, 0.0f));
		render.DrawLine(p, p + GlobalPose[1], Point(0.0f, 1.0f, 0.0f));
		render.DrawLine(p, p + GlobalPose[2], Point(0.0f, 0.0f, 1.0f));
	}
};

static const char* gDesc_COMLocalOffset = "Tests that the center-of-mass (COM) local offset is properly taken into account. The box should rotate on its own if \
the test succeeds. The rendered RGB frame indicates the location of the COM.";

class COMLocalOffset : public InertiaTest
{
	public:
							COMLocalOffset() : InertiaTest(false)	{}
	virtual					~COMLocalOffset()						{}
	virtual	const char*		GetName()			const	{ return "COMLocalOffset";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_COMLocalOffset;		}
END_TEST(COMLocalOffset)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ExplicitInertia = "Same test as before, but using the API that let users set the inertia explicitly.";

class ExplicitInertia : public InertiaTest
{
	public:
							ExplicitInertia() : InertiaTest(true)	{}
	virtual					~ExplicitInertia()						{}
	virtual	const char*		GetName()			const	{ return "ExplicitInertia";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_ExplicitInertia;		}
END_TEST(ExplicitInertia)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AngularVelocity = "Tests that the SetAngularVelocity and GetAngularVelocity functions work as expected. Set the angular damping to zero \
in each plugin's UI if you want the velocity to remain constant. Otherwise it will decrease at a slightly different rate for each engine, depending on how damping \
is implemented. You can also use this test to check that the 'max angular velocity' setting works.";

START_TEST(AngularVelocity, CATEGORY_API, gDesc_AngularVelocity)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(0.87f, 23.38f, 8.87f), Point(-0.49f, -0.47f, -0.74f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const Point Extents(1.5f, 0.5f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(-2.264f, 18.78940f, 3.333f);
		//ObjectDesc.mAngularVelocity	= Point(0.0f, 0.0f, 10.0f);
		const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		pint.mUserData = Handle;

		pint.SetAngularVelocity(Handle, Point(0.0f, 0.0f, 10.0f));

		return true;
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)
	{
		if(pint.mUserData==removed_object)
			pint.mUserData = null;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		if(!pint.mUserData)
			return y;

		return PrintAngularVelocity(pint, renderer, PintActorHandle(pint.mUserData), y, text_scale);
	}

END_TEST(AngularVelocity)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_AddLocalTorque = "Tests that the AddLocalTorque function works as expected. Set the angular damping to zero \
in each plugin's UI if you want the angular velocity to remain constant. That way you can easily see which velocity has been reached \
in each engine, for the same torque.";

START_TEST(AddLocalTorque, CATEGORY_API, gDesc_AddLocalTorque)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		return CreateOverrideTabControl("AddLocalTorque", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(0.87f, 23.38f, 8.87f), Point(-0.49f, -0.47f, -0.74f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const Point Extents(1.5f, 0.5f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(-2.264f, 18.78940f, 3.333f);
		const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		pint.mUserData = Handle;

//		pint.SetAngularVelocity(Handle, Point(0.0f, 0.0f, 10.0f));
		pint.AddLocalTorque(Handle, Point(0.0f, 0.0f, 10.0f));

		return true;
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)
	{
		if(pint.mUserData==removed_object)
			pint.mUserData = null;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		if(!pint.mUserData)
			return y;

		return PrintAngularVelocity(pint, renderer, PintActorHandle(pint.mUserData), y, text_scale);
	}

END_TEST(AddLocalTorque)

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class ContactNotifyTest : public TestBase
	{
		protected:
		Vertices	mContacts;
		Vertices	mTriangles;

		virtual	void	CommonRelease()
		{
			mContacts.Empty();
			mTriangles.Empty();
			TestBase::CommonRelease();
		}

		virtual	void	CommonDebugRender(PintRender& render)
		{
			{
				const udword NbPts = mContacts.GetNbVertices()/2;
				const Point* V = mContacts.GetVertices();
				const Point Color(1.0f, 0.0f, 0.0f);
				for(udword i=0;i<NbPts;i++)
				{
					render.DrawLine(V[0], V[1], Color);
					V += 2;
				}
			}
			{
				const udword NbTris = mTriangles.GetNbVertices()/3;
				const Point* V = mTriangles.GetVertices();
				const Point Color(1.0f, 1.0f, 0.0f);
				for(udword i=0;i<NbTris;i++)
				{
					render.DrawTriangle(V[0], V[1], V[2], Color);
					V += 3;
				}
			}

			//if(!IsPaused())
			{
				mContacts.Reset();
				mTriangles.Reset();
			}
		}
	};
}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ContactNotify = "Contact notification test.";
class ContactNotify : public ContactNotifyTest, public PintContactNotifyCallback
{
	public:
							ContactNotify()				{								}
	virtual					~ContactNotify()			{								}
	virtual	const char*		GetName()			const	{ return "ContactNotify";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_ContactNotify;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_API;			}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
//		desc.mCamera[0] = PintCameraPose(Point(169.52f, 12.43f, 85.61f), Point(0.08f, -0.68f, 0.73f));
		desc.mCamera[0] = PintCameraPose(Point(178.56f, 2.81f, 83.82f), Point(-0.94f, -0.14f, 0.32f));
		desc.mContactNotifyCallback = this;
		SetDefEnv(desc, false);
	}

	virtual	float	GetRenderData(Point& center)	const
	{
		center = Point(169.52f, 12.43f, 85.61f);
		return 1000.0f;
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Archipelago.bin", null, false, 0, TESS_POLYHEDRAL);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportContactNotifications)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(169.52f, 4.0f, 85.61f);
		const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		(void)Handle;

		return true;
	}

	virtual	bool	BufferContacts()		const	{ return false;			}
	virtual	udword	GetContactFlags()		const	{ return CONTACT_ALL;	}
	virtual	float	GetContactThreshold()	const	{ return FLT_MAX;		}

	virtual	void	OnContact(Pint& pint, udword nb_contacts, const PintContactData* contacts)
	{
		//printf("OnContact: %d\n", nb_contacts);

		while(nb_contacts--)
		{
			const PintContactData& Current = *contacts++;
			mContacts.AddVertex(Current.mWorldPos);
			mContacts.AddVertex(Current.mWorldPos + Current.mNormal);

			//printf("feature1: %d\n", feature1);

			const udword feature1 = Current.mInternalFaceIndex1;

			Pint_Actor* ActorAPI = pint.GetActorAPI();
			if(ActorAPI)
			{
				PintShapeHandle S1 = ActorAPI->GetShape(Current.mObject1, 0);
				if(feature1!=INVALID_ID)
				{
					Pint_Shape* ShapeAPI = pint.GetShapeAPI();
					if(ShapeAPI)
					{
						Triangle tri;
						ShapeAPI->GetTriangle(tri, S1, feature1);
						mTriangles.AddVertex(tri.mVerts[0]);
						mTriangles.AddVertex(tri.mVerts[1]);
						mTriangles.AddVertex(tri.mVerts[2]);
					}
				}
			}
		}
	}

END_TEST(ContactNotify)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ContactModify = "Contact modification test.";
class ContactModify : public ContactNotifyTest, public PintContactModifyCallback
{
	public:
							ContactModify()				{								}
	virtual					~ContactModify()			{								}
	virtual	const char*		GetName()			const	{ return "ContactModify";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_ContactModify;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_API;			}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
//		desc.mCamera[0] = PintCameraPose(Point(169.52f, 12.43f, 85.61f), Point(0.08f, -0.68f, 0.73f));
		desc.mCamera[0] = PintCameraPose(Point(178.56f, 2.81f, 83.82f), Point(-0.94f, -0.14f, 0.32f));
//		desc.mContactNotifyCallback = this;
		desc.mContactModifyCallback = this;
		SetDefEnv(desc, false);
	}

	virtual	float	GetRenderData(Point& center)	const
	{
		center = Point(169.52f, 12.43f, 85.61f);
		return 1000.0f;
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		LoadMeshesFromFile_(*this, "Archipelago.bin", null, false, 0, TESS_POLYHEDRAL);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportContactModifications)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(169.52f, 4.0f, 85.61f);
		const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		(void)Handle;

		return true;
	}

	PintShapeHandle	mS1;
	// PintContactModifyCallback
	virtual	bool	PrepContactModify(Pint& pint, udword nb_contacts, PintActorHandle h0, PintActorHandle h1, PintShapeHandle s0, PintShapeHandle s1)	override
	{
//		printf("PrepContactModify: %d\n", nb_contacts);
		mS1 = s1;
		return true;
	}

	virtual	ContactModif	ModifyDelayedContact(Pint& pint, const PR& pose0, const PR& pose1, Point& p, Point& n, float& s, udword feature0, udword feature1, udword index)	override
	{
		return CONTACT_IGNORE;
	}

	virtual	ContactModif	ModifyContact(Pint& pint, const PR& pose0, const PR& pose1, Point& p, Point& n, float& s, udword feature0, udword feature1)	override
	{
		mContacts.AddVertex(p);
		mContacts.AddVertex(p+n);

		//printf("feature1: %d\n", feature1);

		if(feature1!=INVALID_ID)
		{
			Pint_Shape* ShapeAPI = pint.GetShapeAPI();
			if(ShapeAPI)
			{
				Triangle tri;
				ShapeAPI->GetTriangle(tri, mS1, feature1);
				mTriangles.AddVertex(tri.mVerts[0]);
				mTriangles.AddVertex(tri.mVerts[1]);
				mTriangles.AddVertex(tri.mVerts[2]);
			}
		}

		return CONTACT_AS_IS;
	}
	//~PintContactModifyCallback

END_TEST(ContactModify)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Compound = "Basic compound (whose shapes have an identity local pose).";

START_TEST(Compound, CATEGORY_API, gDesc_Compound)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(6.35f, 2.27f, 2.87f), Point(-0.90f, -0.08f, -0.42f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;

		const Point Extents(1.5f, 0.5f, 0.5f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		const Point Extents2(0.5f, 0.5f, 1.5f);
		PINT_BOX_CREATE BoxDesc2(Extents2);
		BoxDesc2.mRenderer	= CreateBoxRenderer(Extents2);
		BoxDesc2.SetNext(&BoxDesc);

//		for(udword i=0;i<1;i++)
		for(udword i=0;i<2;i++)
		{
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc2);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.0f, float(i+1)*2.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(Compound)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CapsuleCompound = "Capsule compound - to check capsule's orientations. Make sure the rendering matches the collision shapes.";

START_TEST(CapsuleCompound, CATEGORY_API, gDesc_CapsuleCompound)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-0.73f, 1.76f, 3.48f), Point(0.24f, -0.26f, -0.94f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;

		const Point X(1.0f, 0.0f, 0.0f);
		const Point Y(0.0f, 1.0f, 0.0f);
		const Point Z(0.0f, 0.0f, 1.0f);

		PINT_CAPSULE_CREATE CapsuleDesc(0.1f, 1.0f);
		CapsuleDesc.mRenderer = CreateRenderer(CapsuleDesc);
		PINT_CAPSULE_CREATE CapsuleDesc2(CapsuleDesc);
		PINT_CAPSULE_CREATE CapsuleDesc3(CapsuleDesc);

		CapsuleDesc.SetNext(&CapsuleDesc2);
		CapsuleDesc2.SetNext(&CapsuleDesc3);

		CapsuleDesc.mName = "CapsuleY";

		CapsuleDesc2.mName = "CapsuleX";
		CapsuleDesc2.mLocalRot = ShortestRotation(Y, X);

		CapsuleDesc3.mName = "CapsuleZ";
		CapsuleDesc3.mLocalRot = ShortestRotation(Y, Z);

		PINT_OBJECT_CREATE ObjectDesc(&CapsuleDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(0.0f, 2.0f, 0.0f);
		CreatePintObject(pint, ObjectDesc);

		return true;
	}

END_TEST(CapsuleCompound)

///////////////////////////////////////////////////////////////////////////////

static Quat FromEuler(const float* src)
{
	float euler[3];
	euler[0] = src[0] / 180.0f * PI;
	euler[1] = src[2] / 180.0f * PI;
	euler[2] = src[1] / 180.0f * PI;

	Quat quat;
	quat.Identity();

	// loop over euler angles, accumulate rotations
	for(int i=0; i<3; i++)
	{
		float qrot[4] = {cosf(euler[i]/2.0f), 0, 0, 0};
		float sa = sinf(euler[i]/2.0f);

		// construct quaternion rotation
		if(i==0)
			qrot[1] = sa;
		else if(i==1)
			qrot[2] = sa;
		else if(i==2)
			qrot[3] = sa;

		// accumulate rotation
		Quat q(qrot[0], qrot[1], qrot[2], qrot[3]);
		Quat tmp = q * quat;
		//Quat tmp = quat * q;
		quat = tmp;
	}

	quat.Normalize();
	return quat;
}

static const char* gDesc_CylinderCompounds = "Cylinder compounds adapted from a MuJoCo scene.";

START_TEST(CylinderCompounds, CATEGORY_API, gDesc_CylinderCompounds)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		return CreateOverrideTabControl("CylinderCompounds", 20);
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(0.98f, 1.91f, 3.82f), Point(-0.24f, -0.26f, -0.94f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;
/*
    <body pos="-.3 0 .3" euler="80 10 0">
      <freejoint/>
      <replicate count="30" euler="0 0 12">
        <geom type="box" pos="0 -.12 0" size=".01315 .005 .1" rgba=".8 0 0 1"/>
      </replicate>
    </body>
*/
		{
			const Point BodyPos(-0.3f, 0.3f, 0.0f);
			const float BodyRot_Euler[3] = { 80.0f, 0.0f, 10.0f };
			const float Euler_Inc[3] = { 0.0f, 0.0f, 12.0f };
			const Point LocalPos(0.0f, 0.0f, -0.12f);
			const Point Size(0.01315f, 0.1f, 0.005f);
			Replicate(pint, BodyPos, BodyRot_Euler, 30, Euler_Inc, LocalPos, Size, 4.0f);
		}
/*
    <body pos="0 0 .4" euler="20 20 0">
      <freejoint/>
      <replicate count="10" euler="0 0 36">
        <geom type="box" pos="0 -0.067 0" size=".025 .01 .1" rgba="0 .8 0 1"/>
      </replicate>
    </body>
*/
		{
			const Point BodyPos(0.0f, 0.4f, 0.0f);
			const float BodyRot_Euler[3] = { 20.0f, 0.0f, 20.0f };
			const float Euler_Inc[3] = { 0.0f, 0.0f, 36.0f };
			const Point LocalPos(0.0f, 0.0f, -0.067f);
			const Point Size(0.025f, 0.1f, 0.01f);
			Replicate(pint, BodyPos, BodyRot_Euler, 10, Euler_Inc, LocalPos, Size, 4.0f);
		}
/*
    <body pos=".3 0 .3" euler="30 30 0">
      <freejoint/>
      <replicate count="4" euler="0 0 90">
        <geom type="box" pos="0.005 -0.05 0" size=".05 .005 .08" rgba="0 0 .8 1"/>
      </replicate>
    </body>
*/
		{
			const Point BodyPos(0.3f, 0.3f, 0.0f);
			const float BodyRot_Euler[3] = { 30.0f, 0.0f, 30.0f };
			const float Euler_Inc[3] = { 0.0f, 0.0f, 90.0f };
			const Point LocalPos(0.005f, 0.0f, -0.05f);
			const Point Size(0.05f, 0.08f, 0.005f);
			Replicate(pint, BodyPos, BodyRot_Euler, 4, Euler_Inc, LocalPos, Size, 4.0f);
		}
		return true;
	}

	void	Replicate(	Pint& pint,
						const Point& body_pos, const float* body_rot_euler,
						udword count, const float* euler_inc, const Point& local_pos,
						const Point& initial_size, float scale)
	{
		float Scale = scale;
		{
			const Point pos = local_pos * Scale;
			const Point size = initial_size * scale;
			PintShapeRenderer* renderer = CreateBoxRenderer(size);

			float a = 0.0f;
			float b = 0.0f;
			float c = 0.0f;

			const udword MAX_COUNT = 32;
			PINT_BOX_CREATE BoxDesc[MAX_COUNT];
			for(udword i=0;i<count;i++)
			{
				float euler[3] = {a,b,c};

				BoxDesc[i].mExtents		= size;
				BoxDesc[i].mRenderer	= renderer;
				BoxDesc[i].mLocalRot	= FromEuler(euler);
				BoxDesc[i].mLocalPos	= pos * Matrix3x3(BoxDesc[i].mLocalRot);
				//BoxDesc[i].mLocalPos	= Matrix3x3(BoxDesc[i].mLocalRot) * pos;
				a += euler_inc[0];
				b += euler_inc[1];
				c += euler_inc[2];
				if(i!=count-1)
					BoxDesc[i].SetNext(&BoxDesc[i+1]);
			}

			{
				PINT_OBJECT_CREATE ObjectDesc(BoxDesc);
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mPosition	= body_pos * Scale;
				ObjectDesc.mRotation	= FromEuler(body_rot_euler);
				CreatePintObject(pint, ObjectDesc);
			}
		}
	}

END_TEST(CylinderCompounds)

///////////////////////////////////////////////////////////////////////////////

/*static const char* gDesc_ContactAndRestOffsets = "Contact and rest offsets.";
START_TEST(ContactAndRestOffsets, CATEGORY_API, gDesc_ContactAndRestOffsets)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		return CreateOverrideTabControl("ContactAndRestOffsets", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.04f, 4.01f, 1.33f), Point(-0.93f, -0.36f, -0.09f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		mPlanarMeshHelper.Generate(32, 0.01f);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Altitude = 0.0f;
		mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, null);

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(0.0f, 4.0f, 0.0f);
		CreatePintObject(pint, ObjectDesc);
		return true;
	}

END_TEST(ContactAndRestOffsets)*/

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DynamicMesh = "Dynamic mesh.";

START_TEST(DynamicMesh, CATEGORY_API, gDesc_DynamicMesh)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.97f, 2.69f, -3.01f), Point(0.56f, -0.40f, 0.72f));
		SetDefEnv(desc, true);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		BinLoaderSettings Settings(0.1f);
		Settings.mResetPivot = true;
		LoadBinMeshesFromFile(*this, "bunny.bin", Settings);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportDynamicMeshes)
			return false;

//		return CreateMeshesFromRegisteredSurfaces(pint, caps, *this, null, null, "Mesh");

		if(!GetNbRegisteredSurfaces())
			return false;
		const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);

		const IndexedSurface* IS = SD->mSurface;
		PintShapeRenderer* Renderer = SD->mRenderer;

		PINT_MESH_CREATE MeshDesc;
		MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
		MeshDesc.mRenderer	= Renderer;
		MeshDesc.mDynamic	= true;

		PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= Point(0.0f, 2.0f, 0.0f);
		CreatePintObject(pint, ObjectDesc);

		ObjectDesc.mPosition	= Point(0.0f, 4.0f, 0.0f);
		CreatePintObject(pint, ObjectDesc);

		return true;
	}

END_TEST(DynamicMesh)

///////////////////////////////////////////////////////////////////////////////

#include "TestScenes_Heightfield.h"

static const char* gDesc_Heightfield = "Heightfield.";

class Heightfield : public HeightfieldTest
{
	public:
								Heightfield() : HeightfieldTest(128, 128)	{							}
	virtual						~Heightfield()							{							}
	virtual	const char*			GetName()						const	{ return "Heightfield";		}
	virtual	const char*			GetDescription()				const	{ return gDesc_Heightfield;	}
	virtual	TestCategory		GetCategory()					const	{ return CATEGORY_API;		}

	virtual	float	GetRenderData(Point& center)	const	{ return 200.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		return CreateOverrideTabControl("Heightfield", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.97f, 2.69f, -3.01f), Point(0.56f, -0.40f, 0.72f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

		mHH.Init(40.0f, 30.0f, 1.0f);

		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!HeightfieldTest::Init(pint, caps, Point(0.0f, 0.0f, 0.0f)))
			return false;

		{
			PINT_SPHERE_CREATE Create(0.1f);
			Create.mRenderer	= CreateSphereRenderer(Create.mRadius);
			PintActorHandle ShapeHandle = CreateDynamicObject(pint, &Create, Point(0.0f, 1.0f, 0.0f));
			//PintActorHandle ShapeHandle = CreateDynamicObject(pint, &Create, Point(15.0f, 2.0f, 15.0f));
			ASSERT(ShapeHandle);
		}

		{
			PINT_BOX_CREATE Create(0.1f, 0.05f, 0.1f);
			Create.mRenderer	= CreateBoxRenderer(Create.mExtents);
			PintActorHandle ShapeHandle = CreateDynamicObject(pint, &Create, Point(1.0f, 1.5f, 0.0f));
			ASSERT(ShapeHandle);
		}

		return true;
	}

END_TEST(Heightfield)

