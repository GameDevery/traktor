/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*!
 * \ingroup World
 */
class T_DLLCLASS VolumeComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const Vector4& getSize() const { return m_size; }

	const Aabb3 getBoundingBox() const { return Aabb3(-m_size / 2.0_simd, m_size / 2.0_simd); }

private:
	Vector4 m_size = Vector4::zero();
};

}
