#pragma once

#include "Core/Object.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class AccShapeResources : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager);

	void destroy();

private:
	friend class AccDisplayRenderer;
	friend class AccQuad;
	friend class AccShape;

	resource::Proxy< render::Shader > m_shaderSolid;
	resource::Proxy< render::Shader > m_shaderTextured;
	resource::Proxy< render::Shader > m_shaderLine;
	resource::Proxy< render::Shader > m_shaderIncrementMask;
	resource::Proxy< render::Shader > m_shaderDecrementMask;
};

	}
}

