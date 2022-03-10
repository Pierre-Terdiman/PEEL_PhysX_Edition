///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Compilation flags for Contact.
 *	\file		ContactSettings.h
 *	\author		Pierre Terdiman
 *	\date		January, 14, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CONTACTSETTINGS_H
#define CONTACTSETTINGS_H

	// PT: ahem, this comes from Tiwak but it produces bad behaviour sometimes, the capsule stops on walls instead of sliding...
	#define CCT_WALK_ON_CORNER_FIRST_SOLUTION
	//#define CCT_WALK_ON_CORNER_SECOND_SOLUTION
	#define K_EPSILON	0.01f

	#ifdef CCT_WALK_ON_CORNER_FIRST_SOLUTION
	#ifdef CCT_WALK_ON_CORNER_SECOND_SOLUTION
	#error You can not have both CCT_WALK_ON_CORNER_SECOND_SOLUTION and CCT_WALK_ON_CORNER_FIRST_SOLUTION defined
	#endif
	#endif

	// This used to be replicated everywhere in the code, causing bugs to sometimes reappear.
	// It is better to define it in a header and use the same constant everywhere. The original value (1e-05f)
	// caused troubles.
	#define CTC_PARALLEL_TOLERANCE	1e-02f

	#define CTC_TBV_CULLING

	enum TriangleCollisionFlag
	{
		// Must be the 3 first ones to be indexed by (flags & (1<<edge_index))
		TCF_ACTIVE_EDGE01	= (1<<0),	//!< Enable collision with edge 0-1
		TCF_ACTIVE_EDGE12	= (1<<1),	//!< Enable collision with edge 1-2
		TCF_ACTIVE_EDGE20	= (1<<2),	//!< Enable collision with edge 2-0
		TCF_DOUBLE_SIDED	= (1<<3),	//!< Triangle is double-sided
	//	TCF_WALKABLE		= (1<<4),
	};

#endif // CONTACTSETTINGS_H
