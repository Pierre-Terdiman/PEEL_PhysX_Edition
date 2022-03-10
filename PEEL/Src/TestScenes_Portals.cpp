///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "GLFontRenderer.h"
#include "PintDLMeshShapeRenderer.h"

///////////////////////////////////////////////////////////////////////////////

// TODO:
// * the cube should be able to enter the blue portal as well...
// Really the business with the contact modif etc is orthogonal to the simpler business of teleporting the single object
// once it crosses a portal
// - must ignore contacts on negative side of portal:
//     - there can be an obstacle directly behind it, preventing going through simple portals
//     - if a portal is horizontal & directly on the ground, we must ignore collisions with the ground ("on" the portal)
// - if these contacts are ignored, does that make the object rotate once its com has passed the portal?
//   => maybe not, since the rotation is linked to the shadow object
// - hmm if the dest portal is on the ceiling does it suck the other object in as soon as it crosses the source portal?
// - Strange: why doesn't the object rotate with CONTACT_IGNORE? It rotates when tweaking the depth...
// ==> lots of other issues & explosions with contact ignore!!!

// * add geom error term to prevent position drift
//	* it's very easy to get desync when gravity is applied to an upper cube!!!!!!!
// * doesn't work with tgs

/*
maybe easiest to only have one object from the physics pov

mirror manager virtually copying obstacles from the other side of the portals to get collisions etc
or we could actually do the copy with filtering rules.......... like query all objects from the other side,
rotate them as needed, copy them with special names and use that to tweak filtering

otherwise newly added constraints might disturb the motion

object whose center is on the front side of the portal considered "master"?
*/

/*
* remove "ReleaseObject" from ToolRayBased.h
- implement release for all tools
*/

// Issues:
// * creating the joint slows down the object a bit at runtime
// * deleting the object while it's picked = crash
// - CCD still an issue
// - one frame delay when we create the shadow object *after* detecting the overlap?
//
// Try to always create the shadow object and just relocate it?

// * check that the joint is deleted when we delete the actor => probably not, see memory increasing
// - recycle joint instead of recreating it all the time
// - make it work with editor, check the weird negative amount of materials there
// - make it work for N cubes (i.e. newly spawned ones)
// - dynamic portals?
// - multiple scenes / scenarios, music, video
// - portal lego cube
// - portal rendering
// - CONTACT_IGNORE: does it work or not?  => no
// - revisit handles, make them safer (using a struct)

// - pint caps for new joints

	#include "PxTransform.h"
	using namespace physx;
namespace
{
	inline_ const Point&	ToPoint(const PxVec3& p)	{ return (const Point&)p;	}
	inline_ const PxVec3&	ToPxVec3(const Point& p)	{ return (const PxVec3&)p;	}
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.x)==OFFSET_OF(PxQuat, x));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.y)==OFFSET_OF(PxQuat, y));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.z)==OFFSET_OF(PxQuat, z));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, w)==OFFSET_OF(PxQuat, w));
	inline_ const Quat&		ToQuat(const PxQuat& q)		{ return (const Quat&)q;	}
	inline_ const PxQuat&	ToPxQuat(const Quat& q)		{ return (const PxQuat&)q;	}

	inline_	const PxTransform	ToPxTransform(const PR& pose)
	{
		return PxTransform(ToPxVec3(pose.mPos), ToPxQuat(pose.mRot));
	}

	inline_	const PR	ToPR(const PxTransform& pose)
	{
		return PR(ToPoint(pose.p), ToQuat(pose.q));
	}
}

