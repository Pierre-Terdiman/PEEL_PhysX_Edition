///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains custom lists.
 *	\file		IceCustomList.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECUSTOMLIST_H
#define ICECUSTOMLIST_H

	class ICECORE_API CustomList : public Cell
	{
						DECLARE_ICE_CLASS(CustomList, Cell)
		public:
						bool			Init(const FieldDescriptor* desc, udword nb_values, bool symbolic=false);

						bool			IsCompatible(const FieldDescriptor* fld)	const	{ return (fld && fld->Type==mBaseType && fld->UserParam==mType);	}

		inline_			udword			GetNbValues()		const			{ return mNbValues;					}
		inline_			void*			GetValues()			const			{ return mValues;					}
//		inline_			String*			GetSymbolicValues()	const			{ return mSymbolic;					}
		inline_			const char**	GetSymbolicValues()	const			{ return mSymbolic;					}

						bool			BuildSymbolicValue(udword index, String& value);

		inline_			FieldType		GetBaseType()		const			{ return mBaseType;					}
		inline_			udword			GetCustomType()		const			{ return mType;						}

						bool			Replace(Cell* cell, const FieldDescriptor* field, udword index);
		protected:
		// List values
						udword			mNbValues;		//!< Number of values in the list
						udword			mValueSize;		//!< Size in bytes of a single value
						void*			mValues;		//!< List of values
//						String*			mSymbolic;		//!< Possible extra list of symbolic values
						const char**	mSymbolic;		//!< Possible extra list of symbolic values
		// List descriptor
						FieldType		mBaseType;		//!< Base type for values
						udword			mType;			//!< User subtype for values
		public:
		// List identifier
						udword			mUserID;		//!< Extra possible ID, left to the user - not used at all by the CustomList
	};

//	ICECORE_API CustomList* CreateDefaultCustomList(eFieldType type);

#endif // ICECUSTOMLIST_H
