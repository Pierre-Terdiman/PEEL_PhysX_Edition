///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_JOINT_PARAMS_H
#define GUI_JOINT_PARAMS_H

#include "PintDef.h"

	class EditBoxInterface;
	class Pint_Joint;
	class PEEL_EditBox;

	class GUI_1DLimitJointParams
	{
		public:
						GUI_1DLimitJointParams();
						~GUI_1DLimitJointParams();

		udword			Init(EditBoxInterface& owner, IceWidget* parent, udword y);

		void			InitFrom(Pint_Joint* joint_api, PintJointHandle handle, const char* min_label, const char* max_label, udword index);
		void			SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors, udword index);
		void			Cancel();
		bool			SomethingChanged()	const;
		void			SetVisible(bool is_visible);

		PEEL_EditBox*	mEditBox_MinLimit;
		PEEL_EditBox*	mEditBox_MaxLimit;
	};

	class GUI_JointParams : public Allocateable
	{
		public:
						GUI_JointParams();
		virtual			~GUI_JointParams();

		virtual	udword	Init(EditBoxInterface& owner, IceWidget* parent, udword y)						= 0;
		virtual	void	InitFrom(Pint_Joint* joint_api, PintJointHandle handle)							= 0;
		virtual	void	SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)	= 0;
		virtual	void	Cancel()																		= 0;
		virtual	bool	SomethingChanged()														const	= 0;
		virtual	void	SetVisible(bool is_visible)														= 0;
	};

	class GUI_SphericalJointParams : public GUI_JointParams
	{
		public:
										GUI_SphericalJointParams();
		virtual							~GUI_SphericalJointParams();

		// GUI_JointParams
		virtual	udword					Init(EditBoxInterface& owner, IceWidget* parent, udword y)						override;
		virtual	void					InitFrom(Pint_Joint* joint_api, PintJointHandle handle)							override;
		virtual	void					SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)	override;
		virtual	void					Cancel()																		override;
		virtual	bool					SomethingChanged()														const	override;
		virtual	void					SetVisible(bool is_visible)														override;
		//~GUI_JointParams

				GUI_1DLimitJointParams	mLimit;
	};

	class GUI_HingeJointParams : public GUI_JointParams
	{
		public:
										GUI_HingeJointParams();
		virtual							~GUI_HingeJointParams();

		// GUI_JointParams
		virtual	udword					Init(EditBoxInterface& owner, IceWidget* parent, udword y)						override;
		virtual	void					InitFrom(Pint_Joint* joint_api, PintJointHandle handle)							override;
		virtual	void					SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)	override;
		virtual	void					Cancel()																		override;
		virtual	bool					SomethingChanged()														const	override;
		virtual	void					SetVisible(bool is_visible)														override;
		//~GUI_JointParams

				GUI_1DLimitJointParams	mLimit;
	};

	class GUI_PrismaticJointParams : public GUI_JointParams
	{
		public:
										GUI_PrismaticJointParams();
		virtual							~GUI_PrismaticJointParams();

		// GUI_JointParams
		virtual	udword					Init(EditBoxInterface& owner, IceWidget* parent, udword y)						override;
		virtual	void					InitFrom(Pint_Joint* joint_api, PintJointHandle handle)							override;
		virtual	void					SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)	override;
		virtual	void					Cancel()																		override;
		virtual	bool					SomethingChanged()														const	override;
		virtual	void					SetVisible(bool is_visible)														override;
		//~GUI_JointParams

				GUI_1DLimitJointParams	mLimit;
				PEEL_EditBox*			mEditBox_SpringStiffness;
				PEEL_EditBox*			mEditBox_SpringDamping;
	};

	class GUI_DistanceJointParams : public GUI_JointParams
	{
		public:
										GUI_DistanceJointParams();
		virtual							~GUI_DistanceJointParams();

		// GUI_JointParams
		virtual	udword					Init(EditBoxInterface& owner, IceWidget* parent, udword y)						override;
		virtual	void					InitFrom(Pint_Joint* joint_api, PintJointHandle handle)							override;
		virtual	void					SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)	override;
		virtual	void					Cancel()																		override;
		virtual	bool					SomethingChanged()														const	override;
		virtual	void					SetVisible(bool is_visible)														override;
		//~GUI_JointParams

				GUI_1DLimitJointParams	mLimit;
	};

	class GUI_D6JointParams : public GUI_JointParams
	{
		public:
										GUI_D6JointParams();
		virtual							~GUI_D6JointParams();

		// GUI_JointParams
		virtual	udword					Init(EditBoxInterface& owner, IceWidget* parent, udword y)						override;
		virtual	void					InitFrom(Pint_Joint* joint_api, PintJointHandle handle)							override;
		virtual	void					SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)	override;
		virtual	void					Cancel()																		override;
		virtual	bool					SomethingChanged()														const	override;
		virtual	void					SetVisible(bool is_visible)														override;
		//~GUI_JointParams

				GUI_1DLimitJointParams	mLimitX;
				GUI_1DLimitJointParams	mLimitY;
				GUI_1DLimitJointParams	mLimitZ;
	};

	class GUI_GearJointParams : public GUI_JointParams
	{
		public:
								GUI_GearJointParams();
		virtual					~GUI_GearJointParams();

		// GUI_JointParams
		virtual	udword			Init(EditBoxInterface& owner, IceWidget* parent, udword y)						override;
		virtual	void			InitFrom(Pint_Joint* joint_api, PintJointHandle handle)							override;
		virtual	void			SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)	override;
		virtual	void			Cancel()																		override;
		virtual	bool			SomethingChanged()														const	override;
		virtual	void			SetVisible(bool is_visible)														override;
		//~GUI_JointParams

				PEEL_EditBox*	mEditBox_GearRatio;
	};

	class GUI_RackJointParams : public GUI_JointParams
	{
		public:
								GUI_RackJointParams();
		virtual					~GUI_RackJointParams();

		// GUI_JointParams
		virtual	udword			Init(EditBoxInterface& owner, IceWidget* parent, udword y)						override;
		virtual	void			InitFrom(Pint_Joint* joint_api, PintJointHandle handle)							override;
		virtual	void			SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)	override;
		virtual	void			Cancel()																		override;
		virtual	bool			SomethingChanged()														const	override;
		virtual	void			SetVisible(bool is_visible)														override;
		//~GUI_JointParams

				PEEL_EditBox*	mEditBox_GearRatio;
	};

#endif
