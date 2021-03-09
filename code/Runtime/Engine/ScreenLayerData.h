#pragma once

#include "Core/Math/Aabb2.h"
#include "Resource/Id.h"
#include "Runtime/Engine/LayerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace runtime
	{

/*!
 * \ingroup Runtime
 */
class T_DLLCLASS ScreenLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StagePipeline;

	resource::Id< render::Shader > m_shader;
};

	}
}
