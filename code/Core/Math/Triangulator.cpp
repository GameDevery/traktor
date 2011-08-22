#include <algorithm>
#include <cmath>
#include "Core/Log/Log.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace
	{

void getAdjacent(const AlignedVector< Vector2 >& points, int32_t index, int32_t& prev, int32_t& next)
{
	prev = index - 1;
	if (prev < 0)
		prev = int32_t(points.size() - 1);

	next = index + 1;
	if (next >= int32_t(points.size()))
		next = 0;
}

float triangleArea(
	const Vector2& a,
	const Vector2& b,
	const Vector2& c
)
{
	return
		a.x * (c.y - b.y) +
		b.x * (a.y - c.y) +
		c.x * (b.y - a.y);
}

bool pointInTriangle(
	const Vector2& p,
	const Vector2& a,
	const Vector2& b,
	const Vector2& c
)
{
	float a1 = triangleArea(a, b, p);
	float a2 = triangleArea(b, c, p);
	float a3 = triangleArea(c, a, p);
	return (a1 > 0 && a2 > 0 && a3 > 0) || (a1 < 0 && a2 < 0 && a3 < 0);
}

bool isConvex(const AlignedVector< Vector2 >& points, int32_t index)
{
	int32_t prev, next;
	getAdjacent(points, index, prev, next);

	const Vector2& p = points[prev];
	const Vector2& c = points[index];
	const Vector2& n = points[next];

	return bool(triangleArea(p, c, n) < 0.0f);
}

struct SortInternalAngle
{
	SortInternalAngle(const AlignedVector< Vector2 >& points)
	:	m_points(points)
	{
	}

	bool operator () (int32_t a, int32_t b)
	{
		int32_t pa, na, pb, nb;
		
		getAdjacent(m_points, a, pa, na);
		getAdjacent(m_points, b, pb, nb);

		float aa1 = acosf(dot(m_points[pa] - m_points[a], m_points[na] - m_points[a]));
		float aa2 = acosf(dot(m_points[pa] - m_points[a], m_points[na] - m_points[a]));
		float aa3 = acosf(dot(m_points[pa] - m_points[a], m_points[na] - m_points[a]));

		float aa = abs(aa1 - PI / 3) + abs(aa2 - PI / 3) + abs(aa3 - PI / 3);

		float ab1 = acosf(dot(m_points[pb] - m_points[b], m_points[nb] - m_points[b]));
		float ab2 = acosf(dot(m_points[pb] - m_points[b], m_points[nb] - m_points[b]));
		float ab3 = acosf(dot(m_points[pb] - m_points[b], m_points[nb] - m_points[b]));

		float ab = abs(ab1 - PI / 3) + abs(ab2 - PI / 3) + abs(ab3 - PI / 3);

		return aa < ab;
	}

	const AlignedVector< Vector2 >& m_points;
};

bool polygonClockwise(const AlignedVector< Vector2 >& points)
{
	float convexSum = 0.0f;
	for (size_t i = 0; i < points.size(); ++i)
	{
		const Vector2& c  = points[i];
		const Vector2& n  = points[(i + 1) % points.size()];
		const Vector2& nn = points[(i + 2) % points.size()];
	
		float aa = (nn.x - c.x) * (nn.x - c.x) + (-nn.y + c.y) * (-nn.y + c.y);
		float bb = ( n.x - c.x) * ( n.x - c.x) + ( -n.y + c.y) * ( -n.y + c.y);
		float cc = (nn.x - n.x) * (nn.x - n.x) + (-nn.y + n.y) * (-nn.y + n.y);

		T_ASSERT (bb >= 0.0f);
		T_ASSERT (cc >= 0.0f);
		
		float B = sqrtf(bb);
		float C = sqrtf(cc);
		float theta = acosf((bb + cc - aa) / (2.0f * B * C));
		
		if (triangleArea(c, n, nn) < 0.0f)
			convexSum += PI - theta;
		else
			convexSum -= PI - theta;
	}
	return bool(convexSum >= 2.0f * PI);
}

	}

