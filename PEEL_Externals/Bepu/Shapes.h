#pragma once

#include "InteropMath.h"
#include "Handles.h"
#include "Utilities.h"
#include "Tree.h"
#include <limits>

namespace Bepu
{
	enum struct ShapeTypes
	{
		Sphere = 0,
		Capsule = 1,
		Box = 2,
		Triangle = 3,
		Cylinder = 4,
		ConvexHull = 5,
		Compound = 6,
		BigCompound = 7,
		Mesh = 8
	};

	struct Sphere
	{
		/// <summary>
		/// Radius of the sphere.
		/// </summary>
		float Radius;
	};

	/// <summary>
	/// Collision shape representing a sphere-expanded line segment.
	/// </summary>
	struct Capsule
	{
		/// <summary>
		/// Spherical expansion applied to the internal line segment.
		/// </summary>
		float Radius;
		/// <summary>
		/// Half of the length of the internal line segment. Oriented along the local Y axis.
		/// </summary>
		float HalfLength;
	};

	/// <summary>
	/// Collision shape representing a solid cuboid.
	/// </summary>
	struct Box
	{
		/// <summary>
		/// Half of the box's width along its local X axis.
		/// </summary>
		float HalfWidth;
		/// <summary>
		/// Half of the box's height along its local Y axis.
		/// </summary>
		float HalfHeight;
		/// <summary>
		/// Half of the box's length along its local Z axis.
		/// </summary>
		float HalfLength;

		Box(float width, float height, float length)
		{
			HalfWidth = width * 0.5f;
			HalfHeight = height * 0.5f;
			HalfLength = length * 0.5f;
		}
	};

	/// <summary>
	/// Collision shape representing an individual triangle. Triangle collisions and ray tests are one-sided; only tests which see the triangle as wound clockwise in right handed coordinates or counterclockwise in left handed coordinates will generate contacts.
	/// </summary>
	struct Triangle
	{
		/// <summary>
		/// First vertex of the triangle in local space.
		/// </summary>
		Vector3 A;
		/// <summary>
		/// Second vertex of the triangle in local space.
		/// </summary>
		Vector3 B;
		/// <summary>
		/// Third vertex of the triangle in local space.
		/// </summary>
		Vector3 C;
	};

	/// <summary>
   /// Collision shape representing a cylinder.
   /// </summary>
	struct Cylinder
	{
		/// <summary>
		/// Radius of the cylinder.
		/// </summary>
		float Radius;
		/// <summary>
		/// Half length of the cylinder along its local Y axis.
		/// </summary>
		float HalfLength;
	};

	struct HullVertexIndex
	{
		//This means you can only have Vector<float>.Count * 65536 points in a convex hull. Oh no!
		uint16_t BundleIndex;
		uint16_t InnerIndex;
	};

	/// <summary>
	/// Dummy type standing in for the compile time variable width Vector3Wide type.
	/// Pointers to buffers of this type should be reinterpreted to either Vector3SIMD128 or Vector3SIMD256 depending on what SIMD width is in use.
	/// </summary>
	struct Vector3Wide
	{

	};

	/// <summary>
	/// Dummy type standing in for the compile time variable width HullBoundingPlanes type.
	/// Pointers to buffers of this type should be reinterpreted to either HullBoundingPlanesSIMD128 or HullBoundingPlanesSIMD256 depending on what SIMD width is in use.
	/// </summary>
	struct HullBoundingPlanes
	{

	};

	struct HullBoundingPlanesSIMD128
	{
		Vector3SIMD128 Normal;
		Vector128F Offset;
	};
	struct HullBoundingPlanesSIMD256
	{
		Vector3SIMD256 Normal;
		Vector256F Offset;
	};

	struct ConvexHull
	{
		/// <summary>
		/// Bundled points of the convex hull.
		/// </summary>
		Buffer<Vector3Wide> Points;
		/// <summary>
		/// Bundled bounding planes associated with the convex hull's faces.
		/// </summary>
		Buffer<HullBoundingPlanes> BoundingPlanes;
		/// <summary>
		/// Combined set of vertices used by each face. Use FaceToVertexIndicesStart to index into this for a particular face. Indices stored in counterclockwise winding in right handed space, clockwise in left handed space.
		/// </summary>
		Buffer<HullVertexIndex> FaceVertexIndices;
		/// <summary>
		/// Start indices of faces in the FaceVertexIndices.
		/// </summary>
		Buffer<int32_t> FaceToVertexIndicesStart;
	};

	/// <summary>
	/// Shape and pose of a child within a compound shape.
	/// </summary>
	struct CompoundChild
	{
		/// <summary>
		/// Local orientation of the child in the compound.
		/// </summary>
		Quaternion LocalOrientation;
		/// <summary>
		/// Local position of the child in the compound.
		/// </summary>
		Vector3 LocalPosition;
		/// <summary>
		/// Index of the shape within whatever shape collection holds the compound's child shape data.
		/// </summary>
		TypedIndex ShapeIndex;
	};

	/// <summary>
	/// Minimalist compound shape containing a list of child shapes. Does not make use of any internal acceleration structure; should be used only with small groups of shapes.
	/// </summary>
	struct Compound
	{
		/// <summary>
		/// Buffer of children within this compound.
		/// </summary>
		Buffer<CompoundChild> Children;
	};

	/// <summary>
	/// Compound shape containing a bunch of shapes accessible through a tree acceleration structure. Useful for compounds with lots of children.
	/// </summary>
	struct BigCompound
	{
		/// <summary>
		/// Acceleration structure for the compound children.
		/// </summary>
		Tree Tree;
		/// <summary>
		/// Buffer of children within this compound.
		/// </summary>
		Buffer<CompoundChild> Children;
	};

	struct Mesh
	{
		/// <summary>
		/// Acceleration structure of the mesh.
		/// </summary>
		Tree Tree;
		/// <summary>
		/// Buffer of triangles composing the mesh. Triangles will only collide with tests which see the triangle as wound clockwise in right handed coordinates or counterclockwise in left handed coordinates.
		/// </summary>
		Buffer<Triangle> Triangles;
		Vector3 Scale;
		Vector3 InverseScale;

		void SetScale(Vector3 scale)
		{
			Scale = scale;
			InverseScale.X = scale.X != 0 ? 1.0f / scale.X : std::numeric_limits<float>::max();
			InverseScale.Y = scale.Y != 0 ? 1.0f / scale.Y : std::numeric_limits<float>::max();
			InverseScale.Z = scale.Z != 0 ? 1.0f / scale.Z : std::numeric_limits<float>::max();
		}
	};
}