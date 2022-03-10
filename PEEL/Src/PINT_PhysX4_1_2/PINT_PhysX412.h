///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_PHYSX411_H
#define PINT_PHYSX411_H

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

	IceWindow*	PhysX_InitGUI(IceWidget* parent, PintGUIHelper& helper);
	void		PhysX_CloseGUI();
	void		PhysX_Init(const PINT_WORLD_CREATE& desc);
	void		PhysX_Close();
	PhysX*		GetPhysX();

	extern "C"	__declspec(dllexport)	PintPlugin*	GetPintPlugin();

#endif