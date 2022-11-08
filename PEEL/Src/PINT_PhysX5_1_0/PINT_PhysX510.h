///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_PHYSX510_H
#define PINT_PHYSX510_H

#include "..\Pint.h"
#include "..\PINT_Common\PINT_CommonPhysX3.h"
#include "..\PINT_Common\PINT_CommonPhysX3_Vehicles.h"

	class PhysX : public SharedPhysX_Vehicles
	{
		public:
										PhysX(const EditableParams& params);
		virtual							~PhysX();

		// Pint
		virtual	const char*				GetName()				const;
		virtual	const char*				GetUIName()				const;
		virtual	void					GetCaps(PintCaps& caps)	const;
		virtual	void					Init(const PINT_WORLD_CREATE& desc);
//		virtual	PintSceneHandle			Init(const PINT_WORLD_CREATE& desc);
		virtual	void					Close();
		virtual	udword					Update(float dt);
		virtual	Point					GetMainColor();

		virtual	void					TestNewFeature();
		//~Pint

				void					UpdateFromUI();
		private:
//				PxProfileZoneManager*	mProfileZoneManager;
				PxPvd*					mPVD;
				PxPvdTransport*			mTransport;
	};

	class PhysX_CQS : public PhysX
	{
		public:
										PhysX_CQS(const EditableParams& params)	: PhysX(params)	{}
		virtual							~PhysX_CQS()											{}

		virtual	void					Close();

		virtual	udword					BatchRaycasts				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)	override;
		virtual	udword					BatchRaycastAny				(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)	override;

		virtual	udword					BatchSphereOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)	override;
		virtual	udword					BatchBoxOverlapAny			(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)		override;
		virtual	udword					BatchCapsuleOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)	override;

		virtual	udword					BatchSphereOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)	override;
		virtual	udword					BatchBoxOverlapObjects		(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)		override;
		virtual	udword					BatchCapsuleOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)	override;

		virtual	udword					BatchBoxSweeps				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)		override;
		virtual	udword					BatchSphereSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)	override;
		virtual	udword					BatchCapsuleSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)	override;
		virtual	udword					BatchConvexSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)	override;

/*
		virtual	udword					BatchRaycastAll				(PintSQThreadContext context, udword nb, PintMultipleObjectsHit* dest, Container& stream, const PintRaycastData* raycasts);

		virtual	udword					FindTriangles_MeshSphereOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintSphereOverlapData* overlaps);
		virtual	udword					FindTriangles_MeshBoxOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintBoxOverlapData* overlaps);
		virtual	udword					FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintCapsuleOverlapData* overlaps);
*/


		virtual	udword					FindTriangles_MeshMeshOverlap	(PintSQThreadContext context, PintActorHandle handle0, PintActorHandle handle1, Container& results);

//				PxArray<PxMeshQuery::IndexPair>	mOverlaps;
	};

	IceWindow*	PhysX_InitGUI(IceWidget* parent, PintGUIHelper& helper);
	void		PhysX_CloseGUI();
	void		PhysX_Init(const PINT_WORLD_CREATE& desc);
	void		PhysX_Close();
	PhysX*		GetPhysX();

	extern "C"	__declspec(dllexport)	PintPlugin*	GetPintPlugin();

#endif