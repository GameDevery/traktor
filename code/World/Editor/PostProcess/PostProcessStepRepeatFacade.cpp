#include "World/PostProcess/PostProcessStepRepeat.h"
#include "World/Editor/PostProcess/PostProcessStepRepeatFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepRepeatFacade", PostProcessStepRepeatFacade, IPostProcessStepFacade)

int32_t PostProcessStepRepeatFacade::getImage(const PostProcessStep* step) const
{
	return 3;
}

std::wstring PostProcessStepRepeatFacade::getText(const PostProcessStep* step) const
{
	return L"Repeat";
}

bool PostProcessStepRepeatFacade::canHaveChildren() const
{
	return true;
}

bool PostProcessStepRepeatFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepRepeatFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepRepeatFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	const PostProcessStepRepeat* repeatStep = checked_type_cast< const PostProcessStepRepeat* >(step);
	outChildren.push_back(repeatStep->getStep());
	return true;
}

	}
}
