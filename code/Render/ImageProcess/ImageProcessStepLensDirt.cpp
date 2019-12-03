#include "Core/Math/Random.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessStepLensDirt.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct Vertex
{
	float position[2];
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepLensDirt", 1, ImageProcessStepLensDirt, ImageProcessStep)

ImageProcessStepLensDirt::ImageProcessStepLensDirt()
{
}

Ref< ImageProcessStep::Instance > ImageProcessStepLensDirt::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	Ref< InstanceLensDirt > instance = new InstanceLensDirt(this);

	if (!resourceManager->bind(m_shader, instance->m_shader))
		return 0;

	instance->m_sources.resize(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		instance->m_sources[i].param = getParameterHandle(m_sources[i].param);
		instance->m_sources[i].source = getParameterHandle(m_sources[i].source);
	}

	AlignedVector< VertexElement > vertexElements;
	vertexElements.push_back(VertexElement(DuPosition, DtFloat2, offsetof(Vertex, position), 0));
	T_ASSERT_M (getVertexSize(vertexElements) == sizeof(Vertex), L"Incorrect size of vertex");

	instance->m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 4 * sizeof(Vertex), false);

	Vertex* vertex = static_cast< Vertex* >(instance->m_vertexBuffer->lock());
	T_ASSERT(vertex);

	vertex->position[0] = -1.0f; vertex->position[1] = -1.0f; ++vertex;
	vertex->position[0] = -1.0f; vertex->position[1] =  1.0f; ++vertex;
	vertex->position[0] =  1.0f; vertex->position[1] = -1.0f; ++vertex;
	vertex->position[0] =  1.0f; vertex->position[1] =  1.0f; ++vertex;

	instance->m_vertexBuffer->unlock();

	instance->m_primitives.setNonIndexed(PtTriangleStrip, 0, 2);

	Random random;
	for (uint32_t i = 0; i < InstanceLensDirt::InstanceCount; ++i)
	{
		float x = random.nextFloat() * 2.0f - 1.0f;
		float y = random.nextFloat() * 2.0f - 1.0f;
		float s = traktor::sqrtf(x * x + y * y) / 1.414214f;
		float w = random.nextFloat() * (s * 0.8f) + 0.2f;
		instance->m_instances[i] = Vector4(
			x,
			y,
			random.nextFloat() * TWO_PI,
			w * w * 1.3f
		);
	}

	return instance;
}

void ImageProcessStepLensDirt::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

ImageProcessStepLensDirt::Source::Source()
{
}

void ImageProcessStepLensDirt::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);

	if (s.getVersion() < 1)
	{
		uint32_t index = 0;
		s >> Member< uint32_t >(L"index", index);
		T_FATAL_ASSERT_M (index == 0, L"Index must be zero, update binding");
	}
}

// Instance

ImageProcessStepLensDirt::InstanceLensDirt::InstanceLensDirt(const ImageProcessStepLensDirt* step)
:	m_step(step)
{
	m_handleInstances = getParameterHandle(L"Instances");
}

void ImageProcessStepLensDirt::InstanceLensDirt::destroy()
{
	safeDestroy(m_vertexBuffer);
}

void ImageProcessStepLensDirt::InstanceLensDirt::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	imageProcess->prepareShader(m_shader);

	m_shader->setVectorArrayParameter(m_handleInstances, m_instances, InstanceCount);

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		ISimpleTexture* source = imageProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source);
	}

	m_shader->draw(
		renderView,
		m_vertexBuffer,
		0,
		m_primitives,
		InstanceCount
	);
}

	}
}
