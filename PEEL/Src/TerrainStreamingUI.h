///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_STREAMING_UI_H
#define TERRAIN_STREAMING_UI_H

#include "GUI_Helpers.h"
#include "PintGUIHelper.h"

	class PintGUIHelper;
	class Widgets;

	class TerrainStreamingUI
	{
		public:
								TerrainStreamingUI();
								~TerrainStreamingUI();

				sdword			CreateUI(PintGUIHelper& helper, Widgets& widgets, IceWindow* parent, sdword y);
				void			SetVisible(bool b);

		inline_	bool			DrawStreamingRegions()			const	{ return mCheckBox_DrawStreamingRegions && mCheckBox_DrawStreamingRegions->IsChecked();			}
		inline_	bool			DrawVertexNormals()				const	{ return mCheckBox_DrawVertexNormals && mCheckBox_DrawVertexNormals->IsChecked();				}
		inline_	bool			DrawWheelOverlapSpheres()		const	{ return mCheckBox_DrawWheelOverlapSpheres && mCheckBox_DrawWheelOverlapSpheres->IsChecked();	}
		inline_	bool			DeformTerrain()					const	{ return mCheckBox_DeformTerrain && mCheckBox_DeformTerrain->IsChecked();						}
		inline_	bool			CreateAllInitialTiles()			const	{ return mCheckBox_CreateAllInitialTiles && mCheckBox_CreateAllInitialTiles->IsChecked();		}

		inline_	float			GetWorldSize()					const	{ return GetFloat(30.0f*4.0f, mEditBox_StreamerWorldSize);										}
		inline_	udword			GetTilesPerSide()				const	{ return GetInt(8*4, mEditBox_StreamerTilesPerSide);											}
		inline_	udword			GetVerticesPerSide()			const	{ return GetInt(9, mEditBox_StreamerVerticesPerSide);											}
		inline_	udword			GetNbAddedRegionsPerFrame()		const	{ return GetInt(4, mEditBox_StreamerNbAddedRegionsPerFrame);									}
		inline_	float			GetWheelOverlapSphereRadius()	const	{ return GetFloat(0.75f, mEditBox_WheelOverlapSphereRadius);									}

				CheckBoxPtr		mCheckBox_DrawStreamingRegions;
				CheckBoxPtr		mCheckBox_DrawVertexNormals;
				CheckBoxPtr		mCheckBox_DrawWheelOverlapSpheres;
				CheckBoxPtr		mCheckBox_DeformTerrain;
				CheckBoxPtr		mCheckBox_CreateAllInitialTiles;

				EditBoxPtr		mEditBox_StreamerWorldSize;
				EditBoxPtr		mEditBox_StreamerTilesPerSide;
				EditBoxPtr		mEditBox_StreamerVerticesPerSide;
				EditBoxPtr		mEditBox_StreamerNbAddedRegionsPerFrame;
				EditBoxPtr		mEditBox_WheelOverlapSphereRadius;
	};

#endif
