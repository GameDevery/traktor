#pragma once

#include <vector>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ShaderGraph;

/*! Shader graph combination generator.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphCombinations : public Object
{
	T_RTTI_CLASS;

public:
	struct Combination
	{
		uint32_t mask;
		uint32_t value;
		Ref< const ShaderGraph > shaderGraph;
	};

	ShaderGraphCombinations(const ShaderGraph* shaderGraph);

	const std::vector< std::wstring >& getParameterNames() const;

	std::vector< std::wstring > getParameterNames(uint32_t mask) const;

	uint32_t getCombinationCount() const;

	uint32_t getCombinationMask(uint32_t index) const;

	uint32_t getCombinationValue(uint32_t index) const;

	Ref< const ShaderGraph > getCombinationShaderGraph(uint32_t index) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	std::vector< std::wstring > m_parameterNames;
	std::vector< Combination > m_combinations;
};

	}
}

