#ifndef traktor_render_ProgramCompilerDx11_H
#define traktor_render_ProgramCompilerDx11_H

#include "Render/IProgramCompiler.h"

namespace traktor
{
	namespace render
	{

/*! \brief DX11 program compiler.
 * \ingroup Render
 */
class ProgramCompilerDx11 : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual const wchar_t* getPlatformSignature() const;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		IProgramHints* hints,
		Stats* outStats
	) const;
};

	}
}

#endif	// traktor_render_ProgramCompilerDx11_H
