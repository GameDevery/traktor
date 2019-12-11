#pragma once

#include <set>
#include <map>
#include "Core/Object.h"
#include "Render/Editor/Shader/PinType.h"

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

class Node;
class OutputPin;
class ShaderGraph;

/*! Shader graph optimizer.
 * \ingroup Render
 *
 * \note The shader graph optimizer only works on
 *       fully resolved shader graphs, i.e. no fragments.
 */
class T_DLLCLASS ShaderGraphOptimizer : public Object
{
	T_RTTI_CLASS;

public:
	ShaderGraphOptimizer(const ShaderGraph* shaderGraph);

	/*! Remove unused branches.
	 *
	 * \return Shader graph with removed unused branches.
	 */
	Ref< ShaderGraph > removeUnusedBranches() const;

	/*! Merge duplicated branches.
	 *
	 * \return Shader graph with removed duplicated branches.
	 */
	Ref< ShaderGraph > mergeBranches() const;

	/*! Insert interpolator nodes where appropriate.
	 *
	 * Using a order analysis to determine where to
	 * best insert interpolators into the graph
	 * in order to split calculations into vertex and pixel
	 * shaders.
	 *
	 * \param frequentUniformsAsLinear Treat uniforms with high update frequency with linear complexity.
	 * \return Shader graph with inserted interpolators.
	 */
	Ref< ShaderGraph > insertInterpolators(bool frequentUniformsAsLinear) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	mutable std::set< const Node* > m_visited;
	mutable int32_t m_insertedCount;
	mutable bool m_frequentUniformsAsLinear;

	void insertInterpolators(ShaderGraph* shaderGraph, Node* node) const;
};

	}
}

