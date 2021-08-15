///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PROFILING_H
#define PROFILING_H

	enum ProfilingUnits
	{
		PROFILING_UNITS_RDTSC,
		PROFILING_UNITS_TIME_GET_TIME,
		PROFILING_UNITS_QPC,
	};

	enum UserProfilingMode
	{
		USER_PROFILING_DEFAULT,
		USER_PROFILING_SIM,
		USER_PROFILING_UPDATE,
		USER_PROFILING_COMBINED,
	};

	class PhysicsTest;
	struct EngineData;

	void	Simulate(	float timestep, ProfilingUnits profiling_units, UserProfilingMode user_profiling_mode,
						PhysicsTest* test, udword nb_engines, EngineData* engines,
						bool randomize_order, bool paused, bool trash_cache);

#endif
