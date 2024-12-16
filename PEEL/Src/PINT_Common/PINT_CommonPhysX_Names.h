///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX_NAMES_H
#define PINT_COMMON_PHYSX_NAMES_H

	class Names
	{
		public:
				Strings						mNames;
				template<class T>
				void						SetNameT(T* obj, const char* name)
				{
					if(!obj)
						return;

					if(name)
					{
						// Make the string persistent
						// TODO: share strings
						IceCore::String* S = mNames.AddString(name);
						name = S->Get();	// Replace non-persistent ptr with persistent ptr
					}

					obj->setName(name);
				}
	};

#endif

