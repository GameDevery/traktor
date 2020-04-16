#ifndef traktor_render_ProgramResourceVrfy_H
#define traktor_render_ProgramResourceVrfy_H

#include <string>
#include "Core/Ref.h"
#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VRFY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief
 * \ingroup Render
 */
class T_DLLCLASS ProgramResourceVrfy : public ProgramResource
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override;

private:
	friend class ProgramCompilerVrfy;
	friend class RenderSystemVrfy;

	Ref< ProgramResource > m_embedded;
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::wstring m_computeShader;
};

	}
}

#endif	// traktor_render_ProgramResourceVrfy_H

