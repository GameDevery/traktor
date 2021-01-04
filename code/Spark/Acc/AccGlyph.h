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

class IndexBuffer;
class IRenderSystem;
class ITexture;
class RenderPass;
class Shader;
class VertexBuffer;

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
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	uint8_t* m_vertex;
	uint32_t m_offset;
	uint32_t m_count;
};

	}
}

