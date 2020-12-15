#pragma once

#include <map>
#include <set>
#include "Core/Object.h"

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

class ShaderGraph;
class Node;

/*! Shader graph technique generator.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphTechniques : public Object
{
	T_RTTI_CLASS;

public:
	ShaderGraphTechniques(const ShaderGraph* shaderGraph);

	std::set< std::wstring > getNames() const;

	ShaderGraph* generate(const std::wstring& name) const;

private:
	std::map< std::wstring, Ref< ShaderGraph > > m_techniques;
};

	}
}

