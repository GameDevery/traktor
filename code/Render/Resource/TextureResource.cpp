#include "Render/Resource/TextureResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureResource", 0, TextureResource, ISerializable)

bool TextureResource::serialize(ISerializer& s)
{
	return true;
}

	}
}
