///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TerrainStreamingUI.h"
#include "PintGUIHelper.h"

TerrainStreamingUI::TerrainStreamingUI()
{
}

TerrainStreamingUI::~TerrainStreamingUI()
{
}

sdword TerrainStreamingUI::CreateUI(PintGUIHelper& helper, Widgets& widgets, IceWindow* parent, sdword y)
{
	const sdword EditBoxWidth = 60;
	const sdword LabelWidth = 180;
	const sdword OffsetX = LabelWidth + 10;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;

	mCheckBox_DrawStreamingRegions = helper.CreateCheckBox(parent, 0, 4, y, 400, 20, "Debug draw streaming regions", &widgets, false, null, null);
	y += YStep;
	mCheckBox_DrawVertexNormals = helper.CreateCheckBox(parent, 0, 4, y, 400, 20, "Debug draw vertex normals", &widgets, false, null, null);
	y += YStep;
	mCheckBox_DrawWheelOverlapSpheres = helper.CreateCheckBox(parent, 0, 4, y, 400, 20, "Debug draw wheel overlap spheres", &widgets, false, null, null);
	y += YStep;
	mCheckBox_DeformTerrain = helper.CreateCheckBox(parent, 0, 4, y, 400, 20, "Deform terrain", &widgets, false, null, null);
	y += YStep;
	mCheckBox_CreateAllInitialTiles = helper.CreateCheckBox(parent, 0, 4, y, 400, 20, "Create all initial tiles", &widgets, true, null, null);
	y += YStep;

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, "World size:", &widgets);
	mEditBox_StreamerWorldSize = helper.CreateEditBox(parent, 1, 4+OffsetX, y, EditBoxWidth, 20, "120.0", &widgets, EDITBOX_FLOAT_POSITIVE, null, null);
	y += YStep;

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, "Nb tiles per side:", &widgets);
	mEditBox_StreamerTilesPerSide = helper.CreateEditBox(parent, 1, 4+OffsetX, y, EditBoxWidth, 20, "32", &widgets, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, "Nb vertices per side:", &widgets);
	mEditBox_StreamerVerticesPerSide = helper.CreateEditBox(parent, 1, 4+OffsetX, y, EditBoxWidth, 20, "9", &widgets, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, "Nb added regions per frame:", &widgets);
	mEditBox_StreamerNbAddedRegionsPerFrame = helper.CreateEditBox(parent, 1, 4+OffsetX, y, EditBoxWidth, 20, "4", &widgets, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, "Wheel overlap sphere radius:", &widgets);
	mEditBox_WheelOverlapSphereRadius = helper.CreateEditBox(parent, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.75", &widgets, EDITBOX_FLOAT_POSITIVE, null, null);
	y += YStep;

	return y;
}

static void _SetVisible(IceWidget* widget, bool b)
{
	if(widget)
		widget->SetVisible(b);
}

void TerrainStreamingUI::SetVisible(bool b)
{
	_SetVisible(mCheckBox_DrawStreamingRegions, b);
	_SetVisible(mCheckBox_DrawVertexNormals, b);
	_SetVisible(mCheckBox_DrawWheelOverlapSpheres, b);
	_SetVisible(mCheckBox_DeformTerrain, b);
	_SetVisible(mCheckBox_CreateAllInitialTiles, b);
	_SetVisible(mEditBox_StreamerWorldSize, b);
	_SetVisible(mEditBox_StreamerTilesPerSide, b);
	_SetVisible(mEditBox_StreamerVerticesPerSide, b);
	_SetVisible(mEditBox_StreamerNbAddedRegionsPerFrame, b);
	_SetVisible(mEditBox_WheelOverlapSphereRadius, b);
}