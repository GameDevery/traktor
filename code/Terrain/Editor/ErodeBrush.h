/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_terrain_ErodeBrush_H
#define traktor_terrain_ErodeBrush_H

#include "Core/Math/Vector2.h"
#include "Core/Misc/AutoPtr.h"
#include "Resource/Proxy.h"
#include "Terrain/Editor/IBrush.h"

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace terrain
	{

class ErodeBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	ErodeBrush(const resource::Proxy< hf::Heightfield >& heightfield);

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material) override final;

	virtual void apply(int32_t x, int32_t y) override final;

	virtual void end(int32_t x, int32_t y) override final;

	virtual Ref< IBrush > clone() const override final;

	virtual bool contained() const override final { return false; }

private:
	struct WaterCell
	{
		float level;
		Vector2 velocity;

		WaterCell()
		:	level(0.0f)
		,	velocity(0.0f, 0.0f)
		{
		}
	};

	resource::Proxy< hf::Heightfield > m_heightfield;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
	AutoArrayPtr< WaterCell > m_water;
};

	}
}

#endif	// traktor_terrain_ErodeBrush_H