static const bool gDynamicPortals = false;
static const bool gUseContactIgnore = false;

	class PhysicsPortal
	{
		public:
			PhysicsPortal();
			~PhysicsPortal();

			void	Init(const Point& pos, const Quat& rot, const Point& extents, bool positive)
			{
				mPose.mPos = pos;
				mPose.mRot = rot;
				mExtents = extents;
				mPositive = positive;
			}

			PintActorHandle	CreateBackPortalGeom(Pint& pint)	const
			{
				if(gDynamicPortals)
					return null;

				const OBB Box = GetOBB();
//				const float epsilon = 0.5f;
//				Box.mExtents += Point(epsilon, epsilon, epsilon);

				{
					Point Pts[8];
					Box.ComputePoints(Pts);

					const udword* Indices = Box.GetTriangles();
					udword Indices2[36*2];
					udword* Dest = Indices2;
					const int KeepQuad[6] = {mPositive,0,!mPositive,0,0,0};
					for(udword i=0;i<6;i++)
					{
						{
							const udword vref0 = *Indices++;
							const udword vref1 = *Indices++;
							const udword vref2 = *Indices++;

							if(KeepQuad[i])
							{
								*Dest++ = vref0;
								*Dest++ = vref1;
								*Dest++ = vref2;
							}
						}

						{
							const udword vref0 = *Indices++;
							const udword vref1 = *Indices++;
							const udword vref2 = *Indices++;

							if(KeepQuad[i])
							{
								*Dest++ = vref0;
								*Dest++ = vref1;
								*Dest++ = vref2;
							}
						}
					}
					udword NbKeptTris = udword(Dest - Indices2)/3;

					PINT_MESH_CREATE MeshCreate;
					MeshCreate.SetSurfaceData(8, Pts, NbKeptTris, Indices2, null);
					MeshCreate.mRenderer = CreateMeshRenderer(MeshCreate.GetSurface());

					PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
					ObjectDesc.mMass		= 0.0f;
		//			ObjectDesc.mPosition	= portal.mPose.mPos;
		//			ObjectDesc.mRotation	= portal.mPose.mRot;
					return CreatePintObject(pint, ObjectDesc);
				}
			}

			PintActorHandle	CreatePortalGeom(Pint& pint)	const
			{
				const OBB Box = GetOBB();
//				const float epsilon = 0.5f;
//				Box.mExtents += Point(epsilon, epsilon, epsilon);

				if(1)
				{
					const PINT_MATERIAL_CREATE LowFrictionMaterial(0.0f, 0.2f, 0.0f);

					const float PortalBorderSize = 0.1f;
					const Point SideExtents(PortalBorderSize, Box.mExtents.y, PortalBorderSize);
					const Point TopBottomExtents(Box.mExtents.x + SideExtents.x*2.0f, PortalBorderSize, PortalBorderSize);

					PINT_BOX_CREATE BoxDescR(SideExtents);
					BoxDescR.mLocalPos.x	= Box.mExtents.x + SideExtents.x;
					BoxDescR.mRenderer		= CreateBoxRenderer(SideExtents);
					BoxDescR.mMaterial		= &LowFrictionMaterial;

					PINT_BOX_CREATE BoxDescL(SideExtents);
					BoxDescL.mLocalPos.x	= -Box.mExtents.x - SideExtents.x;
					BoxDescL.mRenderer		= BoxDescR.mRenderer;
					BoxDescL.mMaterial		= &LowFrictionMaterial;
					BoxDescR.SetNext(&BoxDescL);

					PINT_BOX_CREATE BoxDescT(TopBottomExtents);
					BoxDescT.mLocalPos.y	= Box.mExtents.y + TopBottomExtents.y;
					BoxDescT.mRenderer		= CreateBoxRenderer(TopBottomExtents);
					BoxDescT.mMaterial		= &LowFrictionMaterial;
					BoxDescL.SetNext(&BoxDescT);

					PINT_BOX_CREATE BoxDescB(TopBottomExtents);
					BoxDescB.mLocalPos.y	= -Box.mExtents.y - TopBottomExtents.y;
					BoxDescB.mRenderer		= BoxDescT.mRenderer;
					BoxDescB.mMaterial		= &LowFrictionMaterial;
					BoxDescT.SetNext(&BoxDescB);

/*					const Point BackExtents = Box.mExtents + Point(PortalBorderSize, PortalBorderSize, PortalBorderSize);
					PINT_BOX_CREATE BoxDescBack(BackExtents);
					BoxDescBack.mLocalPos.z	= PortalBorderSize*2.0f * (mPositive ? -1.0f : 1.0f);
					BoxDescBack.mRenderer	= CreateBoxRenderer(BackExtents);
					BoxDescBack.mMaterial	= &LowFrictionMaterial;
					BoxDescB.mNext			= &BoxDescBack;*/

					PINT_OBJECT_CREATE ObjectDesc(&BoxDescR);
					ObjectDesc.mMass		= gDynamicPortals ? 10.0f : 0.0f;
					ObjectDesc.mPosition	= Box.mCenter;
					ObjectDesc.mRotation	= Box.mRot;
					return CreatePintObject(pint, ObjectDesc);
				}
				else
				{
					Point Pts[8];
					Box.ComputePoints(Pts);

					const udword* Indices = Box.GetTriangles();
					udword Indices2[36*2];
					udword* Dest = Indices2;
					const int KeepQuad[6] = {0,1,0,1,1,1};
					for(udword i=0;i<6;i++)
					{
						{
							const udword vref0 = *Indices++;
							const udword vref1 = *Indices++;
							const udword vref2 = *Indices++;

							if(KeepQuad[i])
							{
								*Dest++ = vref0;
								*Dest++ = vref2;
								*Dest++ = vref1;
							}
						}

						{
							const udword vref0 = *Indices++;
							const udword vref1 = *Indices++;
							const udword vref2 = *Indices++;

							if(KeepQuad[i])
							{
								*Dest++ = vref0;
								*Dest++ = vref2;
								*Dest++ = vref1;
							}
						}
					}
					udword NbKeptTris = udword(Dest - Indices2)/3;
					Indices = Indices2;
					for(udword i=0;i<NbKeptTris;i++)
					{
						const udword vref0 = *Indices++;
						const udword vref1 = *Indices++;
						const udword vref2 = *Indices++;
						*Dest++ = vref0;
						*Dest++ = vref2;
						*Dest++ = vref1;
					}

					PINT_MESH_CREATE MeshCreate;
					MeshCreate.SetSurfaceData(8, Pts, NbKeptTris*2, Indices2, null);
					MeshCreate.mRenderer = CreateMeshRenderer(MeshCreate.GetSurface());

					PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
					ObjectDesc.mMass		= 0.0f;
		//			ObjectDesc.mPosition	= portal.mPose.mPos;
		//			ObjectDesc.mRotation	= portal.mPose.mRot;
					return CreatePintObject(pint, ObjectDesc);
				}
			}


			void	Render(PintRender& renderer)
			{
//				renderer.DrawBox(mExtents, mPose);

				const Point Color = mPositive ? Point(0.0f, 0.0f, 0.75f) : Point(1.0f, 0.75f, 0.0f);

				const OBB Box = GetOBB();

				Point Pts[8];
				Box.ComputePoints(Pts);

				const udword* Indices = Box.GetTriangles();
/*				for(udword i=0;i<12;i++)
				{
					const udword vref0 = *Indices++;
					const udword vref1 = *Indices++;
					const udword vref2 = *Indices++;
					renderer.DrawTriangle(Pts[vref0], Pts[vref1], Pts[vref2], Color);
				}*/

				{
					const int KeepQuad[6] = {!mPositive,0,mPositive,0,0,0};
					for(udword i=0;i<6;i++)
					{
						{
							const udword vref0 = *Indices++;
							const udword vref1 = *Indices++;
							const udword vref2 = *Indices++;

							if(KeepQuad[i])
							{
								renderer.DrawTriangle(Pts[vref0], Pts[vref1], Pts[vref2], Color);
							}
						}

						{
							const udword vref0 = *Indices++;
							const udword vref1 = *Indices++;
							const udword vref2 = *Indices++;

							if(KeepQuad[i])
							{
								renderer.DrawTriangle(Pts[vref0], Pts[vref1], Pts[vref2], Color);
							}
						}
					}
				}




				if(0)
				{
					const float Size = 5.0f;
//					renderer.DrawLine(Box.mCenter, Box.mCenter + Box.mRot[0]*Box.mExtents.x, Point(1.0f, 0.0f, 0.0f));
//					renderer.DrawLine(Box.mCenter, Box.mCenter + Box.mRot[1]*Box.mExtents.y, Point(0.0f, 1.0f, 0.0f));
//					renderer.DrawLine(Box.mCenter, Box.mCenter + Box.mRot[2]*Box.mExtents.z, Point(0.0f, 0.0f, 1.0f));
					renderer.DrawLine(Box.mCenter, Box.mCenter + Box.mRot[0]*Size, Point(1.0f, 0.0f, 0.0f));
					renderer.DrawLine(Box.mCenter, Box.mCenter + Box.mRot[1]*Size, Point(0.0f, 1.0f, 0.0f));
					renderer.DrawLine(Box.mCenter, Box.mCenter + Box.mRot[2]*Size, Point(0.0f, 0.0f, 1.0f));
				}
			}

			OBB		GetOBB()	const
			{
				return OBB(mPose.mPos, mExtents, mPose.mRot);
			}

			Plane	GetWorldPlane()	const
			{
				const Plane LocalPlane(0.0f, 0.0f, 1.0f, 0.0f);

				const Matrix4x4 TR = mPose;

				Plane PL;
				TransformPlane(PL, LocalPlane, TR);

				return PL;
			}

			PR		mPose;
			Point	mExtents;
			bool	mPositive;
	};