void Triangulator::freeze(
	const AlignedVector< Vector2 >& points,
	std::vector< Triangle >& outTriangles,
	uint32_t flags
)
{
	bool cw = true;
	if (flags & TsCheckWinding)
		cw = polygonClockwise(points);
	
	AlignedVector< Vector2 > uncut = points;
	std::vector< int32_t > indices(uncut.size());
	for (int32_t i = 0; i < int(uncut.size()); ++i)
		indices[i] = i;

	std::vector< int32_t > ears;
	while (uncut.size() >= 3)
	{
		ears.resize(0);

		// Collect polygon ears.
		for (int32_t i = 0; i < int32_t(uncut.size()); ++i)
		{
			bool convex = isConvex(uncut, i) ^ !cw;
			if (!convex)
				continue;

			int32_t prev, next;
			getAdjacent(uncut, i, prev, next);

			bool ear = true;
			for (int j = 0; j < int32_t(uncut.size()) && ear; ++j)
			{
				if (j == prev || j == i || j == next)
					continue;

				bool convex = isConvex(uncut, j) ^ !cw;
				if (convex)
					continue;

				if (pointInTriangle(uncut[j], uncut[prev], uncut[i], uncut[next]))
					ear = false;
			}
			if (ear)
			{
				ears.push_back(i);
				if (!(flags & TfSorted))
					break;
			}
		}

		if (ears.empty())
			break;

		// Sort ears by minimize internal angle differences.
		if (flags & TfSorted)
			std::sort(ears.begin(), ears.end(), SortInternalAngle(uncut));
		int32_t cut = ears.front();

		// Cut the ear.
		int32_t prev, next;
		getAdjacent(uncut, cut, prev, next);

		Triangle t;
		t.indices[0] = indices[prev];
		t.indices[1] = indices[cut];
		t.indices[2] = indices[next];
		outTriangles.push_back(t);

		uncut.erase(uncut.begin() + cut);
		indices.erase(indices.begin() + cut);
	}
}

void Triangulator::freeze(
	const AlignedVector< Vector4 >& points,
	const Vector4& normal,
	std::vector< Triangle >& outTriangles,
	uint32_t flags
)
{
	// Use maximum axis to determine projection plane.
	Vector4 u(0.0f, 0.0f, 0.0f), v(0.0f, 0.0f, 0.0f);
	if (abs(normal.x()) > abs(normal.y()))
	{
		if (abs(normal.x()) > abs(normal.z()))	// X major
		{
			u = normal.x() > 0.0f ? Vector4(0.0f, 0.0f, 1.0f) : Vector4(0.0f, 0.0f, -1.0f);
			v = Vector4(0.0f, 1.0f, 0.0f);
		}
		else	// Z major
		{
			u = normal.z() > 0.0f ? Vector4(-1.0f, 0.0f, 0.0f) : Vector4(1.0f, 0.0f, 0.0f);
			v = Vector4(0.0f, 1.0f, 0.0f);
		}
	}
	else
	{
		if (abs(normal.y()) > abs(normal.z()))	// Y major
		{
			u = normal.y() > 0.0f ? Vector4(0.0f, 0.0f, -1.0f) : Vector4(0.0f, 0.0f, 1.0f);
			v = Vector4(1.0f, 0.0f, 0.0f);
		}
		else	// Z major
		{
			u = normal.z() > 0.0f ? Vector4(-1.0f, 0.0f, 0.0f) : Vector4(1.0f, 0.0f, 0.0f);
			v = Vector4(0.0f, 1.0f, 0.0f);
		}
	}

	// Project all points onto 2d plane.
	AlignedVector< Vector2 > projected(points.size());
	for (size_t i = 0; i < points.size(); ++i)
	{
		projected[i].x = dot3(u, points[i]);
		projected[i].y = dot3(v, points[i]);
	}

	// Freeze 2d polygon.
	freeze(projected, outTriangles, flags);
}

}
