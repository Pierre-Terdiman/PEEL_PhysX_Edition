///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Common.h"
#include "PintDLConvexShapeRenderer.h"
//#include "ConvexHull.h"

// A basic display-list-based convex renderer. Not batched.

//ConvexHull2* CreateConvexHull2b(udword nb_verts, const Point* verts);

Cvh* CreateConvexHull(udword nb_verts, const Point* verts)
{
#ifdef USE_NEW_CONVEX_HULL
//	ConvexHull2* CreateConvexHull2(udword nb_verts, const Point* verts);
	ConvexHull2* CH = CreateConvexHull2(nb_verts, verts);	// ICE
//	ConvexHull2* CH = CreateConvexHull2b(nb_verts, verts);	// Local
	if(!CH)
	{
		if(0)
		{
			Vertices MergedVerts;
			ConvexHull2* PartialHull = null;

			udword NbToGo = nb_verts;
			while(NbToGo)
			{
				udword Nb = NbToGo>100 ? 100 : NbToGo;

				for(udword i=0;i<Nb;i++)
					MergedVerts.AddVertex(verts[i]);

				DELETESINGLE(PartialHull);
				PartialHull = CreateConvexHull2(MergedVerts.GetNbVertices(), MergedVerts.GetVertices());
				ASSERT(PartialHull);

				MergedVerts.Reset();
				for(udword i=0;i<PartialHull->GetNbVerts();i++)
				{
					MergedVerts.AddVertex(PartialHull->GetVerts()[i]);
				}

				NbToGo -= Nb;
				verts += Nb;
			}
			CH = PartialHull;
		}

		if(0)
		{
			FILE* fp = fopen("d:/NVD/fail.bin", "wb");
			fwrite(verts, sizeof(Point), nb_verts, fp);
			fclose(fp);
		}


		if(1)
		{
			OutputConsoleError("Convex hull computation failed. Using a box instead.\n");

			AABB Box;
			ComputeAABB(Box, verts, nb_verts);

			Point Pts[8];
			Box.ComputePoints(Pts);

			CH = CreateConvexHull2(8, Pts);
		}


	}
#else
	ConvexHull* CH = ICE_NEW(ConvexHull);
	ASSERT(CH);

	CONVEXHULLCREATE Create;
	Create.NbVerts		= nb_verts;
	Create.Vertices		= verts;
	Create.UnifyNormals	= true;
	Create.PolygonData	= true;

	bool status = CH->Compute(Create);
	ASSERT(status);
	if(!status)
	{
		DELETESINGLE(CH);
	}
#endif
	return CH;
}

void DrawHull(const Cvh& hull)
{
	const Point* ConvexVerts = hull.GetVerts();
	const udword NbPolys = hull.GetNbPolygons();
//	printf("NbPolys: %d\n", NbPolys);
	glEnableClientState(GL_VERTEX_ARRAY);
	for(udword i=0;i<NbPolys;i++)
	{
		const HullPolygon& PolygonData = hull.GetPolygon(i);
		glNormal3f(PolygonData.mPlane.n.x, PolygonData.mPlane.n.y, PolygonData.mPlane.n.z);

		const udword NbVertsInPoly = PolygonData.mNbVerts;
		const udword NbTris = NbVertsInPoly - 2;
		const udword* Indices = PolygonData.mVRef;
		udword Offset = 1;
		if(0)
		{
			for(udword i=0;i<NbTris;i++)
			{
				const udword VRef0 = Indices[0];
				const udword VRef1 = Indices[Offset];
				const udword VRef2 = Indices[Offset+1];
				Offset++;

				const Point av3LineEndpoints[] = {ConvexVerts[VRef0], ConvexVerts[VRef1], ConvexVerts[VRef2]};
				glVertexPointer(3, GL_FLOAT, sizeof(Point), &av3LineEndpoints[0].x);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
		}
		else
		{
			if(1)
			{
				Point Vertices[1024];
				for(udword i=0;i<NbVertsInPoly;i++)
				{
					Vertices[i] = ConvexVerts[Indices[i]];
				}
				glVertexPointer(3, GL_FLOAT, sizeof(Point), &Vertices[0].x);
				glDrawArrays(GL_POLYGON, 0, NbVertsInPoly);
			}
			else
			{
				Point Vertices[1024];
				for(udword i=0;i<NbVertsInPoly;i++)
				{
					Vertices[i] = ConvexVerts[Indices[i]];
				}
				glVertexPointer(3, GL_FLOAT, sizeof(Point), &Vertices[0].x);
				glDrawArrays(GL_TRIANGLE_FAN, 0, NbVertsInPoly);
			}
		}
	}
	glDisableClientState(GL_VERTEX_ARRAY);
}

PintDLConvexShapeRenderer::PintDLConvexShapeRenderer(udword nb_verts, const Point* verts)
{
	Cvh* CHull = CreateConvexHull(nb_verts, verts);
	ASSERT(CHull);
//	mNbVerts = CHull->GetNbVerts();
	CreateHullDisplayList(mDisplayListNum, CHull);
	DELETESINGLE(CHull);
}
