///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple container class.
 *	\file		IceContainer.h
 *	\author		Pierre Terdiman
 *	\date		February, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECONTAINER_H
#define ICECONTAINER_H

//	#define CONTAINER_STATS			// #### doesn't work with micro-threads!

	class LinkedList;

/*	enum FindMode
	{
		FIND_CLAMP,
		FIND_WRAP,

		FIND_FORCE_DWORD = 0x7fffffff
	};*/

	class ICECORE_API Container0 : public Allocateable
#ifdef CONTAINER_STATS
		, public ListElem
#endif
	{
		public:
								Container0();
								Container0(const Container0& object);
								Container0(udword size, float growth_factor);
								~Container0();
		// Management
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the container so that it uses an external memory buffer. The container doesn't own the memory, resizing is disabled.
		 *	\param		max_entries		[in] max number of entries in the container
		 *	\param		entries			[in] external memory buffer
		 *	\param		allow_resize	[in] false to prevent resizing the array (forced fixed length), true to enable it. In that last case the
		 *									initial memory is NOT released, it's up to the caller. That way the initial memory can be on the stack.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void			InitSharedBuffers(udword max_entries, udword* entries, bool allow_resize=false);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	A O(1) method to add a value in the container. The container is automatically resized if needed.
		 *	The method is inline, not the resize. The call overhead happens on resizes only, which is not a problem since the resizing operation
		 *	costs a lot more than the call overhead...
		 *
		 *	\param		entry		[in] a udword to store in the container
		 *	\see		Add(float entry)
		 *	\see		Empty()
		 *	\see		Contains(udword entry)
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	Container0&		Add(udword entry)
								{
									// Resize if needed
									if(mCurNbEntries==mMaxNbEntries)	Resize();

									// Add new entry
									mEntries[mCurNbEntries++]	= entry;
									return *this;
								}

		inline_	Container0&		Add(const udword* entries, udword nb)
								{
									if(entries && nb)
									{
										// Resize if needed
										if(mCurNbEntries+nb>mMaxNbEntries)	Resize(nb);

										// Add new entries
										CopyMemory(&mEntries[mCurNbEntries], entries, nb*sizeof(udword));
										mCurNbEntries += nb;
									}
									return *this;
								}

		inline_	Container0&		Add(const Container0& container)
								{
									return Add(container.GetEntries(), container.GetNbEntries());
								}

		inline_	udword*			Reserve(udword nb)
								{
									// Resize if needed
									if(mCurNbEntries+nb>mMaxNbEntries)	Resize(nb);

									// We expect the user to fill reserved memory with 'nb' udwords
									udword* Reserved = &mEntries[mCurNbEntries];

									// Meanwhile, we do as if it had been filled
									mCurNbEntries += nb;

									// This is mainly used to avoid the copy when possible
									return Reserved;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	A O(1) method to add a value in the container. The container is automatically resized if needed.
		 *	The method is inline, not the resize. The call overhead happens on resizes only, which is not a problem since the resizing operation
		 *	costs a lot more than the call overhead...
		 *
		 *	\param		entry		[in] a float to store in the container
		 *	\see		Add(udword entry)
		 *	\see		Empty()
		 *	\see		Contains(udword entry)
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	Container0&		Add(float entry)
								{
									// Resize if needed
									if(mCurNbEntries==mMaxNbEntries)	Resize();

									// Add new entry
									mEntries[mCurNbEntries++]	= IR(entry);
									return *this;
								}

		inline_	Container0&		Add(const float* entries, udword nb)
								{
									// Resize if needed
									if(mCurNbEntries+nb>mMaxNbEntries)	Resize(nb);

									// Add new entries
									CopyMemory(&mEntries[mCurNbEntries], entries, nb*sizeof(float));
									mCurNbEntries+=nb;
									return *this;
								}

		//! Add unique [slow]
		inline_	Container0&		AddUnique(udword entry)
								{
									if(!Contains(entry))
										Add(entry);
									return *this;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Clears the container. All stored values are deleted, and it frees used ram.
		 *	\see		Reset()
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				Container0&		Empty();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Resets the container. Stored values are discarded but the buffer is kept so that further calls don't need resizing again.
		 *	That's a kind of temporal coherence.
		 *	\see		Empty()
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	void			Reset()
								{
									// Avoid the write if possible
									// ### CMOV
									if(mCurNbEntries)	mCurNbEntries = 0;
								}

		// HANDLE WITH CARE - I hope you know what you're doing
		inline_	void			ForceSize(udword size)
								{
									mCurNbEntries = size;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Sets the initial size of the container. If it already contains something, it's discarded.
		 *	\param		nb		[in] Number of entries
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool			SetSize(udword nb);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Refits the container and get rid of unused bytes.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool			Refit();

				bool			Shrink();

		// Checks whether the container already contains a given value.
				bool			Contains(udword entry, udword* location=null) const;
		// Deletes an entry - doesn't preserve insertion order.
				bool			Delete(udword entry);
		// Deletes an entry - does preserve insertion order.
				bool			DeleteKeepingOrder(udword entry);
		//! Deletes the very last entry.
		inline_	void			DeleteLastEntry()						{ if(mCurNbEntries)	mCurNbEntries--;			}
		//! Deletes the entry whose index is given
		inline_	void			DeleteIndex(udword index)				{ mEntries[index] = mEntries[--mCurNbEntries];	}
		//! Deletes the entry whose index is given - does preserve insertion order.
				bool			DeleteIndexKeepingOrder(udword index);
		// Helpers
//				Container0&		FindNext(udword& entry, FindMode find_mode=FIND_CLAMP);
//				Container0&		FindPrev(udword& entry, FindMode find_mode=FIND_CLAMP);
		// Data access.
		inline_	udword			GetNbEntries()					const	{ return mCurNbEntries;					}	//!< Returns the current number of entries.
		inline_	udword			GetMaxNbEntries()				const	{ return mMaxNbEntries;					}	//!< Returns max number of entries before resizing.
		inline_	udword			GetEntry(udword i)				const	{ return mEntries[i];					}	//!< Returns ith entry.
		inline_	udword*			GetEntries()					const	{ return mEntries;						}	//!< Returns the list of entries.

		inline_	udword			GetFirst()						const	{ return mEntries[0];					}
		inline_	udword			GetLast()						const	{ return mEntries[mCurNbEntries-1];		}

		// Growth control
//				float			GetGrowthFactor()				const;												//!< Returns the growth factor
//				void			SetGrowthFactor(float growth);														//!< Sets the growth factor

		inline_	float			GetGrowthFactor()				const	{ return mGrowthFactor;					}	//!< Returns the growth factor
		inline_	void			SetGrowthFactor(float growth)														//!< Sets the growth factor
								{
									// Negative growths are reserved for internal usages
									if(growth<0.0f)	growth = 0.0f;
									mGrowthFactor = growth;
								}

		inline_	bool			IsFull()						const	{ return mCurNbEntries==mMaxNbEntries;	}	//!< Checks the container is full
		inline_	BOOL			IsNotEmpty()					const	{ return mCurNbEntries;					}	//!< Checks the container is empty

		//! Read-access as an array
		inline_	udword			operator[](udword i)			const	{ ASSERT(i<mCurNbEntries); return mEntries[i];	}
		//! Write-access as an array
		inline_	udword&			operator[](udword i)					{ ASSERT(i<mCurNbEntries); return mEntries[i];	}

		// Stats
				udword			GetUsedRam()					const;

		//! Operator for "Container A = Container B"
				void			operator = (const Container0& object);

#ifdef CONTAINER_STATS
		static	udword			GetNbContainers()						{ return mNbContainers;	}
		static	udword			GetTotalBytes()							{ return mUsedRam;		}
		static	LinkedList&		GetContainers()							{ return mContainers;	}
		private:

		static	udword			mNbContainers;		//!< Number of containers around
		static	udword			mUsedRam;			//!< Amount of bytes used by containers in the system
		static	LinkedList		mContainers;
#endif
		private:
		// Resizing
				bool			Resize(udword needed=1);
				void			ShiftFrom(udword index);
		// Data
				udword			mMaxNbEntries;		//!< Maximum possible number of entries
				udword			mCurNbEntries;		//!< Current number of entries
				udword*			mEntries;			//!< List of entries
				float			mGrowthFactor;		//!< Resize: new number of entries = old number * mGrowthFactor
	};

#ifdef ICE_USE_STL_VECTOR
	#define LocalContainer(x, n)	Container	x;
#else
	#define LocalContainer(x, n)	Container0	x;	x.InitSharedBuffers(n, (udword*)StackAlloc(sizeof(udword)*n), true);
#endif

	///////////

#ifdef ICE_USE_STL_VECTOR

	template <class T>
	inline void swap_erase(std::vector<T> &v, typename std::vector<T>::iterator it)
	{
		std::swap(*it, v.back());
		v.pop_back();
	}

	template <class T>
	inline void swap_erase(std::vector<T> &v, int i)
	{
		swap_erase(v, v.begin() + i);
	}

	class ICECORE_API Container2 : public std::vector<udword>,  public Allocateable
	{
		public:
									Container2()											{}
									Container2(const Container2& object)
									{
										for(udword i=0;i<object.GetNbEntries();i++)
											Add(object.GetEntry(i));
									}
	//								Container2(udword size, float growth_factor);
									~Container2()											{}

	//		inline_	udword			GetNbEntries()		const	{ return mCurNbEntries;		}
			inline_	udword			GetNbEntries()		const	{ return size();			}

	//		inline_	udword			GetMaxNbEntries()	const	{ return mMaxNbEntries;		}
			inline_	udword			GetMaxNbEntries()	const	{ return capacity();		}

	//		inline_	udword			GetEntry(udword i)	const	{ return mEntries[i];		}
			inline_	udword			GetEntry(udword i)	const	{ return _Myfirst[i];		}

	//		inline_	udword*			GetEntries()		const	{ return mEntries;			}
			inline_	udword*			GetEntries()		const	{ return _Myfirst;			}

			inline_	udword			GetFirst()			const	{ return front();			}
			inline_	udword			GetLast()			const	{ return back();			}

			inline_	bool			IsFull()			const	{ return _Mylast==_Myend;	}
			inline_	BOOL			IsNotEmpty()		const	{ return size();			}

			inline_	Container2&		Add(udword entry)
									{
										push_back(entry);
										return *this;
									}

			inline_	Container2&		Add(const udword* entries, udword nb)
									{
										if(entries && nb)
										{
											while(nb--)
												push_back(*entries++);
										}
										return *this;
									}

			inline_	Container2&		Add(const Container2& container)
									{
										return Add(container.GetEntries(), container.GetNbEntries());
									}

			inline_	Container2&		Add(float entry)
									{
										push_back(IR(entry));
										return *this;
									}

			inline_	Container2&		Add(const float* entries, udword nb)
									{
										if(entries && nb)
										{
											while(nb--)
											{
												float data = *entries++;
												push_back(IR(data));
											}
										}
										return *this;
									}

			inline_	udword*			Reserve(udword nb)
									{
										reserve(nb+size());
										udword* Current = _Mylast;
										_Mylast += nb;
										return Current;
	/*									// Resize if needed
										if(mCurNbEntries+nb>mMaxNbEntries)	Resize(nb);

										// We expect the user to fill reserved memory with 'nb' udwords
										udword* Reserved = &mEntries[mCurNbEntries];

										// Meanwhile, we do as if it had been filled
										mCurNbEntries += nb;

										// This is mainly used to avoid the copy when possible
										return Reserved;*/
									}

			// Checks whether the container already contains a given value.
					bool			Contains(udword entry, udword* location=null) const
									{
										udword NbEntries = GetNbEntries();
										const udword* Entries = GetEntries();
										// Look for the entry
										for(udword i=0;i<NbEntries;i++)
										{
											if(Entries[i]==entry)
											{
												if(location)	*location = i;
												return true;
											}
										}
										return false;
									}

			//! Add unique [slow]
			inline_	Container2&		AddUnique(udword entry)
									{
										if(!Contains(entry))	Add(entry);
										return *this;
									}

					Container2&		Empty()
									{
										_Tidy();
										return *this;
									}

			inline_	void			Reset()
									{
										clear();
									}

			// HANDLE WITH CARE - I hope you know what you're doing
			inline_	void			ForceSize(udword size)
									{
	//									mCurNbEntries = size;
										_Mylast = _Myfirst + size;
									}

					bool			SetSize(udword nb)
									{
										Empty();
										resize(nb);
										Reset();
										return true;
									}

	/*				bool			Refit()
									{
	//									resize(size());
										return true;
									}*/

	//				bool			Shrink();

			//! Deletes the very last entry.
			inline_	void			DeleteLastEntry()
									{
									//	if(mCurNbEntries)	mCurNbEntries--;
										pop_back();
									}

			//! Deletes the entry whose index is given
			inline_	void			DeleteIndex(udword index)
									{
										swap_erase(*this, index);
	//									mEntries[index] = mEntries[--mCurNbEntries];
									}

			//! Deletes the entry whose index is given - does preserve insertion order.
					bool			DeleteIndexKeepingOrder(udword index)
									{
										std::vector<udword>::iterator it = begin() + index;
										erase(it);
										return true;
									}

			// Deletes an entry - doesn't preserve insertion order.
					bool			Delete(udword entry)
									{
										udword NbEntries = GetNbEntries();
										const udword* Entries = GetEntries();

										// Look for the entry
										for(udword i=0;i<NbEntries;i++)
										{
											if(Entries[i]==entry)
											{
												// Entry has been found at index i. The strategy is to copy the last current entry at index i, and decrement the current number of entries.
												DeleteIndex(i);
												return true;
											}
										}
										return false;
									}

			// Deletes an entry - does preserve insertion order.
					bool			DeleteKeepingOrder(udword entry)
									{
										udword NbEntries = GetNbEntries();
										const udword* Entries = GetEntries();

										// Look for the entry
										for(udword i=0;i<NbEntries;i++)
										{
											if(Entries[i]==entry)
											{
												// Entry has been found at index i.
	//											ShiftFrom(i);
												DeleteIndexKeepingOrder(i);
												return true;
											}
										}
										return false;
									}

			//! Operator for "Container A = Container B"
					void			operator = (const Container2& object)
									{
										Empty();
										for(udword i=0;i<object.GetNbEntries();i++)
											Add(object.GetEntry(i));
									}

			// Stats
					udword			GetUsedRam()					const
									{
										return sizeof(Container2) + GetMaxNbEntries() * sizeof(udword);
									}
	/*
			// Helpers
					Container&		FindNext(udword& entry, FindMode find_mode=FIND_CLAMP);
					Container&		FindPrev(udword& entry, FindMode find_mode=FIND_CLAMP);

			// Growth control
					float			GetGrowthFactor()				const;												//!< Returns the growth factor
					void			SetGrowthFactor(float growth);														//!< Sets the growth factor
	*/
	};
#endif

#ifdef ICE_USE_STL_VECTOR
	typedef	Container2	Container;
#else
	typedef	Container0	Container;
#endif

	class ICECORE_API PtrContainer : public Allocateable
	{
		public:
								PtrContainer();
								~PtrContainer();

		inline_	PtrContainer&	AddPtr(void* ptr)
								{
									// Resize if needed
									if(mCurNbEntries==mMaxNbEntries)
										Resize();

									// Add new entry
									mEntries[mCurNbEntries++] = ptr;
									return *this;
								}

		inline_	PtrContainer&	AddPtr(const void* ptr)
								{
									return AddPtr(const_cast<void*>(ptr));
								}

		inline_	PtrContainer&	AddPtrs(void** entries, udword nb)
								{
									if(entries && nb)
									{
										// Resize if needed
										if(mCurNbEntries+nb>mMaxNbEntries)
											Resize(nb);

										// Add new entries
										CopyMemory(&mEntries[mCurNbEntries], entries, nb*sizeof(void*));
										mCurNbEntries += nb;
									}
									return *this;
								}

		inline_	PtrContainer&	AddPtrs(const void** entries, udword nb)
								{
									return AddPtrs(const_cast<void**>(entries), nb);
								}

		inline_	PtrContainer&	AddUnique(void* ptr)
								{
									if(!Contains(ptr))
										AddPtr(ptr);
									return *this;
								}

		inline_	PtrContainer&	AddUnique(const void* ptr)
								{
									return AddUnique(const_cast<void*>(ptr));
								}

				PtrContainer&	Empty();

		inline_	void			Reset()
								{
									// Avoid the write if possible
									// ### CMOV
									if(mCurNbEntries)
										mCurNbEntries = 0;
								}

		// HANDLE WITH CARE - I hope you know what you're doing
		inline_	void			ForceSize(udword size)
								{
									mCurNbEntries = size;
								}

				bool			SetSize(udword nb);
				bool			Contains(void* entry, udword* location=null) const;
				bool			Delete(void* entry);

		inline_	bool			Delete(const void* ptr)
								{
									return Delete(const_cast<void*>(ptr));
								}

		inline_	void			DeleteLastEntry()						{ if(mCurNbEntries)	mCurNbEntries--;			}
		inline_	void			DeleteIndex(udword index)				{ mEntries[index] = mEntries[--mCurNbEntries];	}

				bool			DeleteKeepingOrder(void* entry);

		// Data access.
		inline_	udword			GetNbEntries()					const	{ return mCurNbEntries;					}	//!< Returns the current number of entries.
		inline_	udword			GetMaxNbEntries()				const	{ return mMaxNbEntries;					}	//!< Returns max number of entries before resizing.
		inline_	void*			GetEntry(udword i)				const	{ return mEntries[i];					}	//!< Returns ith entry.
		inline_	void**			GetEntries()					const	{ return mEntries;						}	//!< Returns the list of entries.

		//! Read-access as an array
		inline_	void*			operator[](udword i)			const	{ ASSERT(i<mCurNbEntries); return mEntries[i];	}
		//! Write-access as an array
		inline_	void*&			operator[](udword i)					{ ASSERT(i<mCurNbEntries); return mEntries[i];	}

				void			operator = (const PtrContainer& object);

		private:
		// Resizing
				bool			Resize(udword needed=1);
				void			ShiftFrom(udword index);
		// Data
				udword			mMaxNbEntries;		//!< Maximum possible number of entries
				udword			mCurNbEntries;		//!< Current number of entries
				void**			mEntries;			//!< List of entries
				float			mGrowthFactor;		//!< Resize: new number of entries = old number * mGrowthFactor
	};

#endif // ICECONTAINER_H