PhysicsPortal::PhysicsPortal()
{
	mPose.Identity();
	mExtents.Zero();
}

PhysicsPortal::~PhysicsPortal()
{
}


static bool gEnablePortalJoint = true;
static bool gReplicatePos = false;
static bool gReplicateRot = false;
static bool gReplicateLin = false;
static bool gReplicateAng = false;

static const char* gDesc_PortalJointAlone = "Portal joint alone";

class PortalJointBase : public TestBase
{
	public:
							PortalJointBase()			{									}
	virtual					~PortalJointBase()			{									}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS_PORTALS;	}
};

///////////////////////////////////////////////////////////////////////////////

static const Point gPortalSize(3.0f, 3.0f, 0.01f);

static const char* gDesc_PortalJointFinal = "Portal joint final";

class PortalJointFinal : public PortalJointBase, public PintContactModifyCallback
{
							PREVENT_COPY(PortalJointFinal)
	public:
	PortalJointFinal::PortalJointFinal() :
//		mExtents				(Point(1.0f, 0.5f, 0.25f)),
		mExtents				(Point(1.0f, 1.0f, 1.0f)),
		mOffset					(Point(0.0f, 0.0f, -4.0f)),
		mPortalGeomHandle0		(null),
		mPortalGeomHandle1		(null),
		mBackPortalGeomHandle0	(null),
		mBackPortalGeomHandle1	(null),
		mRenderer0				(null),
		mRenderer1				(null),
		mHandle0				(null),
		mHandle1				(null)
	{
		const Point Pos0(4.0f, gPortalSize.y, 0.0f);
		const Point Pos1(0.0f, gPortalSize.y+10.0f, -10.0f);

//		const Point Pos0(0.0f, gPortalSize.y, 0.0f);
//		const Point Pos1(0.0f, gPortalSize.y+10.0f, 00.0f);

		Quat Rot0;
		{
			Matrix3x3 Rot;
			Rot.Identity();
			Rot.RotY(-PI/8.0f);
//			Rot.RotYX(PI/4.0f, PI/4.0f);
//			Rot.RotX(PI/2.0f);
			Rot0 = Rot;
		}

		Quat Rot1;
		{
			Matrix3x3 Rot;
			Rot.Identity();
//			Rot.RotY(PI);
			Rot.RotY(PI/4.0f);
//			Rot.RotX(-PI/4.0f);
//			Rot.RotX(-PI/2.0f);
//			Rot.RotYX(PI/4.0f, PI/4.0f);
//			Rot.RotX(PI/2.0f);
			Rot1 = Rot;
		}

		mInitPose0 = PxTransform(ToPxVec3(Pos0), ToPxQuat(Rot0));
		mInitPose1 = PxTransform(ToPxVec3(Pos1), ToPxQuat(Rot1));
	}

