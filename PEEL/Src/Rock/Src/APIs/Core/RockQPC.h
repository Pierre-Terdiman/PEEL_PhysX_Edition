#ifndef ROCK_QPC_H
#define ROCK_QPC_H

#include <Core/RockCommon.h>

namespace Rock
{
	struct CounterFrequencyToTensOfNanos
	{
		unsigned __int64	mNumerator;
		unsigned __int64	mDenominator;

		CounterFrequencyToTensOfNanos(unsigned __int64 inNum, unsigned __int64 inDenom) : mNumerator(inNum), mDenominator(inDenom)	{}

		unsigned __int64 toTensOfNanos(unsigned __int64 inCounter) const
		{
			return (inCounter * mNumerator) / mDenominator;
		}
	};

	class ROCK_API QPCTime
	{
	public:
		typedef double Second;
		static const unsigned __int64 sNumTensOfNanoSecondsInASecond = 100000000;
		//This is supposedly guaranteed to not change after system boot
		//regardless of processors, speedstep, etc.		
		static const CounterFrequencyToTensOfNanos& getBootCounterFrequency();

		static CounterFrequencyToTensOfNanos getCounterFrequency();

		static unsigned __int64 getCurrentCounterValue();

		//SLOW!!
		//Thar be a 64 bit divide in thar!
		static unsigned __int64 getCurrentTimeInTensOfNanoSeconds()
		{
			unsigned __int64 ticks = getCurrentCounterValue();
			return getBootCounterFrequency().toTensOfNanos(ticks);
		}

		QPCTime();
		Second getElapsedSeconds();
		Second peekElapsedSeconds()	const;
		Second getLastTime() const;

		unsigned int peekElapsedUs()	const;

	private:
		signed __int64	mTickCount;
	};
}

#endif
