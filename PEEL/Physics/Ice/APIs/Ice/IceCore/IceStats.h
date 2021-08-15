///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for memory stats.
 *	\file		IceStats.h
 *	\author		Pierre Terdiman
 *	\date		July, 23, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTATS_H
#define ICESTATS_H

#define DECLARE_COUNTER(export, classname)																							\
	class export classname																											\
	{																																\
		public:																														\
		/* Constructor/Destructor */																								\
											classname()					{ m__InstancesAround.Add(udword(this));					}	\
		virtual								~classname()				{ m__InstancesAround.Delete(udword(this));				}	\
																																	\
						udword				GetNbInstances()	const	{ return m__InstancesAround.GetNbEntries();				}	\
						classname**			GetAllInstances()	const	{ return (classname**)m__InstancesAround.GetEntries();	}	\
						udword				GetTotalBytes()																			\
											{																						\
												udword Ram=0;																		\
												classname** List = GetAllInstances();												\
												for(udword i=0;i<GetNbInstances();i++)	Ram+=List[i]->GetUsedRam();					\
												return Ram;																			\
											}																						\
		virtual			udword				GetUsedRam() = 0;																		\
		static			Container			m__InstancesAround;																		\
	};																																\
																																	\
	export Container*	Get##classname();																							\
	export udword		GetNbInstances##classname();																				\
	export udword		GetTotalBytes##classname();

#define IMPLEMENT_COUNTER(api, classname)																							\
	/* Static members */																											\
	Container classname::m__InstancesAround;																						\
																																	\
	Container*	api::Get##classname()				{ return &classname::m__InstancesAround;				}						\
	udword		api::GetNbInstances##classname()	{ return classname::m__InstancesAround.GetNbEntries();	}						\
	udword		api::GetTotalBytes##classname()																						\
	{																																\
		udword Ram=0;																												\
		classname** List = (classname**)classname::m__InstancesAround.GetEntries();													\
		for(udword i=0;i<GetNbInstances##classname();i++)	Ram+=List[i]->GetUsedRam();												\
		return Ram;																													\
	}

	// Ram counters
	struct ICECORE_API RamCounter
	{
				RamCounter() : RamUsed(0), HighWaterMark(0)	{}
				~RamCounter()								{}

		void	Count(udword nb_bytes)	{ RamUsed+=nb_bytes; if(RamUsed>HighWaterMark) HighWaterMark=RamUsed;	}
		void	Deduct(udword nb_bytes)	{ RamUsed-=nb_bytes;													}

		udword	RamUsed;
		udword	HighWaterMark;
	};

#endif // ICESTATS_H