	virtual					~PortalJointFinal()			{									}
	virtual	const char*		GetName()			const	{ return "PortalJointFinal";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_PortalJointFinal;	}

	Point				mOffset;

	PintActorHandle		mH0;
	PintActorHandle		mH1;

	virtual	bool	PrepContactModify(Pint& pint, udword nb_contacts, PintActorHandle h0, PintActorHandle h1, PintShapeHandle s0, PintShapeHandle s1)
	{
		mH0 = h0;
		mH1 = h1;
		return true;
	}

	virtual	ContactModif	ModifyDelayedContact(Pint& pint, const PR& pose0, const PR& pose1, Point& p, Point& n, float& s, udword feature0, udword feature1, udword index)
	{
		return CONTACT_IGNORE;
	}

	virtual	ContactModif	ModifyContact(Pint& pint, const PR& pose0, const PR& pose1, Point& p, Point& n, float& s, udword feature0, udword feature1)
	{
		if(		mH0 == mPortalGeomHandle0 || mH0 == mPortalGeomHandle1
			||	mH1 == mPortalGeomHandle0 || mH1 == mPortalGeomHandle1)
			return CONTACT_AS_IS;

		if(!mCanTeleport0 && !mCanTeleport1)
			return CONTACT_AS_IS;

		PhysicsPortal* Portal0 = &mPortal0;
		PhysicsPortal* Portal1 = &mPortal1;
		if(mCanTeleport1)
			TSwap(Portal0, Portal1);

		Plane PL;
		bool Positive;
		if(mH0==mHandle0 || mH1==mHandle0)
		{
			PL = Portal0->GetWorldPlane();
			Positive = Portal0->mPositive;
		}
		else if(mH0==mHandle1 || mH1==mHandle1)
		{
			PL = Portal1->GetWorldPlane();
			Positive = Portal1->mPositive;
		}
		else
			return CONTACT_AS_IS;

//		return CONTACT_IGNORE;

		const float Dist = PL.Distance(p);

		bool KeepContact = Positive ? Dist>=0.0f : Dist<=0.0f;

		if(		mH0 == mBackPortalGeomHandle0 || mH0 == mBackPortalGeomHandle1
			||	mH1 == mBackPortalGeomHandle0 || mH1 == mBackPortalGeomHandle1)
			KeepContact = false;

		if(KeepContact)
		{
			return CONTACT_AS_IS;
		}
		else
		{
			if(gUseContactIgnore)
				return CONTACT_IGNORE;

			// This one works
			s = 100000.0f;
			return CONTACT_MODIFY;
		}

		// #### this one doesn't work!!!!!!!!!!
		return CONTACT_IGNORE;
	}



	const Point			mExtents;

	PhysicsPortal		mPortal0;
	PhysicsPortal		mPortal1;
	PintActorHandle		mPortalGeomHandle0;
	PintActorHandle		mPortalGeomHandle1;
	PintActorHandle		mBackPortalGeomHandle0;
	PintActorHandle		mBackPortalGeomHandle1;

	PintDLMeshShapeRenderer2*	mRenderer0;
	PintDLMeshShapeRenderer2*	mRenderer1;

	PintActorHandle		mHandle0;
	PintActorHandle		mHandle1;
	PintJointHandle		mJointHandle;

	PxTransform			mInitPose0;
	PxTransform			mInitPose1;

	PxTransform			mDelta0to1;
	PxTransform			mDelta1to0;

//	bool				mDrawExpectedPose;

