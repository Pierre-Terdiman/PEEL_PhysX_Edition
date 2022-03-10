///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestSelector.h"
#include "TestScenes.h"
#include "GLFontRenderer.h"

//static const float Alpha[] = {0.1f, 0.4f, 0.75f, 1.0f, 0.75f, 0.4f, 0.1f};
static const float Alpha[] = {0.2f, 0.3f, 0.6f, 1.0f, 0.6f, 0.3f, 0.2f};

static TestCategory GetCurrentCategory(int start_index)
{
	const udword NbTests = GetNbTests();
	const int NbToGo = ICE_ARRAYSIZE(Alpha);
	for(int i=0;i<NbToGo;i++)
	{
		const float a = Alpha[i];
		const bool Selected = a==1.0f;

		if(Selected)
		{
			const int RealIndex = start_index % NbTests;
			PhysicsTest* Test = GetTest(RealIndex);
			return Test->GetCategory();
		}
		start_index++;
	}
	return CATEGORY_UNDEFINED;
}

static int NextCategory(int start_index)
{
	const TestCategory CurrentCategory = GetCurrentCategory(start_index);
	int CurrentTest = start_index;

	const udword NbTests = GetNbTests();
	TestCategory NewCategory = CurrentCategory;
	do
	{
		// Key down
		CurrentTest++;
		if(CurrentTest==NbTests)
			CurrentTest=0;

		NewCategory = GetCurrentCategory(CurrentTest);
	}while(NewCategory==CurrentCategory && CurrentTest!=start_index);
	return CurrentTest;
}

static int PreviousCategory(int start_index)
{
	const TestCategory CurrentCategory = GetCurrentCategory(start_index);
	int CurrentTest = start_index;

	const udword NbTests = GetNbTests();
	TestCategory NewCategory = CurrentCategory;
	do
	{
		// Key up
		if(CurrentTest)
			CurrentTest--;
		else
			CurrentTest = NbTests-1;

		NewCategory = GetCurrentCategory(CurrentTest);
	}while(NewCategory==CurrentCategory && CurrentTest!=start_index);
	return CurrentTest;
}

int TestSelectionKeyboardCallback(int key, int current_test)
{
	const udword NbTests = GetNbTests();
	switch(key)
	{
		case GLUTX_KEY_DOWN:
		{
			current_test++;
			if(current_test==NbTests)
				current_test=0;
			break;
		}
		case GLUTX_KEY_UP:
		{
			if(current_test)
				current_test--;
			else
				current_test = NbTests-1;
			break;
		}
		case GLUTX_KEY_RIGHT:
		{
			current_test = NextCategory(current_test);
			break;
		}
		case GLUTX_KEY_LEFT:
		{
			current_test = PreviousCategory(current_test);
			break;
		}
	}
	return current_test;
}

PhysicsTest* RenderTestSelector(const GLFontRenderer& texter, float x, float text_scale, int start_index, float& y_last)
{
	int NbTests = GetNbTests();
	const int NbToGo = ICE_ARRAYSIZE(Alpha);
	float y = float(NbToGo) * text_scale;

	const float XOffset = 0.01f;
//	DrawRectangle(XOffset, 1.0f-XOffset, y + text_scale*2.0f, y - text_scale*9.0f, Point(1.0f, 0.5f, 0.2f), 0.5f);

	PhysicsTest* CandidateTest = null;

//	const Point SelectedColor(38.0f/255.0f, 109.0f/255.0f, 103.0f/255.0f);
	const Point SelectedColor(66.0f/255.0f, 142.0f/255.0f, 136.0f/255.0f);

	for(int i=0;i<NbToGo;i++)
	{
		const float a = Alpha[i];
		const bool Selected = a==1.0f;
		if(!Selected)
			//texter.setColor(1.0f, 0.5f, 0.2f, a);
			texter.setColor(SelectedColor.x, SelectedColor.y, SelectedColor.z, a);
		else
			texter.setColor(1.0f, 1.0f, 1.0f, a);

		const int RealIndex = start_index % NbTests;

		PhysicsTest* Test = GetTest(RealIndex);
		if(Test)
		{
			static const char* CategoryText[] = 
			{
				"(undefined)",
				"(API)",
				"(cooking)",
				"(behavior)",
				"(contact generation)",
				"(joints-basics)",
				"(joints)",
				"(joints-portals)",
				"(articulations)",
				"(MC articulations)",
				"(RC articulations)",
				"(performance)",
				"(fracture tests)",
				"(kinematics)",
				"(CCD)",
				"(vehicles)",
				"(character controllers)",
				"(raycast)",
				"(sweep)",
				"(overlap)",
				"(midphase)",
				"(mesh-vs-mesh midphase)",
				"(static scene)",
				"(work in progress)",
				"(disabled)",
				"(internal)",
			};

			const TestCategory TC = Test->GetCategory();
			if(Test->IsPrivate())
			{
				texter.print(x+XOffset, y, text_scale, _F("%d: %s - %s (PRIVATE)", RealIndex, CategoryText[TC], Test->GetName()));
			}
			else
			{
				texter.print(x+XOffset, y, text_scale, _F("%d: %s - %s", RealIndex, CategoryText[TC], Test->GetName()));
			}
		}

		if(Selected)
			CandidateTest = Test;

		y -= text_scale;
		start_index++;
	}
	y_last = y;
	return CandidateTest;
}
