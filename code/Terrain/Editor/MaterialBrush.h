/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_terrain_MaterialBrush_H
#define traktor_terrain_MaterialBrush_H

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

class MaterialBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	MaterialBrush(const resource::Proxy< hf::Heightfield >& heightfiel);

	virtual uint32_t begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material) override final;

	virtual void apply(int32_t x, int32_t y) override final;

	virtual void end(int32_t x, int32_t y) override final;

	virtual Ref< IBrush > clone() const override final;

	virtual bool contained() const override final { return true; }

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	float m_strength;
	int32_t m_material;
};

	}
}

#endif	// traktor_terrain_MaterialBrush_H
