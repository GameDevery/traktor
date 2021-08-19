#pragma once

#include "Core/Object.h"
#include "Core/Math/Matrix33.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class Buffer;
class IRenderSystem;
class ITexture;
class IVertexLayout;
class RenderPass;
class Shader;

	}

	namespace spark
	{

class AccGlyph : public Object
{
	T_RTTI_CLASS;

public:
	AccGlyph();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

	void beginFrame();

	void endFrame();

	void add(
		const Aabb2& bounds,
		const Matrix33& transform,
		const Vector4& textureOffset
	);

	void render(
		render::RenderPass* renderPass,
		const Vector4& frameSize,
		const Vector4& frameTransform,
		render::ITexture* texture,
		uint8_t maskReference,
		uint8_t glyphFilter,
		const Color4f& glyphColor,
		const Color4f& glyphFilterColor
	);

private:
	resource::Proxy< render::Shader > m_shaderGlyph;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	uint8_t* m_vertex;
	uint32_t m_offset;
	uint32_t m_count;
};

	}
}