	virtual	bool	CommonSetup()
	{
		mPortalGeomHandle0 = null;
		mPortalGeomHandle1 = null;
		mHandle0 = null;
		mHandle1 = null;
		mJointHandle = null;
		mRenderer0 = null;
		mRenderer1 = null;

		mOverlap0 = false;
		mOverlap1 = false;
		mBehindPortal0 = false;
		mBehindPortal1 = false;
		mCanTeleport0 = false;
		mCanTeleport1 = false;

		return PortalJointBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		DELETESINGLE(mRenderer1);
		DELETESINGLE(mRenderer0);
		PortalJointBase::CommonRelease();
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		PortalJointBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(19.68f, 8.06f, -2.31f), Point(-0.95f, -0.27f, -0.13f));
		SetDefEnv(desc, true);
		desc.mContactModifyCallback	= this;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportPortalJoints || !caps.mSupportContactModifications)
			return false;

		const PxTransform& pose0 = mInitPose0;
		const PxTransform& pose1 = mInitPose1;

		const Point Pos0 = ToPoint(pose0.p);
		const Point Pos1 = ToPoint(pose1.p);
		const Quat Rot0 = ToQuat(pose0.q);
		const Quat Rot1 = ToQuat(pose1.q);

		mDelta0to1 = pose1 * pose0.getInverse();
		mDelta1to0 = pose0 * pose1.getInverse();

		{
			mPortal0.Init(Pos0, Rot0, gPortalSize, false);
			mPortalGeomHandle0 = mPortal0.CreatePortalGeom(pint);
			mBackPortalGeomHandle0 = mPortal0.CreateBackPortalGeom(pint);

			mPortal1.Init(Pos1, Rot1, gPortalSize, true);
			mPortalGeomHandle1 = mPortal1.CreatePortalGeom(pint);
			mBackPortalGeomHandle1 = mPortal1.CreateBackPortalGeom(pint);
		}

		const PINT_MATERIAL_CREATE LowFrictionMaterial(0.0f, 0.2f, 0.0f);

		PINT_BOX_CREATE BoxDesc(mExtents);
//		BoxDesc.mRenderer = CreateBoxRenderer(mExtents);
		{
			PintSurfaceInterface SI;
			mRenderer0 = ICE_NEW(PintDLMeshShapeRenderer2)(SI, DL_MESH_USE_ACTIVE_EDGES);
			BoxDesc.mRenderer = mRenderer0;

			mRenderer1 = ICE_NEW(PintDLMeshShapeRenderer2)(SI, DL_MESH_USE_ACTIVE_EDGES);
		}
		BoxDesc.mMaterial = &LowFrictionMaterial;

		const float Mass = 1.0f;
		mHandle0 = CreateSimpleObject(pint, &BoxDesc, Mass, Pos0+mOffset, &Rot0);

		return true;
	}

	PxTransform mExpectedPose0to1;
	PxTransform mExpectedPose1to0;
