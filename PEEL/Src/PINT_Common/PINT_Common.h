///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_H
#define PINT_COMMON_H

	struct MemHeader
	{
		const char*	mName;
		udword		mCheckValue;
		udword		mSize;
#ifndef _WIN64
		udword		mPad0;
#endif
	};
	ICE_COMPILE_TIME_ASSERT(sizeof(MemHeader)==16);

	class MyIceAllocator : public IceCore::Allocator
	{
		public:
							MyIceAllocator(const char* name);
		virtual				~MyIceAllocator();

		virtual void*		malloc(size_t size, MemoryType type);
		virtual void*		mallocDebug(size_t size, const char* filename, udword line, const char* class_name, MemoryType type, bool from_new);
		virtual void*		realloc(void* memory, size_t size);
		virtual void*		shrink(void* memory, size_t size);
		virtual void		free(void* memory, bool from_new);

				const char*	mName;
				udword		mCurrentNbAllocs;
				udword		mUsedMemory;
				Allocator*	mPreviousAllocator;
	};

	void	Common_GetFromEditBox(float& value, const IceEditBox* edit_box, float min_value, float max_value);
	void	Common_GetFromEditBox(udword& value, const IceEditBox* edit_box);

	class Widgets;
	void	Common_CloseGUI(Widgets*& gui);
	sdword	Common_CreateDebugVizUI(	IceWidget* parent, sdword x, sdword y, CBCallback check_box_callback, udword id, udword nb_params,
										const bool* debug_viz_params, const char** debug_viz_names, IceCheckBox** check_boxes, Widgets* gui);

#endif