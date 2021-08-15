///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains import-export related code.
 *	\file		IceImportExport.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEIMPORTEXPORT_H
#define ICEIMPORTEXPORT_H

	//! TO BE DOCUMENTED
	#define _DECLARE_ICE_ABSTRACT_CLASS(current_class, base_class)										\
																										\
	public:																								\
																										\
	DECLARE_RTTI(current_class, base_class)																\
																										\
	virtual	udword	GetSize()	const	{ return SIZEOFOBJECT;	}										\
																										\
	virtual bool GetFields(FieldDescriptors& edit, udword flags)	const								\
	{																									\
		base_class::GetFields(edit, flags);																\
		const FieldDescriptor* Fields = current_class::GetDescriptor();									\
		for(udword i=0;i<current_class::GetDescriptorSize();i++)										\
		{																								\
			if(Fields[i].Flags&flags)	edit.AddDescriptor(&Fields[i]);									\
		}																								\
		return true;																					\
	}																									\
																										\
	virtual bool GetFields(FieldDescriptors& edit, FieldType type)	const								\
	{																									\
		base_class::GetFields(edit, type);																\
		const FieldDescriptor* Fields = current_class::GetDescriptor();									\
		for(udword i=0;i<current_class::GetDescriptorSize();i++)										\
		{																								\
			if(Fields[i].Type==type)	edit.AddDescriptor(&Fields[i]);									\
		}																								\
		return true;																					\
	}																									\
																										\
	virtual bool GetFields(FieldDescriptors& edit, FieldType type, udword user)	const					\
	{																									\
		base_class::GetFields(edit, type, user);														\
		const FieldDescriptor* Fields = current_class::GetDescriptor();									\
		for(udword i=0;i<current_class::GetDescriptorSize();i++)										\
		{																								\
			if(Fields[i].Type==type && Fields[i].UserParam==user)	edit.AddDescriptor(&Fields[i]);		\
		}																								\
		return true;																					\
	}																									\
																										\
	virtual bool GetFields(FieldDescriptors& edit)	const												\
	{																									\
		base_class::GetFields(edit);																	\
		const FieldDescriptor* Fields = current_class::GetDescriptor();									\
		for(udword i=0;i<current_class::GetDescriptorSize();i++)	edit.AddDescriptor(&Fields[i]);		\
		return true;																					\
	}																									\
																										\
	virtual const FieldDescriptor* GetFieldDescriptor(FieldType type, udword user)	const				\
	{																									\
		const FieldDescriptor* FD = base_class::GetFieldDescriptor(type, user);							\
		if(FD)	return FD;																				\
																										\
		const FieldDescriptor* Fields = current_class::GetDescriptor();									\
		for(udword i=0;i<current_class::GetDescriptorSize();i++)										\
		{																								\
			if(Fields[i].Type==type && Fields[i].UserParam==user)	return &Fields[i];					\
		}																								\
		return null;																					\
	}																									\
																										\
	virtual const FieldDescriptor* GetFieldDescriptor(const char* name)	const							\
	{																									\
		if(!name)	return null;																		\
		const FieldDescriptor* FD = base_class::GetFieldDescriptor(name);								\
		if(FD)	return FD;																				\
																										\
		return FindDescriptor(name);																	\
	}																									\
																										\
	virtual udword Export(ExportContext& ec)															\
	{																									\
		if(!base_class::Export(ec))	return 0;															\
		else return ec.Save(#current_class, this, mClassDescriptor, current_class::GetDescriptorSize());\
	}																									\
																										\
	virtual udword Import(ImportContext& ic)															\
	{																									\
		if(!base_class::Import(ic))	return 0;															\
		else return ic.Load(#current_class, this, mClassDescriptor, current_class::GetDescriptorSize());\
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class ICECORE_API ImportContext
	{
		public:
					ImportContext();
					~ImportContext();
		udword		Load(const char* class_name, Cell* current_class, const FieldDescriptor* fields, udword nbfields);
	};

	class ICECORE_API ExportContext
	{
		public:
					ExportContext();
					~ExportContext();
		udword		Save(const char* class_name, Cell* current_class, const FieldDescriptor* fields, udword nbfields);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! TO BE DOCUMENTED
	#define DECLARE_METHODS	public:	virtual bool ExecCommand(const char* command, void* user_data=null);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#define BEGIN_METHODS(current_class, base_class)								\
		bool current_class::ExecCommand(const char* command, void* user_data)	{	\
		if(base_class::ExecCommand(command, user_data)) return true;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#define DECLARE_METHOD_0(method)			if(!command) EnumCommand(#method, user_data); else if(strcmp(command, #method)==0)		{ method();			return true; }
	#define DECLARE_METHOD_1(method, p0)		if(!command) EnumCommand(#method, user_data); else if(strcmp(command, #method)==0)		{ method(p0);		return true; }
	#define DECLARE_METHOD_2(method, p0, p1)	if(!command) EnumCommand(#method, user_data); else if(strcmp(command, #method)==0)		{ method(p0, p1);	return true; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#define END_METHODS		return false;	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Avoiding new/delete problems using DLLs:
	// http://www.flipcode.com/cgi-bin/msg.cgi?showThread=Tip-MemoryDLL&forum=totd&id=-1

	//! Creates an instance of a class
	typedef Cell*	(*ClassCreator)		();
	//! Deletes an instance of a class
	typedef void	(*ClassDestructor)	(Cell* instance);

	#define _DECLARE_PROTECTED_CLASS(current_class)										\
		protected:			current_class();											\
		virtual				~current_class();											\
		static	Cell*		CreateInstance();											\
		static	void		DeleteInstance(Cell* instance);								\
		public:																			\
		virtual	const char*	GetClassString()	const	{ return #current_class;	}	\
		static	bool		RegisterClass();

	#define IMPLEMENT_CLASS(current_class)																		\
		Cell* current_class::CreateInstance()				{ return ICE_NEW(current_class);	}				\
		void current_class::DeleteInstance(Cell* instance)	{ if(instance) delete (current_class*)instance;	}	\
		bool current_class::RegisterClass()					{ return GetKernel()->RegisterClass(#current_class, current_class::CreateInstance, current_class::DeleteInstance);}

	#define REGISTER_CLASS(current_class)	current_class::RegisterClass();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#define DECLARE_ICE_CLASS(current_class, base_class)	\
		_DECLARE_PROTECTED_CLASS(current_class)				\
		_DECLARE_ICE_ABSTRACT_CLASS(current_class, base_class)

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Various ways to create classes
	#define CreateICEObject(class_name)			(class_name*)GetKernel()->CreateInstance(#class_name)
	#define LAZY_CREATE(class_name, address)	if(!address)	address = CreateICEObject(class_name)

#endif // ICEIMPORTEXPORT_H
