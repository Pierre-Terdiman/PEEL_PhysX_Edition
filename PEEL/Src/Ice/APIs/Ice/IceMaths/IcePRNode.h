///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for PR nodes.
 *	\file		IcePRNode.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPRNODE_H
#define ICEPRNODE_H

/*
	class ICEMATHS_API PRNode : public HNode
	{
		public:
		//! Constructor
		inline_							PRNode()								{ mPRPtr = &mPR;	}
		//! Destructor
		virtual							~PRNode()								{}

		// WARNING: access to the PR(S) is given through these methods, but you should NOT modify it directly unless you REALLY
		// know what you're doing. Lazy evaluation is heavily used with things like PR(S), and it should be updated thanks to the
		// appropriate (virtual) accessors.

		// The compulsory mPR member is present in all inherited classes, that's the static PR.
		inline_			const	PR*		GetStaticPR()					const	{ return &mPR;									}
		inline_			const	Point*	GetStaticPos()					const	{ return &mPR.mPos;								}
		inline_			const	Quat*	GetStaticRot()					const	{ return &mPR.mRot;								}

		// Otherwise the anonymous "GetPR" can be either static or dynamic, courtesy of the app. The app is responsible for
		// making the internal pointer points to the dynamic PR, stored in higher levels.
		inline_			const	PR*		GetPR()							const	{ return mPRPtr;								}
		inline_			const	Point*	GetPos()						const	{ return mPRPtr->mPos;							}
		inline_			const	Quat*	GetRot()						const	{ return mPRPtr->mRot;							}

		virtual	inline_	void	ResetPRS()								{ mPR.Reset();									}
		//
		virtual inline_	void	SetPR(const PR& pr)						{ mPR		= pr;								}
		virtual inline_	void	SetPos(const Point& pos)				{ mPR.mPos	= pos;								}
		virtual inline_	void	SetRot(const Quat& rot)					{ mPR.mRot	= rot;								}
		//
		virtual inline_	void	AddPR(const PR& pr)						{ mPR		+= pr;								}
		virtual inline_	void	AddPos(const Point& pos)				{ mPR.mPos	+= pos;								}
		virtual inline_	void	AddRot(const Quat& rot)					{ mPR.mRot	*= rot;								}

		protected:
								PR		mPR;			//!< PR data ### try to keep it private

		inline_					void	SetDynamicPR(const PR* ptr)				{ mPRPtr = ptr;									}
		private:
		const					PR*		mPRPtr;			//!< Pointer to static or dynamic PR
	};
*/


	enum PRSFlag
	{
		PRS_RESERVED	= (1<<0),	//!< Reserved flag
		PRS_LOCAL		= (1<<1),	//!< PRS is local/relative (else global/absolute)
/*		PRS_LOCKED_PX	= (1<<2),	//!< x position can't be modified
		PRS_LOCKED_PY	= (1<<3),	//!< y position can't be modified
		PRS_LOCKED_PZ	= (1<<4),	//!< z position can't be modified
		PRS_LOCKED_POS	= (1<<5),	//!< Position can't be modified
		PRS_LOCKED_ROT	= (1<<6),	//!< Rotation can't be modified*/
		PRS_LOCKED		= (1<<7),	//!< PRS can't be modified
	};

	enum PRComponent
	{
		PRS_POSITION	= (1<<0),
		PRS_ROTATION	= (1<<1),
		PRS_SCALE		= (1<<2)
	};

	class ICEMATHS_API PRNode : public HNode
