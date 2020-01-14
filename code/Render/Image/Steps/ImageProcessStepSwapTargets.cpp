#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/Steps/ImageProcessStepSwapTargets.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSwapTargets", 0, ImageProcessStepSwapTargets, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepSwapTargets::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	return new InstanceSwapTargets(
		getParameterHandle(m_destination),
		getParameterHandle(m_source)
	);
}

void ImageProcessStepSwapTargets::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"destination", m_destination);
	s >> Member< std::wstring >(L"source", m_source);
}

// Instance

ImageProcessStepSwapTargets::InstanceSwapTargets::InstanceSwapTargets(handle_t destination, handle_t source)
:	m_destination(destination)
,	m_source(source)
{
}

void ImageProcessStepSwapTargets::InstanceSwapTargets::destroy()
{
}

void ImageProcessStepSwapTargets::InstanceSwapTargets::build(
	ImageProcess* imageProcess,
	RenderContext* renderContext,
	ProgramParameters* sharedParams,
	const RenderParams& params
)
{
	imageProcess->swapTargets(m_destination, m_source);
}

	}
}
