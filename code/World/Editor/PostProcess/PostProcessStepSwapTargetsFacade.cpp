#include "World/PostProcess/PostProcessStepSwapTargets.h"
#include "World/Editor/PostProcess/PostProcessStepSwapTargetsFacade.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepSwapTargetsFacade", PostProcessStepSwapTargetsFacade, IPostProcessStepFacade)

int32_t PostProcessStepSwapTargetsFacade::getImage(const PostProcessStep* step) const
{
	return 8;
}

std::wstring PostProcessStepSwapTargetsFacade::getText(const PostProcessStep* step) const
{
	const PostProcessStepSwapTargets* swapStep = checked_type_cast< const PostProcessStepSwapTargets* >(step);
	return L"Swap targets \"" + swapStep->getDestination() + L"\" <-> \"" + swapStep->getSource() + L"\"";
}

bool PostProcessStepSwapTargetsFacade::canHaveChildren() const
{
	return false;
}

bool PostProcessStepSwapTargetsFacade::addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSwapTargetsFacade::removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const
{
	return false;
}

bool PostProcessStepSwapTargetsFacade::getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const
{
	return false;
}

	}
}
