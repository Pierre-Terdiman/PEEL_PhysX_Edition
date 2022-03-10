///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_ERROR_H
#define PINT_COMMON_PHYSX3_ERROR_H

	class PEEL_PhysX3_ErrorCallback : public PxErrorCallback
	{
	public:
				PEEL_PhysX3_ErrorCallback()		{}
		virtual	~PEEL_PhysX3_ErrorCallback()	{}

		virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);
	};

#endif

