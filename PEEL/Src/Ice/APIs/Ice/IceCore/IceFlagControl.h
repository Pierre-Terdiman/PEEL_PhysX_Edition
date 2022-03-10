///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for flags control.
 *	\file		IceFlagControl.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *	\warning	the macro contains virtual methods so don't use it blindly
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFLAGCONTROL_H
#define ICEFLAGCONTROL_H

	//! Read-only flags
	#define DECLARE_FLAGS(flag_type, member, read_only_mask)			\
	public:	/* Public interface */										\
																		\
	/* Get a copy of all flags */										\
	inline_	udword	Get##flag_type()		const						\
					{													\
						return member;									\
					}													\
																		\
	/* Checks a particular flag is set or not */						\
	inline_	BOOL	IsSet(flag_type flag)	const						\
					{													\
						return member & flag;							\
					}													\
																		\
	/* Enables a particular flag */										\
	virtual	bool	Enable(flag_type flag)								\
					{													\
						ASSERT(CountBits2(flag)==1 && "Enable: use a single enum value at a time!");\
						/* Nothing to do if already enabled */			\
						if(IsSet(flag)) return true;					\
						/* Don't modify read-only flags */				\
						if(read_only_mask & flag) return false;			\
						/* Set it on */									\
						member |= flag;									\
						/* Tell the user the flag has been modified */	\
						return OnModifiedFlag(flag);					\
					}													\
																		\
	/* Disables a particular flag */									\
	virtual	bool	Disable(flag_type flag)								\
					{													\
						ASSERT(CountBits2(flag)==1 && "Disable: use a single enum value at a time!");\
						/* Nothing to do if already disabled */			\
						if(!IsSet(flag)) return true;					\
						/* Don't modify read-only flags */				\
						if(read_only_mask & flag) return false;			\
						/* Set it off */								\
						member &= ~flag;								\
						/* Tell the user the flag has been modified */	\
						return OnModifiedFlag(flag);					\
					}													\
																		\
	/* Flips a particular flag */										\
	virtual	bool	Flip(flag_type flag)								\
					{													\
						ASSERT(CountBits2(flag)==1 && "Flip: use a single enum value at a time!");\
						/* Don't modify read-only flags */				\
						if(read_only_mask & flag) return false;			\
						/* Flip it */									\
						member ^= flag;									\
						/* Tell the user the flag has been modified */	\
						return OnModifiedFlag(flag);					\
					}													\
																		\
	/* Set on or off according to a boolean */							\
	virtual	bool	Set(flag_type flag, bool b)							\
					{													\
						if(b)	return Enable(flag);					\
						else	return Disable(flag);					\
					}													\
																		\
	/* User modification callback - to implement each time */			\
	virtual	bool	OnModifiedFlag(flag_type flag);						\
																		\
	protected: /* Protected interface, to setup read-only flags */		\
																		\
	/* Enables a particular flag */										\
			bool	ForceEnable(flag_type flag)							\
					{													\
						ASSERT(CountBits2(flag)==1 && "ForceEnable: use a single enum value at a time!");\
						/* Nothing to do if already enabled */			\
						if(IsSet(flag)) return true;					\
						/* Set it on */									\
						member |= flag;									\
						/* Tell the user the flag has been modified */	\
						return OnModifiedFlag(flag);					\
					}													\
																		\
	/* Disables a particular flag */									\
			bool	ForceDisable(flag_type flag)						\
					{													\
						ASSERT(CountBits2(flag)==1 && "ForceDisable: use a single enum value at a time!");\
						/* Nothing to do if already disabled */			\
						if(!IsSet(flag)) return true;					\
						/* Set it off */								\
						member &= ~flag;								\
						/* Tell the user the flag has been modified */	\
						return OnModifiedFlag(flag);					\
					}													\
																		\
	/* Flips a particular flag */										\
			bool	ForceFlip(flag_type flag)							\
					{													\
						ASSERT(CountBits2(flag)==1 && "ForceFlip: use a single enum value at a time!");\
						/* Flip it */									\
						member ^= flag;									\
						/* Tell the user the flag has been modified */	\
						return OnModifiedFlag(flag);					\
					}													\
																		\
	/* Set on or off according to a boolean */							\
			bool	ForceSet(flag_type flag, bool b)					\
					{													\
						if(b)	return ForceEnable(flag);				\
						else	return ForceDisable(flag);				\
					}													\
																		\
	private:	udword	member;	public:

	//! Read-write flags
	#define DECLARE_ALL_FLAGS(flag_type, member)	DECLARE_FLAGS(flag_type, member, 0)

#endif // ICEFLAGCONTROL_H
