#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Plane.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IndexBuffer;
class IRenderSystem;
class RenderContext;
class Shader;
class VertexBuffer;

	}

	namespace world
	{

class IWorldRenderPass;

	}

	namespace spray
	{

struct TrailVertex;

/*! Ribbon trail renderer.
 * \ingroup Spray
 */
class T_DLLCLASS TrailRenderer : public Object
{
	T_RTTI_CLASS;

public:
	explicit TrailRenderer(render::IRenderSystem* renderSystem);

	virtual ~TrailRenderer();

	void destroy();

	void render(
		render::Shader* shader,
		const CircularVector< Vector4, 64 >& points,
		const Vector4& cameraPosition,
		const Plane& cameraPlane,
		float width,
		float time,
		float age
	);

	void flush(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	struct Batch
	{
		render::Shader* shader;
		uint32_t points;
		Vector4 timeAndAge;
	};

	Ref< render::VertexBuffer > m_vertexBuffers[4];
	Ref< render::IndexBuffer > m_indexBuffer;
	uint32_t m_count;
	TrailVertex* m_vertex;
	AlignedVector< Batch > m_batches;
};

	}
}

