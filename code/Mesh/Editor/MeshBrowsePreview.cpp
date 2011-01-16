#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Raster.h"
#include "Editor/IEditor.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshBrowsePreview.h"
#include "Model/Model.h"
#include "Model/Utilities.h"
#include "Model/Formats/ModelFormat.h"
#include "Ui/Bitmap.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MeshBrowsePreview", 0, MeshBrowsePreview, editor::IBrowsePreview)

TypeInfoSet MeshBrowsePreview::getPreviewTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshAsset >());
	return typeSet;
}

Ref< ui::Bitmap > MeshBrowsePreview::generate(const editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const MeshAsset > meshAsset = instance->getObject< MeshAsset >();
	if (!meshAsset)
		return 0;

	std::wstring assetPath = editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, meshAsset->getFileName());

	Ref< model::Model > model = model::ModelFormat::readAny(fileName);
	if (!model)
		return 0;

	Aabb3 boundingBox = model::calculateModelBoundingBox(*model);

	Ref< drawing::Image > meshThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8(),
		64,
		64
	);

	meshThumb->clear(Color4f(0.6f, 0.6f, 0.6f));

	drawing::Raster raster(meshThumb);

	const std::vector< model::Polygon >& polygons = model->getPolygons();
	const std::vector< model::Vertex >& vertices = model->getVertices();
	const AlignedVector< Vector4 >& positions = model->getPositions();
	const AlignedVector< Vector4 >& normals = model->getNormals();

	AlignedVector< Vector2 > screenVertices;

	for (std::vector< model::Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		const std::vector< uint32_t >& polygonVertices = i->getVertices();
		if (polygonVertices.empty())
			continue;

		screenVertices.resize(polygonVertices.size());
		for (uint32_t j = 0; j < polygonVertices.size(); ++j)
		{
			const model::Vertex& vertex = vertices[polygonVertices[j]];
			Vector4 position = positions[vertex.getPosition()];

			position = (position - boundingBox.getCenter()) / boundingBox.getExtent();

			float iz = 1.0f / (position.z() * 0.5f + 1.5f);

			screenVertices[j].x = position.x() * iz * 28.0f + 32.0f;
			screenVertices[j].y = 32.0f - position.y() * iz * 28.0f;
		}

		float shade = 1.0f;
		if (i->getNormal() != model::c_InvalidIndex)
			shade = abs(normals[i->getNormal()].z() * 0.3f) + 0.7f;

		raster.drawPolyLine(
			&screenVertices[0],
			screenVertices.size(),
			Color4f(shade, shade, shade, 1.0f)
		);
	}

	return new ui::Bitmap(meshThumb);
}

	}
}
