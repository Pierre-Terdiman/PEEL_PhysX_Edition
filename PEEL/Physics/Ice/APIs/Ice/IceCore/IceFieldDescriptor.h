///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains field descriptors.
 *	\file		IceFieldDescriptor.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFIELDDESCRIPTOR_H
#define ICEFIELDDESCRIPTOR_H

	// Forward declarations
	class Cell;
	struct FieldDescriptor;
	class FieldController;
	class ParameterBlock;

	//! Defines a generic field
	#define _FIELD(type, name, fieldtype, count, flags, accessor, callback, user)	{ fieldtype, #name, OFFSET_OF(type, name), count, flags, accessor, callback, user }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Defines a single base field
	#define DEFINE_FIELD(type, name, fieldtype, flags, accessor, callback)						_FIELD(type, name, fieldtype,     1, flags, accessor, callback, 0)
	//! Defines a static array of base fields
	#define DEFINE_FIELDS(type, name, fieldtype, count, flags, accessor, callback)				_FIELD(type, name, fieldtype, count, flags, accessor, callback, 0)
	//! Defines a single custom field
	#define DEFINE_USER_FIELD(type, name, fieldtype, flags, accessor, callback, user)			_FIELD(type, name, fieldtype,     1, flags, accessor, callback, user)
	//! Defines a static array of custom fields
	#define DEFINE_USER_FIELDS(type, name, fieldtype, count, flags, accessor, callback, user)	_FIELD(type, name, fieldtype, count, flags, accessor, callback, user)
	//! Defines a dynamic array of base fields
	#define DEFINE_ARRAY(type, name, fieldtype, flags, accessor, callback)						_FIELD(type, name, fieldtype,     0, flags, accessor, callback, 0)
	//! Defines a dynamic array of custom fields
	#define DEFINE_USER_ARRAY(type, name, fieldtype, flags, accessor, callback, user)			_FIELD(type, name, fieldtype,     0, flags, accessor, callback, user)

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Starts field declarations
	#define BEGIN_FIELDS(current_class)		FieldDescriptor current_class::mClassDescriptor[] = {	\
											{ FIELD_FORCE_DWORD, #current_class, 0, 0, 0, null, null, 0 },
	//! Ends field declarations
	#define END_FIELDS(current_class)		};	udword current_class::GetDescriptorSize()	const { return ICE_ARRAYSIZE(mClassDescriptor);	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Declares a static class descriptor for the class. (a.k.a. meta-data)
	//! You can use this on all classes, not only Cell-based ones. This macro must be used in the header files,
	//! then fields must be declared in the cpp file using BEGIN_FIELDS / DEFINE_FIELD / END_FIELDS macros.
	#define DECLARE_FIELDS																		\
																								\
	static	FieldDescriptor		mClassDescriptor[];												\
																								\
	/* Gets the number of defined fields. There's always at least one for the class itself	*/	\
	/* This is ONLY for current class, not base classes. So NO VIRTUAL there.				*/	\
	/*virtual*/ udword GetDescriptorSize()	const;												\
	/* Gets field descriptors */																\
	const FieldDescriptor* GetDescriptor()	const	{ return mClassDescriptor;	}				\
	/* Finds a field given its name */															\
	const FieldDescriptor* FindDescriptor(const char* name)	const								\
	{																							\
		if(!name)	return null;																\
		udword Size = GetDescriptorSize();														\
		for(udword i=0;i<Size;i++)																\
		{																						\
			const FieldDescriptor* FD = &mClassDescriptor[i];									\
			if(strcmp(FD->Name, name)==0)	return FD;											\
		}																						\
		return null;																			\
	}																							\
	/* Setups from a parameter block */															\
	bool SetFrom(const ParameterBlock& pb)														\
	{																							\
		udword Size = GetDescriptorSize();														\
		for(udword i=0;i<Size;i++)																\
		{																						\
			if(mClassDescriptor[i].SetFrom(this, pb))	return true;							\
		}																						\
		return false;																			\
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Field modification callback. Called by the system each time a declared field must be modified.
	 *	\param		owner		[in] object to modify
	 *	\param		field		[in] descriptor of modified field
	 *	\param		data		[in] address of the new value
	 *	\return		true to override the system modification - which means you handled it yourself
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef bool	(*ModificationNotif)	(Cell* owner, const FieldDescriptor* field, const void* data);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Field control callback. Called by the system each time a declared field must be updated by a controller.
	 *	\param		owner		[in] object to modify
	 *	\param		ctrl		[in] field controller
	 *	\param		time		[in] current time-related info
	 *	\return		true if success
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef bool	(*SetAccessor)			(Cell* owner, FieldController* ctrl, const TimeInfo& time);

	//! A field descriptor
	struct ICECORE_API FieldDescriptor
	{
		// Compulsory values
						FieldType			Type;			//!< Field type (bool, byte, quaternion, etc)
						const char*			Name;			//!< Field name (appears exactly as in the source file)
						udword				Offset;			//!< Offset from the start of the class (ie from "this", field is located at "this"+Offset)
						uword				Count;			//!< Number of items of type Type (0 for dynamic sizes)
						uword				Flags;			//!< Field parameters
						ModificationNotif	Callback;		//!< Modification callback
						SetAccessor			Accessor;		//!< Set accessor
		// User-defined values
						udword				UserParam;		//!< A user-defined parameter / ID to customize some fields.
		// Generic methods
						udword				FieldSize()										const;
		inline_			void*				Address(void* class_ptr)						const	{ return (void*)(size_t(class_ptr) + Offset);	}
		inline_			void				DefaultCopy(void* ptr, const void* data)		const	{ CopyMemory(Address(ptr), data, FieldSize());	}
						bool				SetFrom(void* ptr, const ParameterBlock& pb)	const;
		// Cell methods
						void				Replace(Cell* cell, const void* data)			const;
		inline_			void				Copy(Cell* dest, Cell* src)						const	{ Replace(dest, Address(src));					}
						uword				GetCount()										const;
	};

	class ICECORE_API FieldDescriptors : public Allocateable
	{
		public:
											FieldDescriptors()			{}
											~FieldDescriptors()			{}

		inline_			udword				GetNbDescriptors()								const	{ return mContainer.GetNbEntries();						}
		inline_			FieldDescriptor**	GetDescriptors()								const	{ return (FieldDescriptor**)mContainer.GetEntries();	}
		inline_			FieldDescriptor*	GetDescriptor(udword i)							const	{ return (FieldDescriptor*)mContainer.GetEntry(i);		}

						void				AddDescriptor(const FieldDescriptor* descriptor)		{ mContainer.AddUnique(descriptor);						}

											PREVENT_COPY(FieldDescriptors)
		private:
						PtrContainer		mContainer;
	};

#endif // ICEFIELDDESCRIPTOR_H
