#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetRasterizer.h"
#include "Mesh/Editor/MeshBrowsePreview.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MeshBrowsePreview", 0, MeshBrowsePreview, editor::IBrowsePreview)

TypeInfoSet MeshBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< MeshAsset >();
}

Ref< ui::Bitmap > MeshBrowsePreview::generate(const editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const MeshAsset > asset = instance->getObject< MeshAsset >();
	if (!asset)
		return nullptr;

	Ref< drawing::Image > meshThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		ui::dpi96(128),
		ui::dpi96(128)
	);
	meshThumb->clear(Color4f(0.6f, 0.6f, 0.6f, 1.0f));

	MeshAssetRasterizer().generate(editor, asset, meshThumb);

	drawing::ScaleFilter scaleFilter(
		ui::dpi96(64),
		ui::dpi96(64),
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgLinear
	);
	meshThumb->apply(&scaleFilter);

	return new ui::Bitmap(meshThumb);
}

	}
}
