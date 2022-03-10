
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEINSTRUMENTTRACE_H
#define ICEINSTRUMENTTRACE_H

	class ICECORE_API InstrumentTrace
	{
	#ifndef NO_INSTRUMENTATION
		private:
			bool	dataLinked;
			void*	data;

			void	LinkData();

			// Public interface.
	public:
			inline_			InstrumentTrace(): dataLinked(false), data(0)		{}		// Public constructor.
			inline_			~InstrumentTrace()									{}		// Public destructor.

			inline_	void	Set(int i, void *_data, int size)	{ if (!dataLinked) LinkData(); if (data) memcpy((int*)data + i, _data, size);	}
			inline_	void	Set(int i, int val)					{ if (!dataLinked) LinkData(); if (data) ((int*)data)[i] = val;					}
			inline_	int		Get(int i)							{ if (!dataLinked) LinkData(); if (data) return ((int*)data)[i]; else return 0;	}

	#else

	public:
			inline_	void	Set(int i, void *data, int size)	{}
			inline_	void	Set(int i, int val)					{}
			inline_	int		Get(int i)							{ return 0; }
	#endif
	};

#endif // ICEINSTRUMENTTRACE_H
