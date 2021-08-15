///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains simple stacks.
 *	\file		IceStack.h
 *	\author		Pierre Terdiman
 *	\date		February, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTACK_H
#define ICESTACK_H

	class ICECORE_API LIFOStack : public Container
	{
		public:
								LIFOStack()					{}
								~LIFOStack()				{}
		// Management
		inline_	LIFOStack&		Push(udword entry)			{	Add(entry);	return *this;	}
				bool			Pop(udword& entry);
	};

	class ICECORE_API FIFOStack : public Container
	{
		public:
								FIFOStack() : mCurIndex(0)	{}
								~FIFOStack()				{}
		// Management
		inline_	FIFOStack&		Push(udword entry)			{	Add(entry);	return *this;	}
				bool			Pop(udword& entry);
		private:
				udword			mCurIndex;			//!< Current index within the container
	};

	class ICECORE_API LIFOStackPtr : public PtrContainer
	{
		public:
								LIFOStackPtr()				{}
								~LIFOStackPtr()				{}
		// Management
		inline_	LIFOStackPtr&	Push(void* entry)			{	AddPtr(entry);	return *this;	}
				bool			Pop(void*& entry);
	};

	class ICECORE_API FIFOStackPtr : public PtrContainer
	{
		public:
								FIFOStackPtr() : mCurIndex(0)	{}
								~FIFOStackPtr()					{}
		// Management
		inline_	FIFOStackPtr&	Push(void* entry)				{	AddPtr(entry);	return *this;	}
				bool			Pop(void*& entry);
		private:
				udword			mCurIndex;			//!< Current index within the container
	};

#endif // ICESTACK_H
