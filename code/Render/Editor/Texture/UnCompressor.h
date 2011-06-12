#ifndef traktor_render_UnCompressor_H
#define traktor_render_UnCompressor_H

#include "Render/Editor/Texture/ICompressor.h"

namespace traktor
{
	namespace render
	{

class UnCompressor : public ICompressor
{
	T_RTTI_CLASS;

public:
	virtual bool compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const;
};

	}
}

#endif	// traktor_render_UnCompressor_H
