#pragma once

#include "Core/Object.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace spray
	{

class TrailInstance;

/*! Ribbon trail.
 * \ingroup Spray
 */
class Trail : public Object
{
	T_RTTI_CLASS;

public:
	explicit Trail(
		const resource::Proxy< render::Shader >& shader,
		float width,
		float age,
		float lengthThreshold,
		float breakThreshold
	);

	Ref< TrailInstance > createInstance() const;

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Proxy< render::Shader > m_shader;
	float m_width;
	float m_age;
	float m_lengthThreshold;
	float m_breakThreshold;
};

	}
}

