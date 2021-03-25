#pragma once

#include "Render/Resource/ShaderResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

/*! Bind textures to shaders.
 * \ingroup Render
 */
class T_DLLCLASS TextureLinker : public Object
{
	T_RTTI_CLASS;

public:
	struct TextureReader
	{
		virtual ~TextureReader() {}

		virtual Ref< ITexture > read(const Guid& textureGuid) const = 0;
	};

	TextureLinker(const TextureReader& textureReader);

	bool link(const ShaderResource::Combination& shaderCombination, IProgram* program) const;

private:
	const TextureReader& m_textureReader;
};

	}
}

