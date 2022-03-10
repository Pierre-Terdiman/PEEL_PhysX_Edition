#include "stdafx.h"

// Implementation of Andrew's monotone chain 2D convex hull algorithm.
// Asymptotic complexity: O(n log n).
// Practical performance: 0.5-1.0 seconds for n=1000000 on a 1GHz machine.
#include <algorithm>
#include <vector>
using namespace std;

typedef double coord_t;         // coordinate type
typedef double coord2_t;  // must be big enough to hold 2*max(|coordinate|)^2

namespace
{
struct Point_ {
	coord_t x, y;

	bool operator <(const Point_ &p) const {
		return x < p.x || (x == p.x && y < p.y);
	}
};
}
// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
// Returns a positive value, if OAB makes a counter-clockwise turn,
// negative for clockwise turn, and zero if the points are collinear.
static coord2_t cross(const Point_ &O, const Point_ &A, const Point_ &B)
{
	return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Returns a list of points on the convex hull in counter-clockwise order.
// Note: the last point in the returned list is the same as the first one.
static vector<Point_> convex_hull(vector<Point_> P)
{
	int n = int(P.size()), k = 0;
	vector<Point_> H(2*n);

	// Sort points lexicographically
	sort(P.begin(), P.end());

	// Build lower hull
	for (int i = 0; i < n; ++i) {
		while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
		H[k++] = P[i];
	}

	// Build upper hull
	for (int i = n-2, t = k+1; i >= 0; i--) {
		while (k >= t && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
		H[k++] = P[i];
	}

	H.resize(k-1);
	return H;
}

void CreateConvexHull2D(Vertices& verts)
{
	vector<Point_> in;

	for(udword i=0;i<verts.GetNbVertices();i++)
	{
		Point_ p;
		p.x = verts.GetVertices()[i].x;
		p.y = verts.GetVertices()[i].y;
		in.push_back(p);
	}

	vector<Point_> out = convex_hull(in);

	verts.Reset();
	for(udword i=0;i<out.size();i++)
	{
		const Point_ p = out[i];
		verts.AddVertex(Point(float(p.x), float(p.y), 0.0f));
	}
}