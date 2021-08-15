///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrashCache.h"

static volatile int trashSum = 0;
void trashCache()
{
	const int size = 1024*1024*64;
	void* buf_ = ICE_ALLOC(size);
	volatile int* buf = (int*)buf_;
	printf("t");
	for (int j = 0; j < size/sizeof(buf[0]); j++)
	{
		trashSum += buf[j];
		buf[j] = 0xdeadbeef;
	}
	ICE_FREE(buf_);
}

/*
static const udword K = 2;
static const udword size = K*K*K*K*K;
__declspec(align(128)) char data[size*2+1];

void initData()
{
	for (udword i = 0; i < size*2+1; i++)
		data[i] = char(i*2971215073UL + 433494437UL);
}

template<int N>
void loop8()
{
#define LOOP \
	for (udword i = 1; i <= K; i++) \
		for (udword j = 1; j <= K; j++) \
			for (udword k = 1; k <= K; k++) \
				for (udword l = 1; l <= K; l++) \
					for (udword m = 1; m <= K; m++) { \
						udword ii = i*j*k*l*m; \
						if (data[ii-1] > data[size*2-ii]) \
						{ \
							char tmp = data[ii-1]; \
							data[ii-1] = data[size*2-ii]; \
							data[size*2-ii] = tmp; \
						} }
	LOOP LOOP LOOP LOOP
	LOOP LOOP LOOP LOOP
}

template<int N>
void loop8x16()
{
	loop8<0+N>();
	loop8<1+N>();
	loop8<2+N>();
	loop8<3+N>();
	loop8<4+N>();
	loop8<5+N>();
	loop8<6+N>();
	loop8<7+N>();
	loop8<8+N>();
	loop8<9+N>();
	loop8<10+N>();
	loop8<11+N>();
	loop8<12+N>();
	loop8<13+N>();
	loop8<14+N>();
	loop8<15+N>();
}

template<int N>
void loop8x16x8()
{
	loop8x16<0*N>();
	loop8x16<16*N>();
	loop8x16<32*N>();
	loop8x16<48*N>();
	loop8x16<64*N>();
	loop8x16<80*N>();
	loop8x16<96*N>();
	loop8x16<112*N>();

	loop8x16<128*N>();
	loop8x16<(128+16)*N>();
	loop8x16<(128+32)*N>();
	loop8x16<(128+48)*N>();
	loop8x16<(128+64)*N>();
	loop8x16<(128+80)*N>();
	loop8x16<(128+96)*N>();
	loop8x16<(128+112)*N>();
	loop8x16<(128+128)*N>();
}

void trashIcacheAndBranchPredictors()
{
	initData();
	loop8x16x8<1>();
	loop8x16x8<2>();
	loop8x16x8<3>();
	loop8x16x8<4>();
}
*/