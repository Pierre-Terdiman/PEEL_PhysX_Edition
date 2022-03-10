///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains all string-related code.
 *	\file		IceString.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTRING_H
#define ICESTRING_H

	#define STRING_STATS
//	#define STRING_TRACKER

	#ifdef STRING_TRACKER
	FUNCTION ICECORE_API	const LinkedList&	GetAllStrings();
	#endif

	enum ASCIICode
	{
		ASCII_TAB				= 0x09,
		ASCII_NEXT_LINE			= 0x0a,
		ASCII_CARRIAGE_RETURN	= 0x0d,
		ASCII_SPACE				= 0x20,
	};

	// See: http://www.gotw.ca/gotw/084.htm
	// Implemented as non-members functions so that they both work with "String" and "const char*".

	FUNCTION ICECORE_API	bool		IsDecimal(const char* string, sdword* value=null);
	FUNCTION ICECORE_API	bool		IsWord(const char* string, sword* value=null);
	FUNCTION ICECORE_API	bool		IsByte(const char* string, sbyte* value=null);
	FUNCTION ICECORE_API	bool		IsFloat(const char* string, float* value=null);
	FUNCTION ICECORE_API	bool		IsBool(const char* string, bool* value=null);
	FUNCTION ICECORE_API	sdword		ToDecimal(const char* string);
	FUNCTION ICECORE_API	float		ToFloat(const char* string);
	FUNCTION ICECORE_API	udword		CountNbArgs(const char* string);

	FUNCTION ICECORE_API	sdword		Stricmp(const char*, const char*);

	FUNCTION ICECORE_API	bool		CopyToClipboard(const char*);

	// Same as "%g" but leaves one zero after decimal point. (e.g. "1.0000" => "1.0" instead of "1")
	FUNCTION ICECORE_API	void		ConvertAndRemoveTrailingZeros(char* text, float f);

	class ICECORE_API String : public Allocateable
	#ifdef STRING_TRACKER
		, public ListElem
	#endif
	{
		public:
		// Constructors/Destructor
		/*explicit*/				String();
		/*explicit*/				String(const char* string);
		/*explicit*/				String(const String& str);
									~String();

						bool		Set(const char* string);					// Assign a string
						bool		Set(const String& string);					// Assign a string
		inline_	const	char*		Get()								const	{ return mText;							}
		inline_	const	char*		GetBuffer()							const	{ return mText;							}

						void		Reset();

						udword		Length()							const;		// Get the string's length
		inline_			udword		GetLength()							const	{ return Length();						}

						sdword		Compare(const String& str2)			const;	// Compare the String with another one
						sdword		Compare(const char* str2)			const;	// Compare the String with another one
						udword		FuzzyCompare(const String& str2)	const;	// Fuzzy comparisons between strings

						String&		NewLine();

						String&		ToUpper();									// Transform into uppercase
						String&		ToLower();									// Transform into lowercase

						udword		Replace(char old_char, char new_char);		// Replace all 'old_char' characters with 'new_char' ones
		// Parameters
						bool		GetParameter(String* param, const char*& curpar)	const;	// Parameters parsing
						udword		NbParams()	const;

						String&		SetAt(ubyte cara, udword pos);
						ubyte		GetAt(udword pos);

		// String extraction methods
						String		Mid(udword first, udword count = -1)	const;
						String		Left(udword count)						const;
						String		Right(udword count)						const;

		// Searching methods (to search a specific character or a sub-String)
						sdword		Find(ubyte character, udword from = 0, sdword to = -1);
						sdword		Find(const String& stringtofind, udword from = 0, sdword to = -1);
						sdword		ReverseFind(ubyte character);

		// Numerical operand extraction methods
						bool		Extract(float& f, udword from);
						bool		Extract(udword& i, udword from);
						bool		Extract(sdword& i, udword from);

		// Plain-text methods (where starts the next line? where is the end of the current line? etc...)
						sdword		NextLine(sdword from);
						sdword		GetEndOfLine(sdword from);

		inline_			bool		IsValid()							const	{ return mText!=0;		}
		inline_			bool		IsEmpty()							const	{ return Length()==0;	}
		// Cast operators
		//! Cast operator for const char* = String
		inline_			operator const char*()							const	{ return (const char*)(mText);								}
		//! Cast operator for char* = String
		inline_			operator char*()								const	{ return (char*)(mText);									}
		//! Cast operator for float = String
		inline_			operator float()								const	{ float tmp = MAX_FLOAT; IsFloat(mText, &tmp);	return tmp;	}
		//! Cast operator for sdword = String
		inline_			operator sdword()								const	{ sdword tmp = INVALID_NUMBER; IsDecimal(mText, &tmp); return tmp;	}

		// Access operators
						const char	operator[](udword i)				const	{ return mText ? mText[i] : (const char)0;					}

		// Assignment operators
		//! Operator for String = const char*
		inline_			String&		operator=(const char* string)				{ Set(string);				return *this; }
		//! Operator for String = char*
		inline_			String&		operator=(char* string)						{ Set((const char*)string);	return *this; }
		//! Operator for String = String
		inline_			String&		operator=(const String& string)				{ Set(string);				return *this; }

		//! Concatenation operator for String += String
						String&		operator+=(const String& string);
		// Stats
#ifdef STRING_STATS
		static			udword		GetNbStrings()								{ return mNbStrings;	}
		static			udword		GetTotalBytes()								{ return mUsedRam;		}
		private:

		static			udword		mNbStrings;		//!< Number of strings around
		static			udword		mUsedRam;		//!< Amount of bytes used by strings in the system
#endif
						char*		mText;			//!< The character string held in the class
	};

	ICECORE_API	inline_ bool operator==(const String& s1, const char* s2)	{ return s1.Compare(s2) == 0; }	//!< Operator for String == char*
	ICECORE_API	inline_ bool operator!=(const String& s1, const char* s2)	{ return s1.Compare(s2) != 0; }	//!< Operator for String != char*

	ICECORE_API	inline_ bool operator==(const String& s1, const String& s2)	{ return s1.Compare(s2) == 0; }	//!< Operator for String == String
	ICECORE_API	inline_ bool operator!=(const String& s1, const String& s2)	{ return s1.Compare(s2) != 0; }	//!< Operator for String != String
	ICECORE_API	inline_ bool operator< (const String& s1, const String& s2)	{ return s1.Compare(s2) <  0; }	//!< Operator for String < String
	ICECORE_API	inline_ bool operator> (const String& s1, const String& s2)	{ return s1.Compare(s2) >  0; }	//!< Operator for String > String
	ICECORE_API	inline_ bool operator<=(const String& s1, const String& s2)	{ return s1.Compare(s2) <= 0; }	//!< Operator for String <= String
	ICECORE_API	inline_ bool operator>=(const String& s1, const String& s2)	{ return s1.Compare(s2) >= 0; }	//!< Operator for String >= String

	class ICECORE_API StringRef
	{
		public:
		inline_	StringRef(const char* text) : mText(text)	{}

		const char*	mText;
	};
	inline_ bool operator==(const StringRef& s1, const char* s2)
	{
		if(!s1.mText || !s2)	return false;
		return stricmp(s1.mText, s2)==0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Sorts a list of strings in lexicographical order. Default implementation uses qsort().
	 *	\relates	String
	 *	\fn			SortStrings(udword nb, void* list, udword stride)
	 *	\param		nb		[in] number of strings to sort
	 *	\param		list	[in] list of strings. Actually a list of structures whose first member is a String*.
	 *	\param		stride	[in] structure's extra stride, not counting sizeof(String*). Default to 0.
	 *	\return		true if success
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICECORE_API bool SortStrings(udword nb, void* list, udword stride=0);

	// A simple string container
	class ICECORE_API Strings : public PtrContainer
	{
		public:
								Strings();
								~Strings();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Adds a string to the container. A new string object is internally created. The container owns this object.
		 *	\param		string	[in] new string
		 *	\return		newly created string, or null if failure
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				String*			AddString(const char* string);

		inline_	udword			GetNbStrings()	const	{ return GetNbEntries();				}
		inline_	const String**	GetStrings()	const	{ return (const String**)GetEntries();	}
		inline_	String**		GetStrings()			{ return (String**)GetEntries();		}
	};

#endif // ICESTRING_H



#ifdef TOFIX

class	FROOTS_API		String
{
	//String "arithmetic" operators
					String					operator+(String& str);
					String					operator+(char* car);
					String					operator+(char car);
					String&					operator+=(String& str);
					String&					operator+=(char* str);
					String&					operator+=(char car);
					String&					operator=(String& str);
					String&					operator=(char* str);

	//String boolean operators
					bool					operator==(String& str);
					bool					operator==(char* str);
					bool					operator!=(String& str);
					bool					operator!=(char* str);

	protected:
					udword					SetLength(udword count, bool KeepOldContent = true);	//This method is protected because it transforms the String object in a temporary "non-valid" String!

					ubyte*					mCharBuf;									//The actual buffer of character the String represents
					sdword					mLength;									//Cache of the String's length
};

#endif