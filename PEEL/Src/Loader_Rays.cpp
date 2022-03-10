///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader_Rays.h"

/*
static bool LoadRays(const char* filename, TestBase& test, bool only_rays, bool no_processing)
{
	IceFile BinFile(filename);
	if(!BinFile.IsValid())
		return false;

	const udword NbRays = BinFile.LoadDword();

	const float Scale = gValveScale;
	udword NbIsRays = 0;
	udword NbIsSwept = 0;
	udword NbAligned = 0;
	udword NbAlignedUp = 0;
	udword NbAlignedDown = 0;
//	FILE* fp = fopen("d:\\rays.txt", "w");

	Matrix3x3 Idt;
	Idt.Identity();

#ifdef VALVE_ROTATE45
	Matrix3x3 Rot;
	Rot.RotX(45.0f * DEGTORAD);
#endif

	if(no_processing)
	{
		for(udword i=0;i<NbRays;i++)
		{
			Source1_Ray_t RayData;
			BinFile.LoadBuffer(&RayData, sizeof(Source1_Ray_t));

			const Point Origin(	RayData.m_Start.x+RayData.m_StartOffset.x,
								RayData.m_Start.y+RayData.m_StartOffset.y,
								RayData.m_Start.z+RayData.m_StartOffset.z);
			Point Dir(RayData.m_Delta.x, RayData.m_Delta.y, RayData.m_Delta.z);

			const float MaxDist = Dir.Magnitude();
			if(MaxDist!=0.0f)
			{
	//			fprintf(fp, "%f\n", MaxDist*Scale);
				Dir/=MaxDist;

				if(only_rays || RayData.m_IsRay)
				{
					test.RegisterRaycast(Origin, Dir, MaxDist);
				}
				else
				{
					const Point Extents(RayData.m_Extents.x, RayData.m_Extents.y, RayData.m_Extents.z);
					const OBB Box(Origin, Extents, Idt);
					test.RegisterBoxSweep(Box, Dir, MaxDist*Scale);
				}
			}
		}
	}
	else
	{
		for(udword i=0;i<NbRays;i++)
		{
			Source1_Ray_t RayData;
			BinFile.LoadBuffer(&RayData, sizeof(Source1_Ray_t));

			if(RayData.m_IsRay)
				NbIsRays++;
			if(RayData.m_IsSwept)
				NbIsSwept++;

			const Point Origin(	(RayData.m_Start.x+RayData.m_StartOffset.x)*Scale,
								(RayData.m_Start.z+RayData.m_StartOffset.z)*Scale,
								(RayData.m_Start.y+RayData.m_StartOffset.y)*Scale);
			Point Dir(RayData.m_Delta.x, RayData.m_Delta.z, RayData.m_Delta.y);

			const float MaxDist = Dir.Magnitude();
			if(MaxDist!=0.0f)
			{
	//			fprintf(fp, "%f\n", MaxDist*Scale);
				Dir/=MaxDist;

				if(Dir.x==0.0f && Dir.z==0.0f)
				{
					NbAligned++;
					if(Dir.y>0.0f)
						NbAlignedUp++;
					else
						NbAlignedDown++;
				}

				if(only_rays || RayData.m_IsRay)
				{
#ifdef VALVE_ROTATE45
					test.RegisterRaycast(Origin*Rot, Dir*Rot, MaxDist*Scale);
#else
					test.RegisterRaycast(Origin, Dir, MaxDist*Scale);
#endif
				}
				else
				{
					const Point Extents(RayData.m_Extents.x*Scale, RayData.m_Extents.z*Scale, RayData.m_Extents.y*Scale);
					const OBB Box(Origin, Extents, Idt);
					test.RegisterBoxSweep(Box, Dir, MaxDist*Scale);
				}
			}
		}
	}
//	fclose(fp);
	return true;
}

void LoadRaysFile(TestBase& test, const char* filename, bool only_rays, bool no_processing)
{
	ASSERT(filename);

	if(!LoadRays(_F("../build/%s", filename), test, only_rays, no_processing))
		if(!LoadRays(_F("./%s", filename), test, only_rays, no_processing))
			printf(_F("Failed to load '%s'\n", filename));
}
*/