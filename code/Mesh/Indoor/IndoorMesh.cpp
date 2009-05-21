#include <algorithm>
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Context/RenderContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

render::handle_t s_handleUserParameter = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.IndoorMesh", IndoorMesh, Object)

IndoorMesh::IndoorMesh()
{
	if (!s_handleUserParameter)
		s_handleUserParameter = render::getParameterHandle(L"UserParameter");
}

const Aabb& IndoorMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void IndoorMesh::render(
	render::RenderContext* renderContext,
	const world::WorldRenderView* worldRenderView,
	const Matrix44& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	Vector4 cameraPosition = worldRenderView->getView().inverse().translation();
	
	// Create initial clipper frustum, clipper frustums can have more than
	// 6 planes as it's later reconstructed from clipped portals.
	AlignedVector< Plane > frustum(6);
	for (int i = 0; i < 6; ++i)
		frustum[i] = worldRenderView->getCullFrustum().pl[i];

	// Find initially active sectors which are the sectors that the camera is within,
	// as sector bounding boxes are lously calculated more than one sector
	// can become initially active.
	std::set< int > activeSectors;

	static bool searchActiveSectors = true;
	if (searchActiveSectors)
	{
		for (int i = 0; i < int(m_sectors.size()); ++i)
		{
			const Sector& sector = m_sectors[i];
			if (sector.boundingBox.inside(cameraPosition))
				activeSectors.insert(i);
		}
		if (!activeSectors.empty())
		{
			std::set< int > visibleSectors;
			for (std::set< int >::iterator i = activeSectors.begin(); i != activeSectors.end(); ++i)
				findVisibleSectors(
					frustum,
					worldRenderView->getView(),
					*i,
					visibleSectors
				);

			activeSectors.insert(visibleSectors.begin(), visibleSectors.end());
		}
	}
	if (activeSectors.empty())
	{
		for (int i = 0; i < int(m_sectors.size()); ++i)
			activeSectors.insert(i);
	}

	// Render sectors, should probarly sort all visible parts by their shader as
	// it will otherwise be alot of state changes.

	const std::vector< render::Mesh::Part >& meshParts = m_mesh->getParts();

	for (std::set< int >::iterator i = activeSectors.begin(); i != activeSectors.end(); ++i)
	{
		Sector& sector = m_sectors[*i];
		for (std::vector< Part >::iterator j = sector.parts.begin(); j != sector.parts.end(); ++j)
		{
			resource::Proxy< render::Shader >& material = j->material;
			if (!material.validate())
				continue;

			render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

			renderBlock->type = render::RbtOpaque;
			renderBlock->distance = distance;
			renderBlock->shader = material;
			renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
			renderBlock->indexBuffer = m_mesh->getIndexBuffer();
			renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
			renderBlock->primitives = &meshParts[j->meshPart].primitives;

			renderBlock->shaderParams->beginParameters(renderContext);
			if (parameterCallback)
				parameterCallback->setParameters(renderBlock->shaderParams);
			worldRenderView->setShaderParameters(renderBlock->shaderParams, worldTransform, getBoundingBox());
			renderBlock->shaderParams->endParameters(renderContext);

			renderContext->draw(renderBlock);
		}
	}
}

void IndoorMesh::findVisibleSectors(
	const AlignedVector< Plane >& frustum,
	const Matrix44& view,
	int currentSector,
	std::set< int >& outVisibleSectors
)
{
	for (AlignedVector< Portal >::const_iterator i = m_portals.begin(); i != m_portals.end(); ++i)
	{
		if (i->sectorA == currentSector || i->sectorB == currentSector)
		{
			int nextSector = (i->sectorA == currentSector) ? i->sectorB : i->sectorA;
			if (outVisibleSectors.find(nextSector) != outVisibleSectors.end())
				continue;

			Winding clipped;
			for (AlignedVector< Vector4 >::const_iterator j = i->winding.points.begin(); j != i->winding.points.end(); ++j)
				clipped.points.push_back(
					view * *j
				);

			Plane clippedPlane;
			if (clipped.getPlane(clippedPlane) && clippedPlane.normal().z() >= 0.0f)
				std::reverse(clipped.points.begin(), clipped.points.end());

			for (AlignedVector< Plane >::const_iterator j = frustum.begin(); j != frustum.end(); ++j)
			{
				Winding front, back;
				clipped.split(*j, front, back);
				clipped = front;

				if (clipped.points.size() < 3)
					break;
			}

			if (clipped.points.size() >= 3)
			{
				outVisibleSectors.insert(nextSector);

				AlignedVector< Plane > nextFrustum(clipped.points.size());
				for (size_t i = 0, j = clipped.points.size() - 1; i < clipped.points.size(); j = i++)
				{
					nextFrustum[i] = Plane(
						Vector4(0.0f, 0.0f, 0.0f, 1.0f),
						clipped.points[i],
						clipped.points[j]
					);
				}

				findVisibleSectors(nextFrustum, view, nextSector, outVisibleSectors);
			}
		}
	}
}

	}
}
