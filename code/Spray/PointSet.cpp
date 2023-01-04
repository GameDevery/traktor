/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Spray/PointSet.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSet", PointSet, Object)

void PointSet::add(const Point& point)
{
	m_points.push_back(point);
}

bool PointSet::read(IStream* stream)
{
	Reader r(stream);

	uint32_t pointCount;
	r >> pointCount;

	m_points.resize(pointCount);
	for (uint32_t i = 0; i < pointCount; ++i)
	{
		float tmp[3+3+4];
		if (r.read(tmp, 3+3+4, sizeof(float)) != (3+3+4) * sizeof(float))
			return false;
		m_points[i].position = Vector4::loadUnaligned(&tmp[0]).xyz1();
		m_points[i].normal = Vector4::loadUnaligned(&tmp[3]).xyz1();
		m_points[i].color = Vector4::loadUnaligned(&tmp[6]);
	}

	return true;
}

bool PointSet::write(IStream* stream) const
{
	Writer w(stream);

	w << (uint32_t)m_points.size();

	for (uint32_t i = 0; i < m_points.size(); ++i)
	{
		float tmp[3+3+4];
		m_points[i].position.storeUnaligned(&tmp[0]);
		m_points[i].normal.storeUnaligned(&tmp[3]);
		m_points[i].color.storeUnaligned(&tmp[6]);
		if (w.write(tmp, 3+3+4, sizeof(float)) != (3+3+4) * sizeof(float))
			return false;
	}

	return true;
}

}
