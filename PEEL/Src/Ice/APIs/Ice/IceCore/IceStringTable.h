///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a string table.
 *	\file		IceStringTable.h
 *	\author		Pierre Terdiman
 *	\date		September, 26, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTRINGTABLE_H
#define ICESTRINGTABLE_H

	class ICECORE_API SortedStrings : public Allocateable
	{
		public:
											SortedStrings()	: mSorted(FALSE){}
											~SortedStrings()				{}

		inline_			udword				GetNbStrings()			const	{ return mContainer.GetNbEntries();										}
		inline_			const String**		GetStrings()					{ if(!mSorted)	Sort();	return (const String**)mContainer.GetEntries();	}
		inline_			const String*		GetString(udword i)				{ if(!mSorted)	Sort();	return (const String*)mContainer.GetEntry(i);	}

						SortedStrings&		AddString(const String& str)	{ mContainer.AddPtr(&str);	mSorted = FALSE; return *this;				}
						udword				Find(const char* str);
		private:
						PtrContainer		mContainer;
						BOOL				mSorted;
		// Lazy-sorting
						void				Sort();
	};

	class ICECORE_API StringDescriptor : public ListElem, public Allocateable
	{
		public:
			inline_			StringDescriptor() : mStr(null)	{}
			inline_			~StringDescriptor()				{}

		const String*		mStr;
	};

	class ICECORE_API StringTable : public Allocateable
	{
		public:
						StringTable();
						~StringTable();

		bool			Init(udword nb=10);
		bool			Free();

		bool					Add(const String& str);
		const StringDescriptor*	Find(const String& str);

		private:
						udword			mNbLists;
						udword			mMask;
						LinkedList**	mLists;
	};

#endif // ICESTRINGTABLE_H
