///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_Helpers.h"
#include "MyConvex.h"
#include "TestScenes.h"

float GetFloat(float value, const IceEditBox* edit_box)
{
	if(edit_box)
	{
		const float Value = edit_box->GetFloat();
		switch(edit_box->GetFilter())
		{
			case EDITBOX_FLOAT:
				ASSERT(Value>=MIN_FLOAT && Value<=MAX_FLOAT);
			break;
			case EDITBOX_FLOAT_POSITIVE:
				ASSERT(Value>=0.0f);
			break;
			default:
				ASSERT(0);
			break;
		}
		value = Value;
	}
	return value;
}

udword GetInt(udword value, const IceEditBox* edit_box)
{
	if(edit_box)
	{
		const sdword Value = edit_box->GetInt();
		ASSERT(Value>=0);
		value = udword(Value);
	}
	return value;
}

IceComboBox* CreateConvexObjectComboBox(IceWidget* parent, sdword x, sdword y, bool enabled)
{
	IceComboBox* CB = CreateComboBox<IceComboBox>(parent, 0, x, y, 150, 20, "Convex index", null, null);
	CB->Add("Convex 0");	// Cylinder
	CB->Add("Convex 1");	// "Big convex" from MAX
	CB->Add("Convex 2");	// Dodecahedron?
	CB->Add("Convex 3");	// Pyramid
	CB->Add("Convex 4");	// Truncated cone
	CB->Add("Convex 5");	// "Sphere"
	CB->Add("Convex 6");	// Cone
	CB->Add("Convex 7");	// Pentagon
	CB->Add("Convex 8");	// Thin pyramid
	CB->Add("Convex 9");	// Cylinder
	CB->Add("Convex 10");	// Debris / random
	CB->Add("Convex 11");	// Debris / random
	CB->Add("Convex 12");	// Debris / random
	CB->Add("Convex 13");	// Debris / random

	CB->Select(CONVEX_INDEX_0);
	CB->SetEnabled(enabled);
	return CB;
}

IceComboBox* CreateShapeTypeComboBox(IceWidget* parent, sdword x, sdword y, bool enabled, udword mask)
{
	IceComboBox* CB = CreateComboBox<IceComboBox>(parent, 0, x, y, 150, 20, "Shape type", null, null);

	// See enum PintShape
	CB->Add(mask & SSM_UNDEFINED ? "Undefined" : "Undefined");
	CB->Add(mask & SSM_SPHERE ? "Sphere" : "Sphere (not supported)");
	CB->Add(mask & SSM_CAPSULE ? "Capsule" : "Capsule (not supported)");
	CB->Add(mask & SSM_CYLINDER ? "Cylinder" : "Cylinder (not supported)");
	CB->Add(mask & SSM_BOX ? "Box" : "Box (not supported)");
	CB->Add(mask & SSM_CONVEX ? "Convex" : "Convex (not supported)");
	CB->Add(mask & SSM_MESH ? "Mesh" : "Mesh (not supported)");

	CB->Select(0);
	CB->SetEnabled(enabled);
	return CB;
}

ResetButton::ResetButton(PhysicsTest& test, const ButtonDesc& desc) : mTest(test), IceButton(desc)
{
}

void ResetButton::OnClick()
{
	mTest.mMustResetTest = true;
}
