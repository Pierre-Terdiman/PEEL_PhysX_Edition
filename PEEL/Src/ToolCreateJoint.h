///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_CREATE_JOINT_H
#define TOOL_CREATE_JOINT_H

#include "ToolRayBased.h"
#include "GUI_ActorSelection.h"

	class ToolCreateJointBase : public ToolRayBased
	{
		public:
									ToolCreateJointBase();
		virtual						~ToolCreateJointBase();

		virtual	void				Reset				(udword pint_index);

		// GUI_RenderInterface
		virtual	void				PostRenderCallback	()	override;
		//~GUI_RenderInterface

		virtual	void				OnComboBoxChange();
		virtual	void				OnSnapButton(IceButton& button)	{}

		virtual	void				OnFromShape(IceButton& button)	{}

		protected:
				void				CreateJointComboBox(PintGUIHelper& helper, IceWidget* parent, Widgets& owner, sdword x, sdword y);
				void				CreateJointWindows(PintGUIHelper& helper, IceWidget* parent, Widgets& owner, sdword x, sdword y);

				Pint*				mPint;
				ComboBoxPtr			mJointType;
				EditPosWindowPtr	mEditPos0;
				EditPosWindowPtr	mEditPos1;
				Quat				mEditRot0;	// TODO: move to rotation editor
				Quat				mEditRot1;	// TODO: move to rotation editor
				ButtonPtr			mSnap0;
				ButtonPtr			mSnap1;
				ButtonPtr			mFromShape0;
				ButtonPtr			mFromShape1;
				IceWindow*			mGearJointWindow;
				IceWindow*			mRackJointWindow;

				enum DelayedAction
				{
					NONE,
					CREATE_JOINT,
					SNAP_POS_0,
					SNAP_POS_1,
				};
				DelayedAction		mPendingAction;
	};

	class ToolCreateJoint : public ToolCreateJointBase, public ActorSelectionCallback, public ShapeSelectionCallback
	{
		public:
								ToolCreateJoint();
		virtual					~ToolCreateJoint();

		virtual	void			CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void			Reset				(udword pint_index);
		virtual	void			Deselect			()						{ Reset(INVALID_ID);	}

		virtual	void			OnObjectReleased	(Pint& pint, PintActorHandle removed_object);

		virtual	void			RightDownCallback	(Pint& pint, udword pint_index);
//		virtual	void			SetMouseData		(const MouseInfo& mouse);
		virtual	void			MouseMoveCallback	(const MouseInfo& mouse);

		// GUI_RenderInterface
		virtual	void			PreRenderCallback	()													override;
		virtual	void			RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		//~GUI_RenderInterface

		// ActorSelectionCallback
		virtual	void			OnSelectedActor		(Pint* pint, PintActorHandle h, void* user_data)	override;
		//~ActorSelectionCallback

		// ShapeSelectionCallback
		virtual	void			OnSelectedShape		(PintShapeHandle h, void* user_data)	override;
		//~ShapeSelectionCallback

		virtual	void			OnComboBoxChange();
		virtual	void			OnSnapButton(IceButton& button);
		virtual	void			OnFromShape(IceButton& button);

				void			OnActorSelected();
		private:
		template<class T>
				void			SetupPivot(T& desc, Pint& pint, const Quat& q);
		template<class T>
				void			SetupPivot(T& desc, Pint& pint);

				void			CreateJoint(Pint& pint);

				void			SelectActor(Pint& pint, const PintRaycastHit& hit);
				bool			ValidateTouchedActor(Pint& pint, IceButton* button, PintActorHandle& dstActor, PintShapeHandle& dstShape, const PintRaycastHit& hit, PintActorHandle other_actor, bool& validate);
				void			ResetButtonNames(bool reset0, bool reset1);

				ButtonPtr		mButton0;
				ButtonPtr		mButton1;
				ButtonPtr		mActiveButton;
				EditBoxPtr		mEditBoxName;
				PintActorHandle	mActor0;
				PintShapeHandle	mShape0;
				PintActorHandle	mActor1;
				PintShapeHandle	mShape1;
	};

	struct LinkActor
	{
		PintActorHandle	mActor;
		//PintShapeHandle	mShape;
	};

	class LinkActors
	{
		public:
									LinkActors();
									~LinkActors();

		inline_	void				Empty()						{ mLinkActors.Empty();	}

		inline_	LinkActor*			ReserveSlot()				{ mDirty = true;	return ICE_RESERVE(LinkActor, mLinkActors);			}

		inline_	udword				GetNbLinkActors()	const	{ return mLinkActors.GetNbEntries()/(sizeof(LinkActor)/sizeof(udword));	}
		inline_	const LinkActor*	GetLinkActors()		const	{ return reinterpret_cast<const LinkActor*>(mLinkActors.GetEntries());	}

		inline_	void				SortActors(Pint& pint)
									{
										if(mDirty)
											Sort(pint);
									}
		private:
				Container			mLinkActors;
				bool				mDirty;

				void				Sort(Pint& pint);
	};

	class ToolCreateChain : public ToolCreateJointBase
	{
		public:
								ToolCreateChain();
		virtual					~ToolCreateChain();

		virtual	void			CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void			Reset				(udword pint_index);
		virtual	void			Deselect			()						{ Reset(INVALID_ID);	}

		virtual	void			RightDownCallback	(Pint& pint, udword pint_index);

		// GUI_RenderInterface
		virtual	void			RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		//~GUI_RenderInterface

		virtual	void			OnSnapButton(IceButton& button);

				ButtonPtr		mButton0;
				ButtonPtr		mActiveButton;	// Used as a bool in this case (whether mButton0 is pushed or not)
				LinkActors		mLinkActors;
		private:
				bool			ValidateActor(PintActorHandle actor)	const;
				void			CreateJoints(Pint& pint);
	};

#endif
