#include "Core/Math/Const.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Model/Model.h"

namespace traktor
{
    namespace hf
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.ConvertHeightfield", ConvertHeightfield, Object)

Ref< model::Model > ConvertHeightfield::convert(const Heightfield* heightfield, int32_t step, float vistaDistance) const
{
	int32_t size = heightfield->getSize();

    int32_t ix0, iz0;
    int32_t ix1, iz1;

    if (vistaDistance > FUZZY_EPSILON)
    {
        heightfield->worldToGrid(-vistaDistance / 2.0f, -vistaDistance / 2.0f, ix0, iz0);
        heightfield->worldToGrid( vistaDistance / 2.0f,  vistaDistance / 2.0f, ix1, iz1);

        ix0 = clamp(ix0, 0, size);
        iz0 = clamp(iz0, 0, size);
        ix1 = clamp(ix1, 0, size);
        iz1 = clamp(iz1, 0, size);
    }
    else
    {
        ix0 = 0;
        iz0 = 0;
        ix1 = size;
        iz1 = size;
    }

    size = max(ix1 - ix0, iz1 - iz0);

	int32_t outputSize = size / step;

	Ref< model::Model > model = new model::Model();

    // Add texcoord channels.
    uint32_t baseChannel = model->addUniqueTexCoordChannel(L"Base");
    uint32_t lightmapChannel = model->addUniqueTexCoordChannel(L"Lightmap");

    // Add single material for entire heightfield.
    model::Material material;
    material.setName(L"Heightfield");
    model->addMaterial(material);

    // Convert vertices.
	model->reservePositions(outputSize * outputSize);
    model->reserveVertices(outputSize * outputSize);

	model::Vertex vertex;
    for (int32_t iz = iz0; iz < iz1; iz += step)
    {
        for (int32_t ix = ix0; ix < ix1; ix += step)
        {
			float wx, wz;
			heightfield->gridToWorld(ix, iz, wx, wz);

			uint32_t positionId = model->addPosition(Vector4(
				wx,
				heightfield->getWorldHeight(wx, wz),
				wz,
				1.0f
			));

			uint32_t texCoordId = model->addTexCoord(Vector2(
				float(ix) / (size - 1),
				float(iz) / (size - 1)
			));

			vertex.setPosition(positionId);
			vertex.setTexCoord(baseChannel, texCoordId);
            vertex.setTexCoord(lightmapChannel, texCoordId);

			model->addVertex(vertex);
		}
	}

    // Convert polygons.
	model::Polygon polygon;
	for (int32_t iz = 0; iz < outputSize - 1; ++iz)
	{
		int32_t offset = iz * outputSize;
		for (int32_t ix = 0; ix < outputSize - 1; ++ix)
		{
			float wx, wz;
            heightfield->gridToWorld(ix0 + ix * step, iz0 + iz * step, wx, wz);

			if (!heightfield->getWorldCut(wx, wz))
				continue;
			if (!heightfield->getWorldCut(wx + step, wz))
				continue;
			if (!heightfield->getWorldCut(wx + step, wz + step))
				continue;
			if (!heightfield->getWorldCut(wx, wz + step))
				continue;

			int32_t indices[] =
			{
				offset + ix,
				offset + ix + 1,
				offset + ix + 1 + outputSize,
				offset + ix + outputSize
			};

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
	}

    return model;
}

    }
}