//	class ICEMATHS_API PRNode
	{
		public:
								PRNode();
		virtual					~PRNode();

								DECLARE_ALL_FLAGS(PRSFlag, mPRSFlags)

		// WARNING: access to the PR(S) is given through these methods, but you should NOT modify it directly unless you REALLY
		// know what you're doing. Lazy evaluation is heavily used with things like PR(S), and it should be updated thanks to the
		// appropriate (virtual) accessors.

		// The compulsory PR member is present in all inherited classes, that's the static PR.
		inline_	const	PR*		GetStaticPR()			const	{ return &mStaticPR;		}
		inline_	const	Point*	GetStaticPos()			const	{ return &mStaticPR.mPos;	}
		inline_	const	Quat*	GetStaticRot()			const	{ return &mStaticPR.mRot;	}
/*
		// Otherwise the anonymous "GetPR" can be either static or dynamic, courtesy of the app. The app is responsible for
		// making the internal pointer points to the dynamic PR, stored in higher levels.
		inline_	const	PR*		GetPR()					const	{ return mPRPtr;			}
		inline_	const	Point*	GetPos()				const	{ return &mPRPtr->mPos;		}
		inline_	const	Quat*	GetRot()				const	{ return &mPRPtr->mRot;		}
*/
		virtual	inline_	void	InvalidatePR(udword flags)		{	/* To override */		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the PR. This is the same as a "set" operation, without checkings & callbacks.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void	InitStaticPR(const PR& pr)		{ mStaticPR = pr;			}
		// Static PR control
						void	ResetStaticPR();
		//
						BOOL	SetStaticPR(const PR& pr);
						BOOL	SetStaticPos(const Point& pos);
						BOOL	SetStaticRot(const Quat& rot);
		//
						void	AddStaticPR(const PR& pr);
						void	AddStaticPos(const Point& pos);
						void	AddStaticRot(const Quat& rot);
/*
		// Dynamic PR control
		inline_			void	ResetPRS()						{ if(mPRSFlags&PRS_LOCKED) return;	mPRPtr->Reset();		InvalidatePR();	}
		//
		inline_			void	SetPR(const PR& pr)				{ if(mPRSFlags&PRS_LOCKED) return;	*mPRPtr			= pr;	InvalidatePR();	}
		inline_			void	SetPos(const Point& pos)		{ if(mPRSFlags&PRS_LOCKED) return;	mPRPtr->mPos	= pos;	InvalidatePos();}
		inline_			void	SetRot(const Quat& rot)			{ if(mPRSFlags&PRS_LOCKED) return;	mPRPtr->mRot	= rot;	InvalidateRot();}
		//
		inline_			void	AddPR(const PR& pr)				{ if(mPRSFlags&PRS_LOCKED) return;	*mPRPtr			+= pr;	InvalidatePR();	}
		inline_			void	AddPos(const Point& pos)		{ if(mPRSFlags&PRS_LOCKED) return;	mPRPtr->mPos	+= pos;	InvalidatePos();}
		inline_			void	AddRot(const Quat& rot)			{ if(mPRSFlags&PRS_LOCKED) return;	mPRPtr->mRot	*= rot;	InvalidateRot();}
*/
		// Helpers
						bool	SetNonMovable();
						bool	SetMovable();
/*
		inline_			bool	IsPRStatic()			const	{ return mPRPtr==&mStaticPR;	}
		inline_			bool	IsPRDynamic()			const	{ return mPRPtr!=&mStaticPR;	}
*/
		protected:
						PR		mStaticPR;						//!< Static PR data ### try to keep it private
/*
		inline_			void	SetDynamicPR(PR* ptr)
						{
							if(mPRSFlags&PRS_LOCKED) return;	// "Locked" means "static by all means"!
							if(!ptr) return;					// Basic checkings
							// ### CMOV
							if(mPRPtr!=ptr)	mPRPtr = ptr;		// mPRPtr now points the app-controlled dynamic PR
						}
*/
//		inline_			void	SetStaticPR()					{ mPRPtr = &mStaticPR;	/* Resets the dynamic pointer */	}
/*		private:
						PR*		mPRPtr;							//!< Pointer to static or dynamic PR
*/
	};

	class ICEMATHS_API DynamicPRNode : public PRNode
	{
		public:
								DynamicPRNode();
		virtual					~DynamicPRNode();

		// The anonymous "GetPR" can be either static or dynamic, courtesy of the app.
		inline_	const	PR*		GetPR()					const	{ return mCurrentPR;			}
		inline_	const	Point*	GetPos()				const	{ return &mCurrentPR->mPos;		}
		inline_	const	Quat*	GetRot()				const	{ return &mCurrentPR->mRot;		}

		// Gets previous PR
		inline_	const	PR*		GetPreviousPR()			const	{ return mPreviousPR;			}
		inline_	const	Point*	GetPreviousPos()		const	{ return &mPreviousPR->mPos;	}
		inline_	const	Quat*	GetPreviousRot()		const	{ return &mPreviousPR->mRot;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the PR. This is the same as a "set" operation, without checkings & callbacks.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void	InitPR(const PR& pr)
								{
									*mCurrentPR = pr;
									if(mPreviousPR!=mCurrentPR)	*mPreviousPR = pr;
								}
		// Dynamic PR control
						void	ResetPR();
		//
						BOOL	SetPR(const PR& pr);
						BOOL	SetPos(const Point& pos);
						BOOL	SetRot(const Quat& rot);
		//
						void	AddPR(const PR& pr);
						void	AddPos(const Point& pos);
						void	AddRot(const Quat& rot);

		// The app is reponsible for the flip! Else multiple controllers could each call AddPR (for example),
		// and the "previous" PR would have no sense.
		inline_			void	FlipPR()						{ PR* Tmp = mCurrentPR;	mCurrentPR = mPreviousPR; mPreviousPR = Tmp;			}

		inline_			bool	IsPRStatic()			const	{ return mCurrentPR==&mStaticPR;	}
		inline_			bool	IsPRDynamic()			const	{ return mCurrentPR!=&mStaticPR;	}

		inline_			void	MakeStaticPR()
								{
									mCurrentPR = mPreviousPR = &mStaticPR;	// Resets the dynamic pointers
								}
		inline_			void	MakeDynamicPR()
								{
									if(IsSet(PRS_LOCKED)) return;	// "Locked" means "static by all means"!
									if(mCurrentPR==&mStaticPR)	mCurrentPR = &mDynamicPR[0];
									if(mPreviousPR==&mStaticPR)	mPreviousPR = &mDynamicPR[1];
								}
		private:
						PR		mDynamicPR[2];					//!< Dynamic PR data
						PR*		mCurrentPR;						//!< Pointer to static or dynamic PR
						PR*		mPreviousPR;					//!< Pointer to static or dynamic PR
	};

/*	### TEMP. REMOVED, redesigning stuff

	class ICEMATHS_API PRSNode : public HNode
	{
		public:
		inline_					PRSNode()	{}
		virtual							~PRSNode()	{}

		// Access to the PR(S) is given through these methods, but you shouldn't modify it directly unless you really know
		// what you're doing. Lazy evaluation is heavily used with things like PR(S), and it should be updated thanks to the
		// appropriate (virtual) methods.
		inline_	const PRS*		GetPRS()	const						{ return &mPRS;									}
		inline_	const PR*		GetPR()		const						{ return (const PR*)&mPRS;						}
		inline_	const Point*	GetPos()	const						{ return &mPRS.mPos;							}
		inline_	const Quat*		GetRot()	const						{ return &mPRS.mRot;							}
		inline_	const Point*	GetScale()	const						{ return &mPRS.mScale;							}

		virtual	inline_	void	ResetPRS()								{ mPRS.Reset();									}
		//
		virtual inline_	void	SetPRS(const PRS& prs)					{ mPRS			= prs;							}
		virtual inline_	void	SetPR(const PR& pr)						{ mPRS.mPos		= pr.mPos; mPRS.mRot = pr.mRot;	}
		virtual inline_	void	SetPos(const Point& pos)				{ mPRS.mPos		= pos;							}
		virtual inline_	void	SetRot(const Quat& rot)					{ mPRS.mRot		= rot;							}
		virtual inline_	void	SetScale(const Point& scale)			{ mPRS.mScale	= scale;						}
		//
		virtual inline_	void	AddPos(const Point& pos)				{ mPRS.mPos		+= pos;							}

		protected:
						PRS				mPRS;
	};
*/

	class ICEMATHS_API LazyPRNode : public DynamicPRNode
	{
		public:
		//! Constructor
											LazyPRNode() :	mValidAddress(1),
															mWorldMultiplier(null),
															mMatrixHistory(null)	{ mPRSTimeStamp = 0;/*hum*/				}

		//! Destructor
		virtual								~LazyPRNode()							{ ReleaseMatrixHistory();				}

		override(PRNode)	inline_	void	InvalidatePR(udword flags)				{ mValidAddress|=1;	mPRSTimeStamp++;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the cached world matrix. This is a lazy-evaluated accessor:
		 *	- if cached matrix is invalid, it gets recomputed
		 *	- else cached pointer is returned
		 *
		 *	If resulting matrix is the identity, the global identity matrix address is returned instead of the local matrix.
		 *	That way all immovable nodes share the same matrix pointer, so you can batch them together and save SetTransform calls.
		 *
		 *	\return		address of a matrix corresponding to current PR data
		 *	\warning	do NOT store this pointer, as it may point to invalid data after the PR has been updated
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			const Matrix4x4*	GetWorldMatrix()			const
											{
												// Check matrix pointer. The rationale behind this is that any valid pointer should be at least
												// word aligned in sake of efficiency. So we use the last bit as a validity flag, which saves some ram.
												if(mValidAddress&1)	const_cast<LazyPRNode* const>(this)->UpdateMatrix();	// "mutable method"

												// Return now valid cached address
												return (const Matrix4x4*)mValidAddress;
											}

		//! HANDLE WITH CARE
		inline_			const Matrix4x4*	GetCachedWorldMatrix()		const		{ return &mCachedMatrix;	}

		inline_			udword				GetPRSTimeStamp()			const		{ return mPRSTimeStamp;		}

		// Matrix history support

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the matrix history.
		 *	\param		nb		[in] size of history buffer (number of saved matrices)
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				InitMatrixHistory(udword nb);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Releases the matrix history.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			void				ReleaseMatrixHistory();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the matrix history. The first item is the most recent one.
		 *	\return		the matrix history
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	HistoryBuffer<Matrix4x4>*	GetMatrixHistory()			const		{ return mMatrixHistory;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Updates the matrix history.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_			void				UpdateMatrixHistory()
											{
												if(mMatrixHistory)
												{
													const Matrix4x4* World = GetWorldMatrix();
													if(!World)	World = &Get4x4IdentityMatrix();
													mMatrixHistory->Record(*World);
												}
											}

		// World multiplier support

		inline_			void				SetWorldMultiplier(const Matrix4x4* multiplier)		{ mWorldMultiplier = multiplier;	InvalidatePR(PRS_POSITION|PRS_ROTATION);		}
		inline_			const Matrix4x4*	GetWorldMultiplier()	const						{ return mWorldMultiplier;								}

		// Master matrix support

//						void				SetMasterMatrix(const Matrix4x4* master_matrix);
		private:
			// Cached world matrix
						size_t				mValidAddress;		//!< Matrix address (local or global)
		const			Matrix4x4*			mWorldMultiplier;	//!< Possible world multiplier
						Matrix4x4			mCachedMatrix;		//!< Lazy-evaluated cached matrix ### could keep those in a matrix cache
						udword				mPRSTimeStamp;		//!< Current timestamp
			// Matrix history
				HistoryBuffer<Matrix4x4>*	mMatrixHistory;		//!< Possible matrix history
		// Lazy evaluation
						void				UpdateMatrix();
	};

#endif // ICEPRNODE_H
