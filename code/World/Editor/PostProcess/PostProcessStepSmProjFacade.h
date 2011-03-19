#ifndef traktor_world_PostProcessStepSmProjFacade_H
#define traktor_world_PostProcessStepSmProjFacade_H

#include "World/Editor/PostProcess/IPostProcessStepFacade.h"

namespace traktor
{
	namespace world
	{

class PostProcessStepSmProjFacade : public IPostProcessStepFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const PostProcessStep* step) const;

	virtual std::wstring getText(const PostProcessStep* step) const;

	virtual bool canHaveChildren() const;

	virtual bool addChild(PostProcessStep* parentStep, PostProcessStep* childStep) const;

	virtual bool removeChild(PostProcessStep* parentStep, PostProcessStep* childStep) const;

	virtual bool getChildren(const PostProcessStep* step, RefArray< PostProcessStep >& outChildren) const;
};

	}
}

#endif	// traktor_world_PostProcessStepSmProjFacade_H

