///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Grid.h"
#include "Pint.h"

void RenderGrid(PintRender& renderer)
{
	const float gridSize = 200.0f;
	const float minX = -gridSize;
	const float maxX = gridSize;
	const float minZ = -gridSize;
	const float maxZ = gridSize;
	const float dX = 1.0f;
	const float dZ = 1.0f;

	const Point color(1.0f, 1.0f, 1.0f);

	float currentX = minX;
	while(currentX<=maxX)
	{
		renderer.DrawLine(Point(currentX, 0.0f, minZ), Point(currentX, 0.0f, maxZ), color);
		currentX += dX;
	}

	float currentZ = minZ;
	while(currentZ<=maxZ)
	{
		renderer.DrawLine(Point(minX, 0.0f, currentZ), Point(maxX, 0.0f, currentZ), color);
		currentZ += dZ;
	}
}