//	PxTransform	mLastFreePose;
	bool	mOverlap0;
	bool	mOverlap1;
	bool	mBehindPortal0;
	bool	mBehindPortal1;
	bool	mCanTeleport0;
	bool	mCanTeleport1;

	void	CreateShadowObject(Pint& pint, const PxTransform& pose, const PxTransform& delta, const Point& linVel, const Point& angVel)
	{
		if(!mHandle1)
		{
			const PINT_MATERIAL_CREATE LowFrictionMaterial(0.0f, 0.0f, 0.0f);

			PINT_BOX_CREATE BoxDesc(mExtents);
//			BoxDesc.mRenderer = CreateBoxRenderer(mExtents);
			{
				BoxDesc.mRenderer = mRenderer1;
			}
			BoxDesc.mMaterial = &LowFrictionMaterial;

			const float Mass = 1.0f;
			mHandle1 = CreateSimpleObject(pint, &BoxDesc, Mass, ToPoint(pose.p), &ToQuat(pose.q), &linVel, &angVel);

			if(1)
			{
				PINT_PORTAL_JOINT_CREATE Desc;
				Desc.mObject0	= mHandle0;
				Desc.mObject1	= mHandle1;
				Desc.mRelPose	= ToPR(delta);
				mJointHandle	= pint.CreateJoint(Desc);
				ASSERT(mJointHandle);
			}
		}
	}

	void	ReleaseShadowObject(Pint& pint)
	{
		if(mHandle1)
		{
			ReleasePintObject(pint, mHandle1, true);
			mHandle1 = null;

			// NEW: ReleasePintObject now automatically releases the joint...
			//pint.ReleaseJoint(mJointHandle);
			mJointHandle = null;
		}
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		struct Local
		{
			static bool UpdateRenderer(Pint& pint, PintActorHandle handle, const OBB& ObjectBox, const PhysicsPortal& portal, PintDLMeshShapeRenderer2* renderer, bool clip)
			{
				Point Pts[8];
				ObjectBox.ComputePoints(Pts);

				const udword* Tris = ObjectBox.GetTriangles();
				Triangle T[12];
				for(udword i=0;i<12;i++)
				{
					T[i].mVerts[0] = Pts[*Tris++];
					T[i].mVerts[1] = Pts[*Tris++];
					T[i].mVerts[2] = Pts[*Tris++];
				}

				TriSurface TS;
				TS.Init(12, T);

				if(!clip)
				{
					if(renderer)
						renderer->Init(TS);
//					pint.EnableGravity(handle, true);
					Pint_Actor* API  = pint.GetActorAPI();
					if(API)
						API->SetGravityFlag(handle, true);
					return true;
				}

				const Plane PL = portal.GetWorldPlane();

				TriSurface Surface;

				if(portal.mPositive)
					TS.Cut(PL, &Surface, null);
				else
					TS.Cut(PL, null, &Surface);

				if(renderer)
					renderer->Init(Surface);

				const udword NbFaces = Surface.GetNbFaces();
				//### this is wrong, we can have more than 12 faces
				bool gravity;
				if(NbFaces==12)
				{
					gravity = true;
				}
				else if(NbFaces==0)
				{
					gravity = false;
				}
				else
				{
					const float d = PL.Distance(ObjectBox.mCenter);
					if(portal.mPositive)
					{
						gravity = d>=0.0f;
					}
					else
					{
						gravity = d<=0.0f;
					}

//					const float d = PL.Distance(ObjectBox.mCenter) * portal.mPositive ? -1.0f : 1.0f;
//					const float d = PL.Distance(ObjectBox.mCenter) * portal.mPositive ? 1.0f : -1.0f;
//					pint.EnableGravity(handle, d>=0.0f);
//					pint.EnableGravity(handle, true);
//					printf("%d\n", d>=0.0f);
				}
//				pint.EnableGravity(handle, gravity);
				Pint_Actor* API  = pint.GetActorAPI();
				if(API)
					API->SetGravityFlag(handle, gravity);
				return gravity;
			}
		};

		if(gDynamicPortals)
		{
			if(mPortalGeomHandle0)
				mPortal0.mPose = pint.GetWorldTransform(mPortalGeomHandle0);

			if(mPortalGeomHandle1)
				mPortal1.mPose = pint.GetWorldTransform(mPortalGeomHandle1);

			const PxTransform pose0 = ToPxTransform(mPortal0.mPose);
			const PxTransform pose1 = ToPxTransform(mPortal1.mPose);

			mDelta0to1 = pose1 * pose0.getInverse();
			mDelta1to0 = pose0 * pose1.getInverse();

			if(mJointHandle)
			{
				if(mCanTeleport0)
					pint.SetPortalJointRelativePose(mJointHandle, ToPR(mDelta0to1));
				else if(mCanTeleport1)
					pint.SetPortalJointRelativePose(mJointHandle, ToPR(mDelta1to0));
			}

		}

		if(mHandle0)
		{
			const PR CurrentPR = pint.GetWorldTransform(mHandle0);

			const OBB ObjectBox(CurrentPR.mPos, mExtents, CurrentPR.mRot);

			const OBB PortalBox0 = mPortal0.GetOBB();
			const OBB PortalBox1 = mPortal1.GetOBB();

			const Plane PortalPlane0 = mPortal0.GetWorldPlane();
			const Plane PortalPlane1 = mPortal1.GetWorldPlane();

			mOverlap0 = OBBOBBOverlap(ObjectBox, PortalBox0);
			mOverlap1 = OBBOBBOverlap(ObjectBox, PortalBox1);

			mBehindPortal0 = PortalPlane0.Distance(ObjectBox.mCenter)>0.0f;
			if(mPortal0.mPositive)
				mBehindPortal0 = !mBehindPortal0;
			mBehindPortal1 = PortalPlane1.Distance(ObjectBox.mCenter)>0.0f;
			if(mPortal1.mPositive)
				mBehindPortal1 = !mBehindPortal1;

			const PxTransform CurrentPose(ToPxVec3(CurrentPR.mPos), ToPxQuat(CurrentPR.mRot));

			mExpectedPose0to1 = mDelta0to1 * CurrentPose;
			mExpectedPose1to0 = mDelta1to0 * CurrentPose;


/*			if(!mOverlap0 && !mBehindPortal0 && !mOverlap1 && !mBehindPortal1)
			{
				mLastFreePose = CurrentPose;
			}*/

			if(!mOverlap0 && !mBehindPortal0 && !mOverlap1 && !mBehindPortal1)
			{
				ReleaseShadowObject(pint);
			}

			if(!mOverlap0 && !mBehindPortal0)
			{
				mCanTeleport0 = false;
			}
			if(!mOverlap1 && !mBehindPortal1)
			{
				mCanTeleport1 = false;
			}

			if(mOverlap0 && !mBehindPortal0)
				mCanTeleport0 = true;
			if(mOverlap1 && !mBehindPortal1)
				mCanTeleport1 = true;

//			const Point LinVel = pint.GetWorldLinearVelocity(mHandle0);
//			const Point AngVel = pint.GetWorldAngularVelocity(mHandle0);
			Pint_Actor* API  = pint.GetActorAPI();
			Point LinVel(0.0f, 0.0f, 0.0f);
			Point AngVel(0.0f, 0.0f, 0.0f);
			if(API)
			{
				API->GetLinearVelocity(mHandle0, LinVel, true);
				API->GetAngularVelocity(mHandle0, AngVel, true);
			}

//			if(mOverlap0 && !mHandle1)
			if(mCanTeleport0 && !mHandle1)
			{
				const PxVec3 ExpectedLinVel = mDelta0to1.rotate(ToPxVec3(LinVel));
				const PxVec3 ExpectedAngVel = mDelta0to1.rotate(ToPxVec3(AngVel));

				CreateShadowObject(pint, mExpectedPose0to1, mDelta0to1, ToPoint(ExpectedLinVel), ToPoint(ExpectedAngVel));
			}

//			if(mOverlap1 && !mHandle1)
			if(mCanTeleport1 && !mHandle1)
			{
				const PxVec3 ExpectedLinVel = mDelta1to0.rotate(ToPxVec3(LinVel));
				const PxVec3 ExpectedAngVel = mDelta1to0.rotate(ToPxVec3(AngVel));

				CreateShadowObject(pint, mExpectedPose1to0, mDelta1to0, ToPoint(ExpectedLinVel), ToPoint(ExpectedAngVel));
			}

			if(mCanTeleport0 && !mOverlap0 && mBehindPortal0)
			{
				mCanTeleport0 = false;
				mCanTeleport1 = false;

				ReleaseShadowObject(pint);

				pint.SetWorldTransform(mHandle0, ToPR(mExpectedPose0to1));

				const PxVec3 ExpectedLinVel = mDelta0to1.rotate(ToPxVec3(LinVel));
				const PxVec3 ExpectedAngVel = mDelta0to1.rotate(ToPxVec3(AngVel));

//				pint.SetWorldLinearVelocity(mHandle0, ToPoint(ExpectedLinVel));
//				pint.SetWorldAngularVelocity(mHandle0, ToPoint(ExpectedAngVel));
				if(API)
				{
					API->SetLinearVelocity(mHandle0, ToPoint(ExpectedLinVel), true);
					API->SetAngularVelocity(mHandle0, ToPoint(ExpectedAngVel), true);
				}
			}

			if(mCanTeleport1 && !mOverlap1 && mBehindPortal1)
			{
				mCanTeleport0 = false;
				mCanTeleport1 = false;

				ReleaseShadowObject(pint);

				pint.SetWorldTransform(mHandle0, ToPR(mExpectedPose1to0));

				const PxVec3 ExpectedLinVel = mDelta1to0.rotate(ToPxVec3(LinVel));
				const PxVec3 ExpectedAngVel = mDelta1to0.rotate(ToPxVec3(AngVel));

//				pint.SetWorldLinearVelocity(mHandle0, ToPoint(ExpectedLinVel));
//				pint.SetWorldAngularVelocity(mHandle0, ToPoint(ExpectedAngVel));
				if(API)
				{
					API->SetLinearVelocity(mHandle0, ToPoint(ExpectedLinVel), true);
					API->SetAngularVelocity(mHandle0, ToPoint(ExpectedAngVel), true);
				}
			}

/*			if(mHandle0)
			{
				const PR Pose = pint.GetWorldTransform(mHandle0);
				const OBB ObjectBox(Pose.mPos, mExtents, Pose.mRot);
				Local::UpdateRenderer(pint, mHandle0, ObjectBox, mPortal0, mRenderer0, true);
			}
			if(mHandle1)
			{
				const PR Pose = pint.GetWorldTransform(mHandle1);
				const OBB ObjectBox(Pose.mPos, mExtents, Pose.mRot);
				Local::UpdateRenderer(pint, mHandle1, ObjectBox, mPortal1, mRenderer1, true);
			}*/

			if(mCanTeleport0||mCanTeleport1)
			{
				{
					const PR Pose = pint.GetWorldTransform(mHandle0);
					const OBB ObjectBox(Pose.mPos, mExtents, Pose.mRot);
					if(mCanTeleport0)
						Local::UpdateRenderer(pint, mHandle0, ObjectBox, mPortal0, mRenderer0, true);
					else
						Local::UpdateRenderer(pint, mHandle0, ObjectBox, mPortal1, mRenderer0, true);
				}

				if(mHandle1)
				{
					const PR Pose = pint.GetWorldTransform(mHandle1);
					const OBB ObjectBox(Pose.mPos, mExtents, Pose.mRot);
					if(mCanTeleport0)
						Local::UpdateRenderer(pint, mHandle1, ObjectBox, mPortal1, mRenderer1, true);
					else
						Local::UpdateRenderer(pint, mHandle1, ObjectBox, mPortal0, mRenderer1, true);
				}
			}
			else
			{
				const PR Pose = pint.GetWorldTransform(mHandle0);
				const OBB ObjectBox(Pose.mPos, mExtents, Pose.mRot);
				Local::UpdateRenderer(pint, mHandle0, ObjectBox, mPortal0, mRenderer0, false);

				TriSurface TS;
				if(mRenderer1)
					mRenderer1->Init(TS);
			}
		}

		return PortalJointBase::Update(pint, dt);
	}

	virtual	void	CommonDebugRender(PintRender& renderer)
	{
		if(0)
		{
			renderer.DrawWireframeOBB(OBB(ToPoint(mExpectedPose0to1.p), mExtents, ToQuat(mExpectedPose0to1.q)), Point(1.0f, 0.75f, 0.0f));
			renderer.DrawWireframeOBB(OBB(ToPoint(mExpectedPose1to0.p), mExtents, ToQuat(mExpectedPose1to0.q)), Point(0.0f, 0.0f, 0.75f));
		}

//		renderer.DrawWirefameOBB(OBB(ToPoint(mLastFreePose.p), mExtents, ToQuat(mLastFreePose.q)), Point(0.0f, 1.0f, 0.0f));

		mPortal0.Render(renderer);	// Yellow
		mPortal1.Render(renderer);	// Blue

		PortalJointBase::CommonDebugRender(renderer);
	}

	virtual	void	CommonRender(PintRender& renderer, PintRenderPass render_pass)
	{
		if(render_pass==PINT_RENDER_PASS_SHADOW)
		{
			mPortal0.Render(renderer);	// Yellow
			mPortal1.Render(renderer);	// Blue
		}
		PortalJointBase::CommonRender(renderer, render_pass);
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		renderer.print(0.0f, y, text_scale, _F("State 0: %d, %d", mOverlap0, mBehindPortal0));
		y -= text_scale;

		renderer.print(0.0f, y, text_scale, _F("State 1: %d, %d", mOverlap1, mBehindPortal1));
		y -= text_scale;
		return y;
	}

