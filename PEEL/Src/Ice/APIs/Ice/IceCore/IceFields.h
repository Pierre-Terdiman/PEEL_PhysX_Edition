///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains supported native fields.
 *	\file		IceFields.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFIELDS_H
#define ICEFIELDS_H

	//! Native supported types, understood by the kernel.
	enum FieldType
	{
		FIELD_BOOL,								//!< A bool (not a BOOL!)
		FIELD_BYTE,								//!< An sbyte or ubyte
		FIELD_WORD,								//!< An sword or uword
		FIELD_DWORD,							//!< An sdword or udword
		FIELD_FLOAT,							//!< A float
		FIELD_DOUBLE,							//!< A double
		FIELD_CELL_PTR,							//!< A Cell pointer (or any pointer to a derived class)
		FIELD_STRING,							//!< A String
		FIELD_POINT,							//!< A vector = 3 floats
		FIELD_QUAT,								//!< A quaternion = 4 floats
		FIELD_PR,								//!< A PR structure (vector + quaternion)
		FIELD_PRS,								//!< A PRS structure (vector + quaternion + vector)
		FIELD_DYNABLOC,							//!< A dynamic data block (struct DynamicBlock)
		FIELD_KID,								//!< A kernel ID
		FIELD_PTR,								//!< A generic pointer (not a Cell*)

		FIELD_FORCE_DWORD		= 0x7fffffff
	};

	// Flags
	enum FieldFlag
	{
		F_HIDDEN				= 0,			//!< Default: hidden field
		F_VISIBLE				= (1<<0),		//!< Visible field
		F_EDITABLE				= (1<<1),		//!< Editable field
		F_DUPLICABLE			= (1<<2),		//!< Duplicable field
		F_GLOBAL_MSG_ON_MODIF	= (1<<3),		//!< Send a global message on field modification
		F_OWNER_MSGS_ON_MODIF	= (1<<4),		//!< Send a message to owners on field modification
		F_REF_MSGS_ON_MODIF		= (1<<5),		//!< Send a message to references on field modification
		F_SELF_MSGS_ON_MODIF	= (1<<6),		//!< Send a message to the modified cell on field modification

		F_ALL					= 0xffffffff	//!< All fields
	};

#endif // ICEFIELDS_H
