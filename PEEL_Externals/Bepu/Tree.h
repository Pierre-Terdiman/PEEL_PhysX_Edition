#pragma once

#include "Utilities.h"
#include "InteropMath.h"
#include <assert.h>

namespace Bepu
{
	//If we ever expand this API, the tree has a lot of dedicated functionality and so it's worth splitting out like this.

	struct NodeChild
	{
		Vector3 Min;
		int32_t Index;
		Vector3 Max;
		int32_t LeafCount;
	};

	//Note that the format of this node implies that we don't explicitly test against the root bounding box during normal execution.
	//For almost all broad phase use cases, queries will be inside the root bounding box anyway. For non-broad phase uses, the outer bounding box will likely be stored
	//elsewhere- for example, in the broad phase.

	/// <summary>
	/// 2-wide tree node.
	/// </summary>
	struct Node
	{
		NodeChild A;
		NodeChild B;
	};

	//Node metadata isn't required or used during collision testing, so it is stored separately.
	//This helps avoid splitting Nodes across cache lines and decreases memory bandwidth requirements during testing.
	/// <summary>
	/// Metadata associated with a 2-child tree node.
	/// </summary>
	struct Metanode
	{
		int32_t Parent;
		int32_t IndexInParent;
		int32_t PackedFlagAndCostChange;
	};

	/// <summary>
	/// Pointer to a leaf's tree location.
	/// </summary>
	/// <remarks>The identity of a leaf is implicit in its position within the leaf array.</remarks>
	struct Leaf
	{
		uint32_t packed;
		/// <summary>
		/// Gets the index of the node that the leaf is directly held by.
		/// </summary>
		int GetNodeIndex()
		{
			return (int)(packed & 0x7FFFFFFF);
		}
		/// <summary>
		/// Gets which child within the owning node the leaf is in.
		/// </summary>
		int GetChildIndex()
		{
			return (int)((packed & 0x80000000) >> 31);
		}


		Leaf(int nodeIndex, int childIndex)
		{
			assert((childIndex & ~1) == 0);
			packed = ((uint32_t)nodeIndex & 0x7FFFFFFF) | ((uint32_t)childIndex << 31);
		}
	};

	struct Tree
	{
		/// <summary>
		/// Buffer of nodes in the tree.
		/// </summary>
		Buffer<Node> Nodes;
		/// <summary>
		/// Buffer of metanodes in the tree. Metanodes contain metadata that aren't read during most query operations but are useful for bookkeeping.
		/// </summary>
		Buffer<Metanode> Metanodes;
		/// <summary>
		/// Buffer of leaves in the tree.
		/// </summary>
		Buffer<Leaf> Leaves;
		/// <summary>
		/// Number of nodes in the tree.
		/// </summary>
		int NodeCount;
		/// <summary>
		/// Number of leaves in the tree.
		/// </summary>
		int LeafCount;
	};
}