///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_Skeleton.h"
#include "..\PINT_Common\PINT_Common.h"

///////////////////////////////////////////////////////////////////////////////

SkeletonPint::SkeletonPint()
{
}

SkeletonPint::~SkeletonPint()
{
}

void SkeletonPint::GetCaps(PintCaps& caps) const
{
}

void SkeletonPint::Init(const PINT_WORLD_CREATE& desc)
{
}

void SkeletonPint::Close()
{
}

udword SkeletonPint::Update(float dt)
{
	return 0;
}

Point SkeletonPint::GetMainColor()
{
	return Point(0.0f, 0.0f, 0.0f);
}

void SkeletonPint::Render(PintRender& renderer, PintRenderPass render_pass)
{
}

void SkeletonPint::RenderDebugData(PintRender& renderer)
{
}

PintActorHandle SkeletonPint::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	return null;
}

bool SkeletonPint::ReleaseObject(PintActorHandle handle)
{
	return false;
}

PintJointHandle SkeletonPint::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	return null;
}



static SkeletonPint* gSkeleton = null;

void Skeleton_Init(const PINT_WORLD_CREATE& desc)
{
	ASSERT(!gSkeleton);
	gSkeleton = ICE_NEW(SkeletonPint);
	gSkeleton->Init(desc);
}

void Skeleton_Close()
{
	if(gSkeleton)
	{
		gSkeleton->Close();
		delete gSkeleton;
		gSkeleton = null;
	}
}

SkeletonPint* GetSkeleton()
{
	return gSkeleton;
}

///////////////////////////////////////////////////////////////////////////////

static Widgets*	gSkeletonGUI = null;

enum SkeletonGUIElement
{
	SKELETON_GUI_MAIN,
	//
};

IceWindow* Skeleton_InitGUI(IceWidget* parent, PintGUIHelper& helper)
{
	IceWindow* Main = helper.CreateMainWindow(gSkeletonGUI, parent, SKELETON_GUI_MAIN, "Skeleton options");

	return Main;
}

void Skeleton_CloseGUI()
{
	Common_CloseGUI(gSkeletonGUI);
}

///////////////////////////////////////////////////////////////////////////////

class SkeletonPlugIn : public PintPlugin
{
	public:
	virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper)	{ return Skeleton_InitGUI(parent, helper);	}
	virtual	void		CloseGUI()											{ Skeleton_CloseGUI();						}
	virtual	void		Init(const PINT_WORLD_CREATE& desc)					{ Skeleton_Init(desc);						}
	virtual	void		Close()												{ Skeleton_Close();							}
	virtual	Pint*		GetPint()											{ return GetSkeleton();						}
};
static SkeletonPlugIn gPlugIn;

PintPlugin*	GetPintPlugin()
{
	return &gPlugIn;
}
