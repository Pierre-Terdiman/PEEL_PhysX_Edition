///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_SCENES_HELPERS_H
#define TEST_SCENES_HELPERS_H

#include "Pint.h"

//	static const float gValveScale = 0.01f;
	static const float gValveScale = 1.0f;

	static const float gSQMaxDist = 1000.0f;
//	static const float gSQMaxDist = 10.0f;

	inline_	float OneOverNb(udword nb)	{ return nb>1 ? 1.0f / float(nb-1) : 0.0f;	}

	class TestBase;
	class SurfaceManager;

	const char*			GetFile								(const char* filename, udword& size);

	void				GenerateCirclePts					(udword nb, Point* pts, float scale, float y);
	udword				GenerateConvex						(Point* verts, udword nbInsideCirclePts, udword nbOutsideCirclePts, float scale0, float scale1, float z);

	PintActorHandle		CreateSimpleObject					(Pint& pint, const PINT_SHAPE_CREATE* shape, float mass, const Point& pos, const Quat* rot=null, const Point* linVel=null, const Point* angVel=null, const char* name=null);
	PintActorHandle		CreateDynamicObject					(Pint& pint, const PINT_SHAPE_CREATE* shape, const Point& pos, const Quat* rot=null, const Point* linVel=null, const Point* angVel=null, const char* name=null);
	PintActorHandle		CreateStaticObject					(Pint& pint, const PINT_SHAPE_CREATE* shape, const Point& pos, const Quat* rot=null, const char* name=null);
	PintActorHandle		CreateDynamicBox					(Pint& pint, float size_x, float size_y, float size_z, const Point& pos, const Quat* rot=null, const PINT_MATERIAL_CREATE* material=null);

	void				CreateSingleTriangleMesh			(SurfaceManager& test, float scale, Triangle* tri=null, bool reverse_winding=false);
	bool				CreateBoxStack						(Pint& pint, const PintCaps& caps, const udword nb_stacks, udword nb_base_boxes, bool use_convexes=false);
	void				CreateBoxContainer					(Pint& pint, float box_height, float box_side, float box_depth);

	// TODO: refactor CreateSeaOfStaticXXX functions
	bool				CreateSeaOfStaticBoxes				(Pint& pint, float amplitude, udword nb_x, udword nb_y, float altitude, bool add_to_scene=true, PtrContainer* created_objects=null);
	bool				CreateSeaOfStaticSpheres			(Pint& pint, float amplitude, udword nb_x, udword nb_y, float altitude);
	bool				CreateSeaOfStaticCapsules			(Pint& pint, float amplitude, udword nb_x, udword nb_y, float altitude);
	bool				CreateSeaOfStaticConvexes			(Pint& pint, const PintCaps& caps, udword nb_x, udword nb_y, float altitude);

	bool				CreateTestScene_ConvexStack_Generic	(Pint& pint, udword NbX, udword NbY, udword NbLayers, udword NbInsideCirclePts, udword NbOutsideCirclePts);

	bool				CreateArrayOfDynamicConvexes		(Pint& pint, const PINT_CONVEX_CREATE& convex_create, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point* offset=null, const Point* lin_vel=null);
	// TODO: refactor GenerateArrayOfXXX functions
	bool				GenerateArrayOfSpheres				(Pint& pint, float radius, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_z, float mass=1.0f, PintCollisionGroup group=0, const Point* offset=null);
	bool				GenerateArrayOfBoxes				(Pint& pint, const Point& extents, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_z, float mass=1.0f, PintCollisionGroup group=0, const Point* offset=null);
	bool				GenerateArrayOfCapsules				(Pint& pint, float radius, float half_height, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_z, float mass=1.0f, PintCollisionGroup group=0, const Point* offset=null);
	bool				GenerateArrayOfConvexes				(Pint& pint, const PintCaps& caps, bool is_static, float scale, udword nb_x, udword nb_y, udword convex_id);
	bool				GenerateArrayOfObjects				(Pint& pint, const PintCaps& caps, PintShape type, udword type_data, udword nb_x, udword nb_y, float altitude, float scale, float mass);

	bool				GenerateGroundObstacles				(Pint& pint, float spacing, float size, bool ramp);

	void				GenerateConvexPile					(Pint& pint, udword nb_x, udword nb_y, udword nb_layers, float amplitude, udword nb_random_pts, float scatter);

	bool				Setup_PotPourri_Raycasts			(Pint& pint, const PintCaps& caps, float mass, udword nb_layers, udword nb_x, udword nb_y);
	bool				Setup_PotPourri_Raycasts			(TestBase& test, udword nb_rays, float max_dist);

	void				RegisterArrayOfRaycasts				(TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, float max_dist, const Point& offset);
	bool				GenerateArrayOfVerticalRaycasts		(TestBase& test, float scale, udword nb_x, udword nb_y, float max_dist);
	udword				DoBatchRaycasts						(TestBase& test, Pint& pint);

	void				RegisterArrayOfBoxSweeps			(TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, const Point& extents, const Point& offset, float max_dist);
	bool				GenerateArrayOfVerticalBoxSweeps	(TestBase& test, float scale, udword nb_x, udword nb_y, float max_dist);
	udword				DoBatchBoxSweeps					(TestBase& test, Pint& pint);
	void				UpdateBoxSweeps						(TestBase& test, float angle);

	void				RegisterArrayOfSphereSweeps			(TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, float radius, const Point& offset, float max_dist);
	bool				GenerateArrayOfVerticalSphereSweeps	(TestBase& test, float scale, udword nb_x, udword nb_y, float max_dist);
	udword				DoBatchSphereSweeps					(TestBase& test, Pint& pint);

	void				RegisterArrayOfCapsuleSweeps		(TestBase& test, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point& dir, float radius, float half_height, const Point& offset, float max_dist);
	bool				GenerateArrayOfVerticalCapsuleSweeps(TestBase& test, float scale, udword nb_x, udword nb_y, float max_dist);
	udword				DoBatchCapsuleSweeps				(TestBase& test, Pint& pint);
	void				UpdateCapsuleSweeps					(TestBase& test, float angle);

	udword				DoBatchConvexSweeps					(TestBase& test, Pint& pint);
	void				UpdateConvexSweeps					(TestBase& test, float angle);

	enum BatchOverlapMode
	{
		OVERLAP_ANY,
		OVERLAP_OBJECTS,
	};

	udword				DoBatchSphereOverlaps				(TestBase& test, Pint& pint, BatchOverlapMode mode);
	udword				DoBatchBoxOverlaps					(TestBase& test, Pint& pint, BatchOverlapMode mode);
	udword				DoBatchCapsuleOverlaps				(TestBase& test, Pint& pint, BatchOverlapMode mode);
	udword				DoBatchConvexOverlaps				(TestBase& test, Pint& pint, BatchOverlapMode mode);

	void				LoadRaysFile						(TestBase& test, const char* filename, bool only_rays, bool no_processing=false);

	#define MAX_LINKS	1024
	//#define SETUP_ROPE_COLLISION_GROUPS
