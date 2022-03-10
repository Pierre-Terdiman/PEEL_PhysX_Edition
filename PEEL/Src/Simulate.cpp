///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Profiling.h"
#include "PEEL.h"
#include "QPCTime.h"
#include "Pint.h"
#include "PintEngineData.h"
#include "TestScenes.h"
#include "TrashCache.h"

/*
The "profile update" in PEEL 1.x was confusing. We need a better system:

- first, we need to introduce a new profilable PostUpdate function (for tests). This could be used to fix
  the "flawed tests" that measure the add actors time. Specifically we could create new objects in the
  regular update function, store them locally, and add them to the scene in the PostUpdate function. The
  point is that we could profile the "interesting" physics bit (addActors) without polluting the results
  with all the Pint-level management code needed to create the objects.

- for this to make sense each test should be able to define which function it wants to profile. This was
  previously achieved with the "ProfileUpdate" function but we need more than a bool if we introduce an
  extra function. Also, the bool was not enough because ideally the test itself could tell the system to
  measure the "combo" of sim + test update. This suggests a design where the "ProfileUpdate" function
  returns a combination of flags (one flag per profilable function).

- on top of that there's the UI to take into account. One thing to note about it is that we should get
  rid of "SQ" in the names there, since this is also needed to "addActors" performance tests. Beyond that
  I guess we should introduce options like "let the test decide" (the default), and ways to override that.
  The only issue is that the "combo" mode becomes tedious with more than 2 profilable functions (there's
  suddenly more than one possibility for that "combo").

- finally there's the question of what we display on screen: SQ tests typically display numbers of hits
  instead of the usual info. I suspect this should be entirely decoupled from the "ProfileUpdate" function.
*/

static QPCTime	mQPCTimer;

extern udword	gFrameNb;

static inline_ void	StartProfile_RDTSC(udword& val)
{
#ifdef _WIN64
	val = udword(__rdtsc());
#else
	__asm{
		cpuid
		rdtsc
		mov		ebx, val
		mov		[ebx], eax
	}
#endif
}

static inline_ void	EndProfile_RDTSC(udword& val)
{
#ifdef _WIN64
	val = udword(__rdtsc()) - val;
#else
	__asm{
		cpuid
		rdtsc
		mov		ebx, val
		sub		eax, [ebx]
		mov		[ebx], eax
	}
#endif
}

static inline_ void	StartProfile_TimeGetTime(udword& val)
{
	val = timeGetTime();
}

static inline_ void	EndProfile_TimeGetTime(udword& val)
{
	val = timeGetTime() - val;
}

static inline_ void	StartProfile_QPC()
{
	mQPCTimer.getElapsedSeconds();
}

static inline_ void	EndProfile_QPC(QPCTime::Second& val)
{
	val = mQPCTimer.getElapsedSeconds();
}

///////////////////////////////////////////////////////////////////////////////

static udword ProfileSimUpdate_RDTSC(EngineData& engine, float dt)
{
	udword val, CurrentMemory;
	{
		::StartProfile_RDTSC(val);
			CurrentMemory = engine.mEngine->Update(dt);
		::EndProfile_RDTSC(val);
	}
	const udword Time = val/1024;
	engine.mTiming.RecordTimeAndMemory(Time, CurrentMemory, gFrameNb);
	engine.mTiming.mCurrentTestResult = INVALID_ID;
	return Time;
}

static udword ProfileSimUpdate_TimeGetTime(EngineData& engine, float dt)
{
	udword val, CurrentMemory;
	{
		::StartProfile_TimeGetTime(val);
			CurrentMemory = engine.mEngine->Update(dt);
		::EndProfile_TimeGetTime(val);
	}
	const udword Time = val;
	engine.mTiming.RecordTimeAndMemory(Time, CurrentMemory, gFrameNb);
	engine.mTiming.mCurrentTestResult = INVALID_ID;
	return Time;
}

static udword ProfileSimUpdate_QPC(EngineData& engine, float dt)
{
	udword val, CurrentMemory;
	{
		QPCTime::Second s;
		::StartProfile_QPC();
			CurrentMemory = engine.mEngine->Update(dt);
		::EndProfile_QPC(s);
		val = udword(s*1000000.0);
	}
	const udword Time = val;
	engine.mTiming.RecordTimeAndMemory(Time, CurrentMemory, gFrameNb);
	engine.mTiming.mCurrentTestResult = INVALID_ID;
	return Time;
}

