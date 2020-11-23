#include "Core/Serialization/DeepClone.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetEditor.h"
#include "Mesh/Editor/MeshAssetEditorFactory.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshAssetEditorFactory", 0, MeshAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet MeshAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< MeshAsset >();
	return typeSet;
}

bool MeshAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > MeshAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new MeshAssetEditor(editor);
}

void MeshAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > MeshAssetEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
