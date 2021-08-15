///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for symbolic lists.
 *	\file		IceSymbolicList.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESYMBOLICLIST_H
#define ICESYMBOLICLIST_H

	// Helpers
	FUNCTION ICECORE_API bool	GetBool(const String& name, bool& flag);
	FUNCTION ICECORE_API bool	GetDecimal(const String& name, udword& value);
	FUNCTION ICECORE_API bool	GetFloat(const String& name, float& value);
	FUNCTION ICECORE_API bool	GetString(const String& name, String& defined);

	class ICECORE_API SymbolicList : public Allocateable
	{
		public:
											SymbolicList(const char* name=null, udword size=0);
											~SymbolicList();
		// Create a new symbol
						bool				AddSymbol(const char* symbol, udword value);
						bool				AddSymbol(const char* symbol, float value);
		// Get the current number of symbols
						udword				GetNbSymbols()											const;
		// Get ith symbol
						const String*		GetSymbol(udword i)										const;
		// Get ith symbolic value
						bool				GetSymbolicValue(udword i, udword& value)				const;
						bool				GetSymbolicValue(udword i, float& value)				const;
		// Symbol-to-value mapping
						bool				GetSymbolicValue(const String& symbol, udword& value)	const;
						bool				GetSymbolicValue(const String& symbol, float& value)	const;
		// Value-to-symbol mapping
						bool				GetSymbolicString(udword value, String& symbol)			const;
						bool				GetSymbolicString(float value, String& symbol)			const;
		// Delete all symbols
						bool				FlushSymbols();
		// Naming
		inline_			void				SetName(const String& name)			{ mName = name;		}
		inline_			void				SetName(const char* name)			{ mName = name;		}
		inline_			const String&		GetName()					const	{ return mName;		}
		private:
						String				mName;		//!< List name == identifier
						PtrContainer		mList;		//!< List of symbolic values
	};

	// Global symbolic lists
	FUNCTION ICECORE_API bool				RegisterSymbolicList(const SymbolicList* list);
	FUNCTION ICECORE_API bool				UnregisterSymbolicList(const SymbolicList* list);
	FUNCTION ICECORE_API SymbolicList*		GetSymbolicList(const char* name);
	FUNCTION ICECORE_API SymbolicList*		CreateSymbolicList(const char* name, udword size=0);

	class ICECORE_API SymbolicStrList : public Allocateable
	{
		public:
											SymbolicStrList(const char* name=null, udword size=0);
											~SymbolicStrList();
		// Create a new symbol
						bool				AddSymbol(const char* symbol, const char* value);
		// Get the current number of symbols
						udword				GetNbSymbols()											const;
		// Get ith symbol
						const String*		GetSymbol(udword i)										const;
		// Get ith symbolic value
						bool				GetSymbolicValue(udword i, String& value)				const;
		// Symbol-to-value mapping
						bool				GetSymbolicValue(const String& symbol, String& value)	const;
		// Value-to-symbol mapping
						bool				GetSymbolicString(const String& value, String& symbol)	const;
		// Delete all symbols
						bool				FlushSymbols();
		// Naming
		inline_			void				SetName(const String& name)			{ mName = name;		}
		inline_			void				SetName(const char* name)			{ mName = name;		}
		inline_			const String&		GetName()					const	{ return mName;		}
		private:
						String				mName;		//!< List name == identifier
						PtrContainer		mList;		//!< List of symbolic values
	};

	// Global symbolic lists
	FUNCTION ICECORE_API bool				RegisterSymbolicStrList(const SymbolicStrList* list);
	FUNCTION ICECORE_API bool				UnregisterSymbolicStrList(const SymbolicStrList* list);
	FUNCTION ICECORE_API SymbolicStrList*	GetSymbolicStrList(const char* name);
	FUNCTION ICECORE_API SymbolicStrList*	CreateSymbolicStrList(const char* name, udword size=0);

	ICECORE_API bool	FindSymbolicValue(const char* list, const char* symbolic, udword& value);
	ICECORE_API bool	FindSymbolicValue(const char* list, const char* symbolic, float& value);

#endif // ICESYMBOLICLIST_H
