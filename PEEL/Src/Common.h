///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef COMMON_H
#define COMMON_H

	inline_ float	degToRad(float a)
	{
		return (float)0.01745329251994329547 * a;
	}

	Quat ShortestRotation(const Point& v0, const Point& v1);

	const char* FindPEELFile(const char* filename);

	template<class T>
	static void DeleteOwnedObjects(PtrContainer& ptrs)
	{
		const udword Nb = ptrs.GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			T* Current = reinterpret_cast<T*>(ptrs.GetEntry(i));
			DELETESINGLE(Current);
		}
		ptrs.Empty();
	}

	// IndexedSurface is not an Allocateable in ICE (because of diamond inheritance issues).
	// So we just add our own tracking here to catch memory leaks.
	class TrackedIndexedSurface : public IndexedSurface, public Allocateable
	{
		public:
					TrackedIndexedSurface()		{}
		virtual		~TrackedIndexedSurface()	{}
	};

	void OutputConsoleError(const char* error);
	void OutputConsoleWarning(const char* warning);
	void OutputConsoleInfo(const char* info);

#endif