END_TEST(PortalJointFinal)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PortalScene1 = "Portal scene 1";

class PortalScene1 : public PortalJointFinal
{
							PREVENT_COPY(PortalScene1)
	public:
	PortalScene1::PortalScene1()
	{
		const Point Pos0(0.0f, gPortalSize.y, 0.0f);
		const Point Pos1(0.0f, gPortalSize.y+10.0f, 0.0f);

		Quat Rot0;
		{
			Matrix3x3 Rot;
			Rot.RotX(PI/2.0f);
			Rot0 = Rot;
		}

		Quat Rot1;
		{
			Matrix3x3 Rot;
			Rot.RotX(PI/2.0f);
			Rot1 = Rot;
		}

		mInitPose0 = PxTransform(ToPxVec3(Pos0), ToPxQuat(Rot0));
		mInitPose1 = PxTransform(ToPxVec3(Pos1), ToPxQuat(Rot1));
		//mOffset = Point(0.0f, 4.0f, 3.0f);
		mOffset = Point(0.0f, 4.0f, 0.0f);
	}

	virtual					~PortalScene1()				{								}
	virtual	const char*		GetName()			const	{ return "PortalScene1";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_PortalScene1;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		PortalJointBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(19.68f, 8.06f, -2.31f), Point(-0.99f, -0.05f, 0.13f));
		SetDefEnv(desc, true);
		desc.mContactModifyCallback	= this;
	}

