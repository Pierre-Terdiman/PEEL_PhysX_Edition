///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PINT_Common.h"
#include "../PintGUIHelper.h"

void Common_GetFromEditBox(float& value, const IceEditBox* edit_box, float min_value, float max_value)
{
	if(edit_box)
	{
//		float Value;
//		bool status = edit_box->GetTextAsFloat(Value);
//		ASSERT(status);
		const float Value = edit_box->GetFloat();
		ASSERT(Value>=min_value && Value<=max_value);
		value = Value;
	}
}

void Common_GetFromEditBox(udword& value, const IceEditBox* edit_box)
{
	if(edit_box)
	{
//		sdword Value;
//		bool status = edit_box->GetTextAsInt(Value);
//		ASSERT(status);
		const sdword Value = edit_box->GetInt();
		ASSERT(Value>=0);
		value = udword(Value);
	}
}

void Common_CloseGUI(Widgets*& gui)
{
	DELETESINGLE(gui);
}

sdword Common_CreateDebugVizUI(	IceWidget* parent, sdword x, sdword y, CBCallback check_box_callback, udword id, udword nb_params,
								const bool* debug_viz_params, const char** debug_viz_names, IceCheckBox** check_boxes, Widgets* gui)
{
	ASSERT(gui);
	ASSERT(check_boxes);
	ASSERT(debug_viz_names);
	ASSERT(debug_viz_params);

	const sdword YStepCB = 16;
	const sdword YSaved = y;

	CheckBoxDesc CBD;
	CBD.mParent		= parent;
	CBD.mX			= x+10;
	CBD.mWidth		= 170;
	CBD.mHeight		= 20;
	CBD.mCallback	= check_box_callback;

	for(udword i=0;i<nb_params;i++)
	{
		CBD.mID			= id++;
		CBD.mY			= y+20;
		CBD.mLabel		= debug_viz_names[i];
		CBD.mChecked	= debug_viz_params[i];
		check_boxes[i]	= ICE_NEW(IceCheckBox)(CBD);
		gui->Register(check_boxes[i]);
		check_boxes[i]->SetVisible(true);
		if(i==0)
			check_boxes[i]->SetEnabled(true);
		else
			check_boxes[i]->SetEnabled(debug_viz_params[0]);
		y += YStepCB;
	}

	if(0)
	{
		GroupBoxDesc GBD;
		GBD.mParent		= parent;
		GBD.mX			= x;
		GBD.mY			= YSaved;
		GBD.mWidth		= 200;
		GBD.mHeight		= nb_params*20;
		GBD.mLabel		= "Debug visualization";
		IceGroupBox* GB = ICE_NEW(IceGroupBox)(GBD);
		gui->Register(GB);
		GB->SetVisible(true);
	}
	else
	{
		EditBoxDesc EBD;
		EBD.mParent		= parent;
		EBD.mX			= x;
		EBD.mY			= YSaved;
		EBD.mWidth		= 200;
		EBD.mHeight		= 20*2 + nb_params*YStepCB;
		EBD.mLabel		= "======= Debug visualization ========";
//		EBD.mLabel		= ""
		EBD.mFilter		= EDITBOX_TEXT;
		EBD.mType		= EDITBOX_READ_ONLY;
		IceEditBox* EB = ICE_NEW(IceEditBox)(EBD);
		EB->SetVisible(true);
		gui->Register(EB);
/*
		LabelDesc LD;
		LD.mParent		= EB;
//		LD.mX			= x+14;
		LD.mX			= 14;
//		LD.mY			= YSaved-5;
		LD.mY			= -5;
		LD.mWidth		= 95;
		LD.mHeight		= 14;
		LD.mLabel		= "Debug visualization";
		IceLabel* Label = ICE_NEW(IceLabel)(LD);
		Label->SetVisible(true);
		gui->Add(udword(Label));*/
	}
	return y;
}




MyIceAllocator::MyIceAllocator(const char* name) : mName(name), mCurrentNbAllocs(0), mUsedMemory(0), mPreviousAllocator(null)
{
	mPreviousAllocator = GetAllocator();
}

MyIceAllocator::~MyIceAllocator()
{
	if(mCurrentNbAllocs)
		printf(_F("%s: %d memory leaks detected... (%d bytes)\n", mName, mCurrentNbAllocs, mUsedMemory));
}

void* MyIceAllocator::malloc(size_t size, MemoryType type)
{
	mCurrentNbAllocs++;
//	return ::malloc(size);

	MemHeader* Header = (MemHeader*)_aligned_malloc(size+sizeof(MemHeader), 16);
	Header->mName		= mName;
	Header->mCheckValue	= 0x12345678;
	Header->mSize		= udword(size);
	mUsedMemory += udword(size);
	return Header+1;
}

void* MyIceAllocator::mallocDebug(size_t size, const char* filename, udword line, const char* class_name, MemoryType type, bool from_new)
{
	mCurrentNbAllocs++;
//	return ::malloc(size);
//	return _aligned_malloc(size, 16);

	MemHeader* Header = (MemHeader*)_aligned_malloc(size+sizeof(MemHeader), 16);
	Header->mName		= mName;
	Header->mCheckValue	= 0x12345678;
	Header->mSize		= udword(size);
	mUsedMemory += udword(size);
	return Header+1;
}

void* MyIceAllocator::realloc(void* memory, size_t size)
{
	return null;
//	return ::realloc(memory, size);
}

void* MyIceAllocator::shrink(void* memory, size_t size)
{
	return memory;
}

void MyIceAllocator::free(void* memory, bool from_new)
{
	mCurrentNbAllocs--;
//	::free(memory);
//	_aligned_free(memory);

	MemHeader* Header = (MemHeader*)memory;
	Header--;
	if(Header->mCheckValue != 0x12345678)
	{
		mPreviousAllocator->free(memory, from_new);
		return;
	}
//	ASSERT(Header->mCheckValue == 0x12345678);
	ASSERT(Header->mName==mName);
	mUsedMemory -= Header->mSize;
	_aligned_free(Header);
}
