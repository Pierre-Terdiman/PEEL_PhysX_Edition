///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains basic custom RTTI code.
 *	\file		IceRTTI.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERTTI_H
#define ICERTTI_H

	/*
		We could use static members as in:

			http://www.flipcode.com/cgi-bin/msg.cgi?showThread=Tip-StaticMemRTTI&forum=totd&id=-1

		But I'd like to use it that way:
		
			SomeClass->IsKindOf("ClassName")

		So I need a real strcmp, I can't just compare static char* since one of them might actually be
		located on the stack, or at another memory location. It works to compare two classes anyway,
		provided you really have two of them.

		In practice, the strcmp is not really a problem, it's fast enough.
	*/

	#define DECLARE_RTTI(current_class, base_class)																				\
																																\
	public:																														\
			/* Gets name of the class */																						\
			virtual	const char* GetClassName()									const	{ return #current_class;	}			\
																																\
			/* RTTI */																											\
			virtual	bool		IsKindOf(const char* name, udword* depth=null)	const											\
								{																								\
									if(depth)	*depth = *depth + 1;															\
									if(strcmp(current_class::GetClassName(), name)==0) return true;								\
									else return base_class::IsKindOf(name, depth);												\
								}																								\
																																\
			/* Gets derivation level */																							\
			virtual	udword		GetLevel()										const	{ return base_class::GetLevel() + 1;	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#define DECLARE_RTTI_BASE(current_class)																					\
																																\
	public:																														\
																																\
			/* Gets name of the class */																						\
			virtual	const char* GetClassName()									const	{ return #current_class;	}			\
																																\
			/* RTTI */																											\
			virtual	bool		IsKindOf(const char* name, udword* depth=null)	const											\
								{																								\
									if(depth)	*depth = *depth + 1;															\
									return strcmp(#current_class, name)==0;														\
								}																								\
																																\
			/* Gets derivation level */																							\
			virtual	udword		GetLevel()										const	{ return 0;					}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Dynamic cast working with the macros above - with or without Cell-based classes.
	#define DYNAMIC_CAST(current_class, class_name)	(##class_name*)((!current_class || !current_class->IsKindOf(#class_name)) ? null : current_class)

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ICERTTI_H