/*	bool CreateCapsuleRope(	Pint& pint, PintActorHandle articulation, PintActorHandle parent,
							float capsule_radius, float half_height, float capsule_mass, float capsule_mass_for_inertia,
							const Point& capsule_dir, const Point& start_pos, udword nb_capsules,
							bool create_distance_constraints, PintActorHandle* handles, const PintCollisionGroup* collision_groups=null);*/
	bool CreateCapsuleRope2(Pint& pint, float& half_height, PintArticHandle articulation, PintActorHandle parent,
							const Point& p0, const Point& p1, udword nb_capsules,
							float capsule_radius, float capsule_mass, float capsule_mass_for_inertia,
							bool /*overlapping_links*/, bool create_distance_constraints, PintActorHandle* handles, const PintCollisionGroup* collision_groups=null);

	/////

	struct CaterpillarTrackObjects : public Allocateable
	{
		udword			mNbGears;
		PintActorHandle	mGears[4];
		PintActorHandle	mChassis;

		void			OnObjectReleased(PintActorHandle removed_object)
		{
			if(mChassis==removed_object)
				mChassis=null;
			for(udword i=0;i<mNbGears;i++)
			{
				if(mGears[i]==removed_object)
					mGears[i]=null;
			}
		}

	};

	class CylinderMesh;
	// WIP & OLD!
	void CreateCaterpillarTrack_OLD(Pint& pint, CaterpillarTrackObjects& objects, const PINT_MATERIAL_CREATE* material, const Point& pos,
									const CylinderMesh& mCylinder, PintShapeRenderer* cylinder_renderer,
									const CylinderMesh& mCylinder2, PintShapeRenderer* cylinder_renderer2,
									udword multiplier=32);

	/////

	struct BRIDGES_CREATE
	{
		BRIDGES_CREATE() :
			mNbBridges		(0),
			mNbPlanks		(0),
			mPlankMass		(1.0f),
			mPlankExtents	(Point(0.0f, 0.0f, 0.0f)),
			mOrigin			(Point(0.0f, 0.0f, 0.0f)),
			mInertiaCoeff	(-1.0f),
			mMultiplier		(1),
			mIncludeSupport	(false)
		{
		}

		udword	mNbBridges;
		udword	mNbPlanks;
		float	mPlankMass;
		Point	mPlankExtents;
		Point	mOrigin;
		float	mInertiaCoeff;
		udword	mMultiplier;
		bool	mIncludeSupport;
	};

	bool CreateBridges(Pint& pint, const PintCaps& caps, BRIDGES_CREATE& create);

	/////

	bool ClampAngularVelocity(Pint& pint, PintActorHandle wheel_parent, PintActorHandle wheel, float max_velocity);

	class GLFontRenderer;
	float PrintAngularVelocity(Pint& pint, GLFontRenderer& renderer, PintActorHandle handle, float y, float text_scale);

	bool GetHingeTwistAngle(Pint& pint, PintJointHandle handle, float& angle);

#endif