#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

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

/*! Static analysis on shader graphs.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphStatic : public Object
{
	T_RTTI_CLASS;

public:
	enum VariableResolveType
	{
		VrtLocal,
		VrtGlobal
	};

	ShaderGraphStatic(const ShaderGraph* shaderGraph);

	/*! Get permutation of shader graph for given platform. */
	Ref< ShaderGraph > getPlatformPermutation(const std::wstring& platform) const;

	/*! Get permutation of shader graph for given renderer. */
	Ref< ShaderGraph > getRendererPermutation(const std::wstring& renderer) const;

	/*! Replace all "Connected" nodes with direct connections. */
	Ref< ShaderGraph > getConnectedPermutation() const;

	/*! Replace all "Type" nodes with direct connections based on input type. */
	Ref< ShaderGraph > getTypePermutation() const;

	/*! Insert swizzle nodes for all inputs to ensure widths are as small as possible. */
	Ref< ShaderGraph > getSwizzledPermutation() const;

	/*! Calculate constant branches and replace with simpler branches. */
	Ref< ShaderGraph > getConstantFolded() const;

	/*! Remove redundant swizzle nodes. */
	Ref< ShaderGraph > cleanupRedundantSwizzles() const;

	/*! Propagate state given as input into PixelOutput. */
	Ref< ShaderGraph > getStateResolved() const;

	/*! Replace variable nodes with direct connections. */
	Ref< ShaderGraph > getVariableResolved(VariableResolveType resolve) const;

	/*! Remove disabled outputs. */
	Ref< ShaderGraph > removeDisabledOutputs() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
};

	}
}
