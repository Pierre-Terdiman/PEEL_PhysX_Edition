///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by multiple PINT plug-ins, don't use engine-specific structures here.

#include "stdafx.h"
#include "PINT_CommonShapeS.h"

CHECK_CONTAINER_ITEM(InternalSphereShapeData);
CHECK_CONTAINER_ITEM(InternalBoxShapeData);
CHECK_CONTAINER_ITEM(InternalCapsuleShapeData);
CHECK_CONTAINER_ITEM(InternalMeshShapeData);

void* SharedSphereShapes::FindShape(float radius, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) const
{
	udword Size = GetNbShapes();
	const InternalSphereShapeData* Data = GetShapes();
	while(Size--)
	{
		const InternalSphereShapeData& CurrentShape = *Data++;
		if(		CurrentShape.mRadius==radius
			&&	CurrentShape.Compare(material, renderer, local_pose, collision_group))
			return CurrentShape.mShape;
	}
	return null;
}

void* SharedBoxShapes::FindShape(const Point& extents, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) const
{
	udword Size = GetNbShapes();
	const InternalBoxShapeData* Data = GetShapes();
	while(Size--)
	{
		const InternalBoxShapeData& CurrentShape = *Data++;
		if(		CurrentShape.mExtents==extents
			&&	CurrentShape.Compare(material, renderer, local_pose, collision_group))
			return CurrentShape.mShape;
	}
	return null;
}

void* SharedCapsuleShapes::FindShape(float radius, float half_height, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) const
{
	udword Size = GetNbShapes();
	const InternalCapsuleShapeData* Data = GetShapes();
	while(Size--)
	{
		const InternalCapsuleShapeData& CurrentShape = *Data++;
		if(		CurrentShape.mRadius==radius
			&&	CurrentShape.mHalfHeight==half_height
			&&	CurrentShape.Compare(material, renderer, local_pose, collision_group))
			return CurrentShape.mShape;
	}
	return null;
}

void* SharedMeshShapes::FindShape(const void* mesh, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) const
{
	udword Size = GetNbShapes();
	const InternalMeshShapeData* Data = GetShapes();
	while(Size--)
	{
		const InternalMeshShapeData& CurrentShape = *Data++;
		if(		CurrentShape.mMesh==mesh
			&&	CurrentShape.Compare(material, renderer, local_pose, collision_group))
			return CurrentShape.mShape;
	}
	return null;
}
