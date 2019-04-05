#include "Render/IProgram.h"
#include "Render/Resource/TextureLinker.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureLinker", TextureLinker, Object)

TextureLinker::TextureLinker(TextureReader& textureReader)
:	m_textureReader(textureReader)
{
}

bool TextureLinker::link(const ShaderResource::Combination& shaderCombination, IProgram* program)
{
	const auto& textures = shaderCombination.textures;
	for (uint32_t i = 0; i < textures.size(); ++i)
	{
		Ref< ITexture > texture = m_textureReader.read(textures[i]);
		if (!texture)
			return false;

		handle_t parameterHandle = getParameterHandleFromTextureReferenceIndex(i);
		program->setTextureParameter(parameterHandle, texture);
	}
	return true;
}

	}
}
