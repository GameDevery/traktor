#include "Render/Image/Steps/ImageProcessStepRepeat.h"
#include "Render/Editor/Image/ImageProcessStepRepeatFacade.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepRepeatFacade", 0, ImageProcessStepRepeatFacade, IImageProcessStepFacade)

int32_t ImageProcessStepRepeatFacade::getImage(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return 3;
}

std::wstring ImageProcessStepRepeatFacade::getText(editor::IEditor* editor, const ImageProcessStep* step) const
{
	return L"Repeat";
}

void ImageProcessStepRepeatFacade::getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const
{
}

bool ImageProcessStepRepeatFacade::canHaveChildren() const
{
	return true;
}

bool ImageProcessStepRepeatFacade::addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepRepeatFacade::removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const
{
	return false;
}

bool ImageProcessStepRepeatFacade::getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const
{
	const ImageProcessStepRepeat* repeatStep = checked_type_cast< const ImageProcessStepRepeat* >(step);
	outChildren.push_back(repeatStep->getStep());
	return true;
}

	}
}
