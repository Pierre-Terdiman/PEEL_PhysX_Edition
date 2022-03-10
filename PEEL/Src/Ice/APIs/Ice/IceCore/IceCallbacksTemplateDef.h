
// Cheap templates inspired from MESA

#ifdef TEMPLATED_CALLBACK
#ifdef TEMPLATED_CONTAINER
#ifdef TEMPLATED_MANAGER
#ifdef TEMPLATED_CODE
#ifdef TEMPLATED_PARAM
#ifdef TEMPLATED_SIZE

	// This class is a collection of callbacks
	class TEMPLATED_EXPORT TEMPLATED_CONTAINER : public PtrContainer
	{
		public:
		inline_								TEMPLATED_CONTAINER()						{}
		inline_								~TEMPLATED_CONTAINER()						{}

		// Returns the number of callbacks in the container
//		inline_			udword				GetNbCallbacks()					const	{ return GetNbEntries();						}
		inline_			udword				GetNbCallbacks()					const	{ return GetNbEntries()>>1;						}
		// Returns the whole list of callbacks
//		inline_			TEMPLATED_CALLBACK*	GetCallbacks()						const	{ return (TEMPLATED_CALLBACK*)GetEntries();		}
		// Returns the ith callback
//		inline_			TEMPLATED_CALLBACK	GetCallback(udword i)				const	{ return (TEMPLATED_CALLBACK)GetEntry(i);		}

		// Adds a callback to the container
		inline_			void				AddCallback(TEMPLATED_CALLBACK callback, void* user_data)	{ AddPtr((void*)callback).AddPtr(user_data);		}
		// Remove a callback from the container, otherwise preserving the insertion order
//		inline_			bool				RemoveCallback(TEMPLATED_CALLBACK callback)	{ return DeleteKeepingOrder(udword(callback));	}
						bool				RemoveCallback(TEMPLATED_CALLBACK callback);

		// Executes all callbacks in order
/*		inline_			void				ExecCallbacks(TEMPLATED_PARAM param, IceCallbackParams* context, udword user_data)
											{
												// Loop through callbacks
												udword NbCallbacks = GetNbEntries();
												TEMPLATED_CALLBACK* Callbacks = (TEMPLATED_CALLBACK*)GetEntries();
												while(NbCallbacks--)
												{
													// Get current callback
													TEMPLATED_CALLBACK cb = *Callbacks++;
													// Execute callback
													if(cb)	(cb)(param, context, user_data);
												}
											}*/
		inline_			void				ExecCallbacks(TEMPLATED_PARAM param, IceCallbackParams* context)
											{
												// Loop through callbacks
												udword NbCallbacks = GetNbCallbacks();
												void** Entries = GetEntries();
												while(NbCallbacks--)
												{
													// Get current callback
													TEMPLATED_CALLBACK cb = (TEMPLATED_CALLBACK)*Entries++;
													void* UserData = *Entries++;
													// Execute callback
													if(cb)	(cb)(param, context, UserData);
												}
											}
	};

	// This class manages a collection of collections of callbacks
	class TEMPLATED_EXPORT TEMPLATED_MANAGER : public Allocateable
	{
		public:
												TEMPLATED_MANAGER()/* : mUserData(0)*/
												{
													// Clear collection pointers
													ZeroMemory(mCollections, TEMPLATED_SIZE*sizeof(TEMPLATED_CONTAINER*));
												}

												~TEMPLATED_MANAGER();
/*		// Setup user-defined data
		inline_			void					SetUserData(udword data)				{ mUserData = data;		}
		// Get user-defined data
		inline_			udword					GetUserData()					const	{ return mUserData;		}
*/
		// Stores a callback in a collection
						bool					AddCallback(TEMPLATED_CODE code, TEMPLATED_CALLBACK callback, void* user_data);

		// Removes a callback from a collection
						bool					RemoveCallback(TEMPLATED_CODE code, TEMPLATED_CALLBACK callback);

		// Executes callbacks from a particular collection
		inline_			void					ExecCallbacks(TEMPLATED_CODE code, TEMPLATED_PARAM param, IceCallbackParams* context)
												{
													// Check the collection exists, then execute callbacks
													if(mCollections[code])	mCollections[code]->ExecCallbacks(param, context/*, mUserData*/);
												}
		private:
						/*udword					mUserData;*/
						TEMPLATED_CONTAINER*	mCollections[TEMPLATED_SIZE];
	};

#endif
#endif
#endif
#endif
#endif
#endif

#undef TEMPLATED_EXPORT
#undef TEMPLATED_CALLBACK
#undef TEMPLATED_CONTAINER
#undef TEMPLATED_MANAGER
#undef TEMPLATED_CODE
#undef TEMPLATED_PARAM
#undef TEMPLATED_SIZE
