/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Model/Model.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.ConvertHeightfield", ConvertHeightfield, Object)

Ref< model::Model > ConvertHeightfield::convert(const Heightfield* heightfield, int32_t step) const
{
	int32_t size = heightfield->getSize();

	int32_t ix0, iz0;
	int32_t ix1, iz1;

	ix0 = 0;
	iz0 = 0;
	ix1 = size;
	iz1 = size;

	size = max(ix1 - ix0, iz1 - iz0);

	const int32_t outputSize = max(size / step, 1);

	Ref< model::Model > model = new model::Model();

	// Add texcoord channels.
	const uint32_t baseChannel = model->addUniqueTexCoordChannel(L"Base");
	const uint32_t lightmapChannel = model->addUniqueTexCoordChannel(L"Lightmap");

	// Add single material for entire heightfield.
	model::Material material;
	material.setName(L"Heightfield");
	model->addMaterial(material);

	// Convert vertices.
	model->reservePositions(outputSize * outputSize);
	model->reserveNormals(outputSize * outputSize);
	model->reserveVertices(outputSize * outputSize);

	model::Vertex vertex;
	for (int32_t iz = 0; iz < outputSize; ++iz)
	{
		for (int32_t ix = 0; ix < outputSize; ++ix)
		{
			float wx, wz;
			heightfield->gridToWorld(ix0 + ix * step, iz0 + iz * step, wx, wz);

			const uint32_t positionId = model->addPosition(Vector4(
				wx,
				heightfield->getWorldHeight(wx, wz),
				wz,
				1.0f
			));

			const Vector4 normal = heightfield->normalAt(ix0 + ix * step, iz0 + iz * step);
			const uint32_t normalId = model->addNormal(normal.xyz0());

			const uint32_t texCoordId = model->addTexCoord(Vector2(
				float(ix) / (outputSize - 1),
				float(iz) / (outputSize - 1)
			));

			vertex.setPosition(positionId);
			vertex.setNormal(normalId);
			vertex.setTexCoord(baseChannel, texCoordId);
			vertex.setTexCoord(lightmapChannel, texCoordId);

			model->addVertex(vertex);
		}
	}

	// Convert polygons.
	model::Polygon polygon;
	for (int32_t iz = 0; iz < outputSize - 1; ++iz)
	{
		const int32_t offset = iz * outputSize;
		for (int32_t ix = 0; ix < outputSize - 1; ++ix)
		{
			const int32_t gx = ix0 + ix * step;
			const int32_t gz = iz0 + iz * step;

			float wx, wz;
			heightfield->gridToWorld(gx, gz, wx, wz);

			if (!heightfield->getWorldCut(wx, wz))
				continue;
			if (!heightfield->getWorldCut(wx + step, wz))
				continue;
			if (!heightfield->getWorldCut(wx + step, wz + step))
				continue;
			if (!heightfield->getWorldCut(wx, wz + step))
				continue;

			const float heights[] =
			{
				heightfield->getGridHeightNearest(gx, gz),
				heightfield->getGridHeightNearest(gx + step, gz),
				heightfield->getGridHeightNearest(gx + step, gz + step),
				heightfield->getGridHeightNearest(gx, gz + step)
			};

			const float ch = (heights[0] + heights[1] + heights[2] + heights[3]) / 4.0f;
			
			const float dh[] =
			{
				std::abs(ch - heights[0]),
				std::abs(ch - heights[1]),
				std::abs(ch - heights[2]),
				std::abs(ch - heights[3])
			};

			const auto it = std::max_element(&dh[0], &dh[4]);
			const int32_t mdhi = (int32_t)std::distance(&dh[0], it);

			const int32_t indices[] =
			{
				offset + ix,
				offset + ix + 1,
				offset + ix + 1 + outputSize,
				offset + ix + outputSize
			};

			if (mdhi == 0 || mdhi == 2)
			{
				polygon.clearVertices();
				polygon.setMaterial(0);
				polygon.addVertex(indices[0]);
				polygon.addVertex(indices[1]);
				polygon.addVertex(indices[3]);
				model->addPolygon(polygon);

				polygon.clearVertices();
				polygon.setMaterial(0);
				polygon.addVertex(indices[1]);
				polygon.addVertex(indices[2]);
				polygon.addVertex(indices[3]);
				model->addPolygon(polygon);
			}
			else
			{
				polygon.clearVertices();
				polygon.setMaterial(0);
				polygon.addVertex(indices[0]);
				polygon.addVertex(indices[1]);
				polygon.addVertex(indices[2]);
				model->addPolygon(polygon);

				polygon.clearVertices();
				polygon.setMaterial(0);
				polygon.addVertex(indices[0]);
				polygon.addVertex(indices[2]);
				polygon.addVertex(indices[3]);
				model->addPolygon(polygon);
			}
		}
	}

	return model;
}

}
