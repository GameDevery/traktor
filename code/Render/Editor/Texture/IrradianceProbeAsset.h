#pragma once

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS IrradianceProbeAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	IrradianceProbeAsset();

	float getFactor() const { return m_factor; }

	virtual void serialize(ISerializer& s) override final;

private:
	float m_factor;
};

	}
}