END_TEST(PortalScene1)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PortalScene2 = "Portal scene 2";

class PortalScene2 : public PortalJointFinal
{
							PREVENT_COPY(PortalScene2)
	public:
	PortalScene2::PortalScene2()
	{
		const Point Pos0(0.0f, gPortalSize.y, 0.0f);
		const Point Pos1(0.0f, gPortalSize.y, 10.0f);

		Quat Rot0;
		{
			Matrix3x3 Rot;
			Rot.RotX(PI/2.0f);
			Rot0 = Rot;
		}

		Quat Rot1;
		{
			Matrix3x3 Rot;
			Rot.RotX(-PI/2.0f);
			Rot1 = Rot;
		}

		mInitPose0 = PxTransform(ToPxVec3(Pos0), ToPxQuat(Rot0));
		mInitPose1 = PxTransform(ToPxVec3(Pos1), ToPxQuat(Rot1));
		mOffset = Point(0.0f, 4.0f, 0.0f);
	}

	virtual					~PortalScene2()				{								}
	virtual	const char*		GetName()			const	{ return "PortalScene2";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_PortalScene2;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		PortalJointBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(19.68f, 8.06f, -2.31f), Point(-0.91f, -0.21f, 0.35f));
		SetDefEnv(desc, true);
		desc.mContactModifyCallback	= this;
	}

END_TEST(PortalScene2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PortalScene3 = "Portal scene 3";

class PortalScene3 : public PortalJointFinal
{
							PREVENT_COPY(PortalScene3)
	public:
	PortalScene3::PortalScene3()
	{
		const Point Pos0(0.0f, gPortalSize.y, 0.0f);
		const Point Pos1(0.0f, gPortalSize.y, 10.0f);

		Quat Rot0;
		{
			Matrix3x3 Rot;
			Rot.RotX(PI/2.0f);
			Rot0 = Rot;
		}

		Quat Rot1;
		{
			Matrix3x3 Rot0;
			Rot0.RotX(-PI/4.0f);

			Matrix3x3 Rot2;
			Rot2.RotY(PI/2.0f);

			Rot1 = Rot0 * Rot2;
		}

		mInitPose0 = PxTransform(ToPxVec3(Pos0), ToPxQuat(Rot0));
		mInitPose1 = PxTransform(ToPxVec3(Pos1), ToPxQuat(Rot1));
		mOffset = Point(0.0f, 8.0f, 0.0f);
	}

	virtual					~PortalScene3()				{								}
	virtual	const char*		GetName()			const	{ return "PortalScene3";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_PortalScene3;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		PortalJointBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(9.86f, 11.18f, 23.34f), Point(-0.43f, -0.33f, -0.84f));
		SetDefEnv(desc, true);
		desc.mContactModifyCallback	= this;
	}

END_TEST(PortalScene3)