static void NoProfileUpdate(EngineData& engine, float dt)
{
	engine.mEngine->Update(dt);
}

static inline_ udword ProfileSimUpdate(ProfilingUnits profiling_units, EngineData& engine, float dt)
{
	if(profiling_units==PROFILING_UNITS_RDTSC)
		return ProfileSimUpdate_RDTSC(engine, dt);
	else if(profiling_units==PROFILING_UNITS_TIME_GET_TIME)
		return ProfileSimUpdate_TimeGetTime(engine, dt);
	else if(profiling_units==PROFILING_UNITS_QPC)
		return ProfileSimUpdate_QPC(engine, dt);
	ASSERT(0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<const bool combined, const bool post_update>
static udword ProfileTestUpdate_RDTSC(PhysicsTest* test, EngineData& engine, float dt)
{
	udword val, TestResult;
	{
		::StartProfile_RDTSC(val);
		{
			if(post_update)
				TestResult = test->PostUpdate(*engine.mEngine, dt);
			else
				TestResult = test->Update(*engine.mEngine, dt);
		}
		::EndProfile_RDTSC(val);
	}
	const udword Time = val/1024;
	if(combined)
		engine.mTiming.UpdateRecordedTime(Time, gFrameNb);
	else
		engine.mTiming.RecordTimeAndMemory(Time, 0, gFrameNb);
	engine.mTiming.mCurrentTestResult = TestResult;
	return Time;
}

template<const bool combined, const bool post_update>
static udword ProfileTestUpdate_TimeGetTime(PhysicsTest* test, EngineData& engine, float dt)
{
	udword val, TestResult;
	{
		::StartProfile_TimeGetTime(val);
		{
			if(post_update)
				TestResult = test->PostUpdate(*engine.mEngine, dt);
			else
				TestResult = test->Update(*engine.mEngine, dt);
		}
		::EndProfile_TimeGetTime(val);
	}
	const udword Time = val;
	if(combined)
		engine.mTiming.UpdateRecordedTime(Time, gFrameNb);
	else
		engine.mTiming.RecordTimeAndMemory(Time, 0, gFrameNb);
	engine.mTiming.mCurrentTestResult = TestResult;
	return Time;
}

template<const bool combined, const bool post_update>
static udword ProfileTestUpdate_QPC(PhysicsTest* test, EngineData& engine, float dt)
{
	udword val, TestResult;
	{
		QPCTime::Second s;
		::StartProfile_QPC();
		{
			if(post_update)
				TestResult = test->PostUpdate(*engine.mEngine, dt);
			else
				TestResult = test->Update(*engine.mEngine, dt);
		}
		::EndProfile_QPC(s);
		val = udword(s*1000000.0);
	}
	const udword Time = val;
	if(combined)
		engine.mTiming.UpdateRecordedTime(Time, gFrameNb);
	else
		engine.mTiming.RecordTimeAndMemory(Time, 0, gFrameNb);
	engine.mTiming.mCurrentTestResult = TestResult;
	return Time;
}

template<const bool combined, const bool post_update>
static inline_ udword ProfileTestUpdate(ProfilingUnits profiling_units, PhysicsTest* test, EngineData& engine, float dt)
{
	if(profiling_units==PROFILING_UNITS_RDTSC)
		return ProfileTestUpdate_RDTSC<combined, post_update>(test, engine, dt);
	else if(profiling_units==PROFILING_UNITS_TIME_GET_TIME)
		return ProfileTestUpdate_TimeGetTime<combined, post_update>(test, engine, dt);
	else if(profiling_units==PROFILING_UNITS_QPC)
		return ProfileTestUpdate_QPC<combined, post_update>(test, engine, dt);
	ASSERT(0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void Simulate(float timestep, ProfilingUnits profiling_units, UserProfilingMode user_profiling_mode,
			  PhysicsTest* test, udword nb_engines, EngineData* engines,
			  bool randomize_order, bool paused, bool trash_cache)
{
//	printf("Simulate starts\n");
	/*if(0 && trash_cache)
	{
		EnableRendering(true);
		static udword Count=0;
		Count++;
		if(Count!=16)
			return;
		Count=0;
	}*/

	if(paused)
		return;

	const float dt = timestep;
//	const float dt = paused ? 0.0f : timestep;

	if(test)
	{
		SPY_ZONE("test CommonPreSimUpdate")

		if(IceCore::GetFocus()!=GetWindowHandle())
			test->LostFocus();

		test->CommonPreSimUpdate(dt);
	}

	Permutation P;
	P.Init(nb_engines);	// TODO: remove this alloc
	if(randomize_order)
		P.Random(nb_engines*2);
	else
		P.Identity();

	const bool CompatibilityMode = false;

	const udword PFlags = test ? test->GetProfilingFlags() : PROFILING_SIM_UPDATE;
	const bool MustProfileTestUpdate = (PFlags & PROFILING_TEST_UPDATE)!=0;

	// By default (USER_PROFILING_DEFAULT) we let the test dictate the profiling flags.
	// But we can let users override these via the UI.
	udword NewFlags = PFlags;
	if(user_profiling_mode==USER_PROFILING_SIM)
		NewFlags = PROFILING_SIM_UPDATE;
	else if(user_profiling_mode==USER_PROFILING_UPDATE)
	{
		NewFlags = PROFILING_TEST_UPDATE;
		if(PFlags & PROFILING_TEST_POST_UPDATE)
			NewFlags = PROFILING_TEST_POST_UPDATE;
	}
	else if(user_profiling_mode==USER_PROFILING_COMBINED)
	{
		NewFlags = PROFILING_SIM_UPDATE;
		if(PFlags & PROFILING_TEST_UPDATE)
			NewFlags |= PROFILING_TEST_UPDATE;
		if(PFlags & PROFILING_TEST_POST_UPDATE)
			NewFlags |= PROFILING_TEST_POST_UPDATE;
	}

	for(udword ii=0;ii<nb_engines;ii++)
	{
		const udword i = P[ii];
		if(!engines[i].mEnabled || !engines[i].mSupportsCurrentTest)
			continue;

		ASSERT(engines[i].mEngine);

		{
			SPY_ZONE("Physics engine update")
			if(CompatibilityMode)
			{
				if(MustProfileTestUpdate && user_profiling_mode==USER_PROFILING_UPDATE)
				{
					NoProfileUpdate(engines[i], dt);
				}
				else
				{
					ProfileSimUpdate(profiling_units, engines[i], dt);
				}
			}
			else
			{
				if(NewFlags & PROFILING_SIM_UPDATE)
					ProfileSimUpdate(profiling_units, engines[i], dt);
				else
					NoProfileUpdate(engines[i], dt);
			}
		}

		{
			SPY_ZONE("UpdateNonProfiled")
			engines[i].mEngine->UpdateNonProfiled(dt);
		}

		if(trash_cache)
			trashCache();
//			trashIcacheAndBranchPredictors();
	}

	if(test)
	{
		{
			SPY_ZONE("test CommonUpdate")
			test->CommonUpdate(dt);
		}

		for(udword ii=0;ii<nb_engines;ii++)
		{
			const udword i = P[ii];
			if(!engines[i].mEnabled || !engines[i].mSupportsCurrentTest)
				continue;

			ASSERT(engines[i].mEngine);

			{
				SPY_ZONE("test PreUpdate")
				test->PreUpdate(*engines[i].mEngine, dt);
			}

			{
				SPY_ZONE("test update")
				if(CompatibilityMode)
				{
					if(!MustProfileTestUpdate || user_profiling_mode==USER_PROFILING_SIM)
					{
						// Normal update without profiling
						test->Update(*engines[i].mEngine, dt);
					}
					else if(user_profiling_mode==USER_PROFILING_UPDATE)
					{
						ProfileTestUpdate<false, false>(profiling_units, test, engines[i], dt);
					}
					else
					{
						ProfileTestUpdate<true, false>(profiling_units, test, engines[i], dt);
					}
				}
				else
				{
					if(NewFlags & PROFILING_TEST_UPDATE)
					{
						if(NewFlags & PROFILING_SIM_UPDATE)
							ProfileTestUpdate<true, false>(profiling_units, test, engines[i], dt);
						else
							ProfileTestUpdate<false, false>(profiling_units, test, engines[i], dt);
					}
					else
					{
						test->Update(*engines[i].mEngine, dt);
					}
				}
			}

			{
				SPY_ZONE("test post update")
				if(NewFlags & PROFILING_TEST_POST_UPDATE)
				{
					if(NewFlags & PROFILING_SIM_UPDATE)
						ProfileTestUpdate<true, true>(profiling_units, test, engines[i], dt);
					else
						ProfileTestUpdate<false, true>(profiling_units, test, engines[i], dt);
				}
				else
				{
					test->PostUpdate(*engines[i].mEngine, dt);
				}
			}
		}
	}

	gFrameNb++;
//	printf("Simulate ends\n");
}
