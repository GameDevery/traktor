#include "Render/Image/Defines/ImageProcessDefineTexture.h"
#include "Render/Editor/Image/ImageProcessDefineTextureFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessDefineTextureFacade", ImageProcessDefineTextureFacade, IImageProcessDefineFacade)

int32_t ImageProcessDefineTextureFacade::getImage(editor::IEditor* editor, const ImageProcessDefine* definition) const
{
	return 0;
}

std::wstring ImageProcessDefineTextureFacade::getText(editor::IEditor* editor, const ImageProcessDefine* definition) const
{
	const ImageProcessDefineTexture* texture = mandatory_non_null_type_cast< const ImageProcessDefineTexture* >(definition);
	return L"-- Texture --";
}

	}
}
