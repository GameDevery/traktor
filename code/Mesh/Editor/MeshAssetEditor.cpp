#include <algorithm>
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Database/Group.h"
#include "Editor/IEditor.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetEditor.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Render/ITexture.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/DropDown.h"
#include "Ui/FileDialog.h"
#include "Ui/InputDialog.h"
#include "Ui/MessageBox.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

bool haveVertexColors(const model::Model& model)
{
	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
	{
		if (model.getVertex(i).getColor() != model::c_InvalidIndex)
			return true;
	}
	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshAssetEditor", MeshAssetEditor, editor::IObjectEditor)

MeshAssetEditor::MeshAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
	m_assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath =  m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");
}

bool MeshAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< MeshAsset* >(object);
	if (!m_asset)
		return false;

	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::Container > containerFile = new ui::Container();
	if (!containerFile->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", ui::dpi96(4), ui::dpi96(4))))
		return false;

	Ref< ui::Static > staticFileName = new ui::Static();
	if (!staticFileName->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_FILENAME")))
		return false;

	Ref< ui::Container > containerFileName = new ui::Container();
	if (!containerFileName->create(containerFile, ui::WsNone, new ui::TableLayout(L"100%,*,*,*", L"*", 0, ui::dpi96(4))))
		return false;

	m_editFileName = new ui::Edit();
	if (!m_editFileName->create(containerFileName, L""))
		return false;

	Ref< ui::Button > browseButton = new ui::Button();
	if (!browseButton->create(containerFileName, L"..."))
		return false;
	browseButton->addEventHandler< ui::ButtonClickEvent >(this, &MeshAssetEditor::eventBrowseClick);

	Ref< ui::Button > previewModelButton = new ui::Button();
	if (!previewModelButton->create(containerFileName, i18n::Text(L"MESHASSET_EDITOR_PREVIEW_MESH")))
		return false;
	previewModelButton->addEventHandler< ui::ButtonClickEvent >(this, &MeshAssetEditor::eventPreviewModelClick);

	Ref< ui::Button > editModelButton = new ui::Button();
	if (!editModelButton->create(containerFileName, i18n::Text(L"MESHASSET_EDITOR_EDIT_MESH")))
		return false;
	editModelButton->addEventHandler< ui::ButtonClickEvent >(this, &MeshAssetEditor::eventEditModelClick);

	Ref< ui::Static > staticImportFilter = new ui::Static();
	if (!staticImportFilter->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_IMPORT_FILTER")))
		return false;

	m_editImportFilter = new ui::Edit();
	if (!m_editImportFilter->create(containerFile, L""))
		return false;

	Ref< ui::Static > staticMeshType = new ui::Static();
	if (!staticMeshType->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE")))
		return false;

	m_dropMeshType = new ui::DropDown();
	if (!m_dropMeshType->create(containerFile))
		return false;

	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_BLEND"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_INDOOR"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_INSTANCE"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_LOD"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_PARTITION"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_PROC"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_SKINNED"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_STATIC"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_STREAM"));
	m_dropMeshType->addEventHandler< ui::SelectionChangeEvent >(this, &MeshAssetEditor::eventMeshTypeChange);

	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerFile, L"");

	Ref< ui::Container > containerOptions = new ui::Container();
	containerOptions->create(containerFile, ui::WsNone, new ui::TableLayout(L"50%,50%", L"*", 0, 0));

	Ref< ui::Container > containerLeft = new ui::Container();
	containerLeft->create(containerOptions, ui::WsNone, new ui::TableLayout(L"*", L"*", 0, 4));

	m_checkRenormalize = new ui::CheckBox();
	if (!m_checkRenormalize->create(containerLeft, i18n::Text(L"MESHASSET_EDITOR_RENORMALIZE")))
		return false;

	m_checkCenter = new ui::CheckBox();
	if (!m_checkCenter->create(containerLeft, i18n::Text(L"MESHASSET_EDITOR_CENTER")))
		return false;

	Ref< ui::Container > containerRight = new ui::Container();
	containerRight->create(containerOptions, ui::WsNone, new ui::TableLayout(L"*,*", L"*", 0, 4));

	m_staticLodSteps = new ui::Static();
	m_staticLodSteps->create(containerRight, i18n::Format(L"MESHASSET_EDITOR_LOD_STEPS", 1));

	m_sliderLodSteps = new ui::Slider();
	m_sliderLodSteps->create(containerRight);
	m_sliderLodSteps->setRange(1, 64);
	m_sliderLodSteps->addEventHandler< ui::ContentChangeEvent >(this, &MeshAssetEditor::eventLodStepsChange);

	Ref< ui::Static > staticLodMaxDistance = new ui::Static();
	staticLodMaxDistance->create(containerRight, i18n::Text(L"MESHASSET_EDITOR_LOD_MAX_DISTANCE"));

	m_editLodMaxDistance = new ui::Edit();
	m_editLodMaxDistance->create(containerRight, L"", ui::WsNone, new ui::NumericEditValidator(true, 0.0f, 10000.0f, 2));

	Ref< ui::Static > staticLodCullDistance = new ui::Static();
	staticLodCullDistance->create(containerRight, i18n::Text(L"MESHASSET_EDITOR_LOD_CULL_DISTANCE"));

	m_editLodCullDistance = new ui::Edit();
	m_editLodCullDistance->create(containerRight, L"", ui::WsNone, new ui::NumericEditValidator(true, 0.0f, 10000.0f, 2));

	Ref< ui::Static > staticScaleFactor = new ui::Static();
	staticScaleFactor->create(containerRight, i18n::Text(L"MESHASSET_EDITOR_SCALE_FACTOR"));

	m_editScaleFactor = new ui::Edit();
	m_editScaleFactor->create(containerRight, L"", ui::WsNone, new ui::NumericEditValidator(true, 0.0f, 10000.0f, 2));

	m_containerMaterials = new ui::Container();
	if (!m_containerMaterials->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*,100%", 0, 0)))
		return false;

	// Material shaders.
	Ref< ui::ToolBar > materialShaderTools = new ui::ToolBar();
	if (!materialShaderTools->create(m_containerMaterials))
		return false;

	materialShaderTools->addItem(new ui::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_BROWSE_TEMPLATE"), ui::Command(L"MeshAssetEditor.BrowseTemplate")));
	materialShaderTools->addItem(new ui::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_REMOVE_TEMPLATE"), ui::Command(L"MeshAssetEditor.RemoveTemplate")));
	materialShaderTools->addItem(new ui::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_CREATE_SHADER"), ui::Command(L"MeshAssetEditor.CreateShader")));
	materialShaderTools->addItem(new ui::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_BROWSE_SHADER"), ui::Command(L"MeshAssetEditor.BrowseShader")));
	materialShaderTools->addItem(new ui::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_REMOVE_SHADER"), ui::Command(L"MeshAssetEditor.RemoveShader")));
	materialShaderTools->addEventHandler< ui::ToolBarButtonClickEvent >(this, &MeshAssetEditor::eventMaterialShaderToolClick);

	m_materialShaderList = new ui::GridView();
	if (!m_materialShaderList->create(m_containerMaterials, ui::GridView::WsColumnHeader | ui::WsDoubleBuffer))
		return false;

	m_materialShaderList->addColumn(new ui::GridColumn(i18n::Text(L"MESHASSET_EDITOR_MATERIAL"), ui::dpi96(180)));
	m_materialShaderList->addColumn(new ui::GridColumn(i18n::Text(L"MESHASSET_EDITOR_TEMPLATE"), ui::dpi96(180)));
	m_materialShaderList->addColumn(new ui::GridColumn(i18n::Text(L"MESHASSET_EDITOR_SHADER"), ui::dpi96(300)));
	m_materialShaderList->addEventHandler< ui::MouseDoubleClickEvent >(this, &MeshAssetEditor::eventMaterialShaderListDoubleClick);

	// Material textures.
	Ref< ui::ToolBar > materialTextureTools = new ui::ToolBar();
	if (!materialTextureTools->create(m_containerMaterials))
		return false;

	materialTextureTools->addItem(new ui::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_CREATE_TEXTURE"), ui::Command(L"MeshAssetEditor.CreateTexture")));
	materialTextureTools->addItem(new ui::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_BROWSE_TEXTURE"), ui::Command(L"MeshAssetEditor.BrowseTexture")));
	materialTextureTools->addItem(new ui::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_REMOVE_TEXTURE"), ui::Command(L"MeshAssetEditor.RemoveTexture")));
	materialTextureTools->addEventHandler< ui::ToolBarButtonClickEvent >(this, &MeshAssetEditor::eventMaterialTextureToolClick);

	m_materialTextureList = new ui::GridView();
	if (!m_materialTextureList->create(m_containerMaterials, ui::GridView::WsColumnHeader | ui::WsDoubleBuffer))
		return false;

	m_materialTextureList->addColumn(new ui::GridColumn(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_NAME"), ui::dpi96(180)));
	m_materialTextureList->addColumn(new ui::GridColumn(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_ASSET"), ui::dpi96(300)));
	m_materialTextureList->addColumn(new ui::GridColumn(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_USAGE"), ui::dpi96(300)));
	m_materialTextureList->addEventHandler< ui::MouseDoubleClickEvent >(this, &MeshAssetEditor::eventMaterialTextureListDoubleClick);

	updateModel();
	updateFile();
	updateMaterialList();

	return true;
}

void MeshAssetEditor::destroy()
{
}

void MeshAssetEditor::apply()
{
	m_asset->setFileName(m_editFileName->getText());
	m_asset->setImportFilter(m_editImportFilter->getText());
	m_asset->setMeshType(MeshAsset::MeshType(m_dropMeshType->getSelected()));
	m_asset->setRenormalize(m_checkRenormalize->isChecked());
	m_asset->setCenter(m_checkCenter->isChecked());
	m_asset->setLodSteps(m_sliderLodSteps->getValue());
	m_asset->setLodMaxDistance(parseString< float >(m_editLodMaxDistance->getText()));
	m_asset->setLodCullDistance(parseString< float >(m_editLodCullDistance->getText()));
	m_asset->setScaleFactor(parseString< float >(m_editScaleFactor->getText()));

	std::map< std::wstring, Guid > materialTemplates;
	std::map< std::wstring, Guid > materialShaders;

	const RefArray< ui::GridRow >& shaderItems = m_materialShaderList->getRows();
	for (auto shaderItem : shaderItems)
	{
		std::wstring materialName = shaderItem->get(0)->getText();
		Ref< db::Instance > materialTemplateInstance = shaderItem->getData< db::Instance >(L"TEMPLATE");
		Ref< db::Instance > materialShaderInstance = shaderItem->getData< db::Instance >(L"INSTANCE");

		if (materialTemplateInstance)
			materialTemplates.insert(std::make_pair(materialName, materialTemplateInstance->getGuid()));
		if (materialShaderInstance)
			materialShaders.insert(std::make_pair(materialName, materialShaderInstance->getGuid()));
	}

	m_asset->setMaterialTemplates(materialTemplates);
	m_asset->setMaterialShaders(materialShaders);

	std::map< std::wstring, Guid > materialTextures;

	const RefArray< ui::GridRow >& textureItems = m_materialTextureList->getRows();
	for (auto textureItem : textureItems)
	{
		std::wstring textureName = textureItem->get(0)->getText();
		Ref< db::Instance > materialTextureInstance = textureItem->getData< db::Instance >(L"INSTANCE");

		if (materialTextureInstance)
			materialTextures.insert(std::make_pair(textureName, materialTextureInstance->getGuid()));
	}

	m_asset->setMaterialTextures(materialTextures);

	m_instance->setObject(m_asset);
}

bool MeshAssetEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void MeshAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size MeshAssetEditor::getPreferredSize() const
{
	return ui::Size(
		ui::dpi96(800),
		ui::dpi96(600)
	);
}

void MeshAssetEditor::updateModel()
{
	Path assetPath = FileSystem::getInstance().getAbsolutePath(m_assetPath, m_asset->getFileName());
	std::wstring importFilter = m_asset->getImportFilter();
	m_model = model::ModelCache(m_modelCachePath).get(assetPath, importFilter);
}

void MeshAssetEditor::updateFile()
{
	Path assetPath = FileSystem::getInstance().getAbsolutePath(m_assetPath, m_asset->getFileName());

	Path assetRelPath;
	if (!FileSystem::getInstance().getRelativePath(assetPath, m_assetPath, assetRelPath))
		assetRelPath = m_asset->getFileName().getOriginal();

	m_editFileName->setText(assetRelPath.getPathName());
	m_editImportFilter->setText(m_asset->getImportFilter());
	m_dropMeshType->select(m_asset->getMeshType());
	m_checkRenormalize->setChecked(m_asset->getRenormalize());
	m_checkCenter->setChecked(m_asset->getCenter());

	m_staticLodSteps->setText(i18n::Format(L"MESHASSET_EDITOR_LOD_STEPS", m_asset->getLodSteps()));

	m_sliderLodSteps->setEnable(m_asset->getMeshType() == MeshAsset::MtLod);
	m_sliderLodSteps->setValue(m_asset->getLodSteps());

	m_editLodMaxDistance->setEnable(m_asset->getMeshType() == MeshAsset::MtLod);
	m_editLodMaxDistance->setText(toString(m_asset->getLodMaxDistance()));

	m_editLodCullDistance->setEnable(m_asset->getMeshType() == MeshAsset::MtLod);
	m_editLodCullDistance->setText(toString(m_asset->getLodCullDistance()));

	m_editScaleFactor->setText(toString(m_asset->getScaleFactor()));
}

void MeshAssetEditor::updateMaterialList()
{
	m_materialShaderList->removeAllRows();
	m_materialTextureList->removeAllRows();

	if (m_model)
	{
		const AlignedVector< model::Material >& materials = m_model->getMaterials();

		const std::map< std::wstring, Guid >& materialTemplates = m_asset->getMaterialTemplates();
		const std::map< std::wstring, Guid >& materialShaders = m_asset->getMaterialShaders();

		for (const auto& material : materials)
		{
			Ref< ui::GridRow > shaderItem = new ui::GridRow();
			shaderItem->add(new ui::GridItem(material.getName()));

			std::wstring materialTemplate = i18n::Text(L"MESHASSET_EDITOR_TEMPLATE_NOT_ASSIGNED");
			std::wstring materialShader = i18n::Text(L"MESHASSET_EDITOR_SHADER_NOT_ASSIGNED");

			// Find template for material.
			{
				auto it = materialTemplates.find(material.getName());
				if (it != materialTemplates.end())
				{
					if (!it->second.isNull())
					{
						Ref< db::Instance > materialTemplateInstance = m_editor->getSourceDatabase()->getInstance(it->second);
						if (materialTemplateInstance)
						{
							materialTemplate = materialTemplateInstance->getName();
							shaderItem->setData(L"TEMPLATE", materialTemplateInstance);
						}
					}
					else
						materialTemplate = i18n::Text(L"MESHASSET_EDITOR_DISABLED");
				}
			}

			// Find shader for material.
			{
				auto it = materialShaders.find(material.getName());
				if (it != materialShaders.end())
				{
					if (!it->second.isNull())
					{
						Ref< db::Instance > materialShaderInstance = m_editor->getSourceDatabase()->getInstance(it->second);
						if (materialShaderInstance)
						{
							materialShader = materialShaderInstance->getName();
							shaderItem->setData(L"INSTANCE", materialShaderInstance);
						}
					}
					else
						materialShader = i18n::Text(L"MESHASSET_EDITOR_DISABLED");
				}
			}

			shaderItem->add(new ui::GridItem(materialTemplate));
			shaderItem->add(new ui::GridItem(materialShader));

			m_materialShaderList->addRow(shaderItem);
		}

		std::set< std::wstring > textureNames;
		const std::map< std::wstring, Guid >& materialTextures = m_asset->getMaterialTextures();
		for (const auto& material : materials)
		{
			std::wstring modelTextures[] =
			{
				material.getDiffuseMap().name,
				material.getSpecularMap().name,
				material.getRoughnessMap().name,
				material.getMetalnessMap().name,
				material.getTransparencyMap().name,
				material.getEmissiveMap().name,
				material.getReflectiveMap().name,
				material.getNormalMap().name
			};

			for (uint32_t j = 0; j < sizeof_array(modelTextures); ++j)
			{
				if (modelTextures[j].empty() || textureNames.find(modelTextures[j]) != textureNames.end())
					continue;

				textureNames.insert(modelTextures[j]);

				Ref< db::Instance > materialTextureInstance;
				std::wstring materialTexture = i18n::Text(L"MESHASSET_EDITOR_TEXTURE_NOT_ASSIGNED");

				auto it = materialTextures.find(modelTextures[j]);
				if (it != materialTextures.end())
				{
					materialTextureInstance = m_editor->getSourceDatabase()->getInstance(it->second);
					if (materialTextureInstance)
						materialTexture = materialTextureInstance->getName();
				}

				Ref< ui::GridRow > textureItem = new ui::GridRow();
				textureItem->add(new ui::GridItem(modelTextures[j]));
				textureItem->add(new ui::GridItem(materialTexture));

				StringOutputStream ss;
				if (modelTextures[j] == material.getDiffuseMap().name)
					ss << L" | Diffuse";
				if (modelTextures[j] == material.getSpecularMap().name)
					ss << L" | Specular";
				if (modelTextures[j] == material.getRoughnessMap().name)
					ss << L" | Roughness";
				if (modelTextures[j] == material.getMetalnessMap().name)
					ss << L" | Metalness";
				if (modelTextures[j] == material.getTransparencyMap().name)
					ss << L" | Transparency";
				if (modelTextures[j] == material.getEmissiveMap().name)
					ss << L" | Emissive";
				if (modelTextures[j] == material.getReflectiveMap().name)
					ss << L" | Reflective (*)";
				if (modelTextures[j] == material.getNormalMap().name)
					ss << L" | Normal";
				textureItem->add(new ui::GridItem(ss.str().substr(3)));

				textureItem->setData(L"INSTANCE", materialTextureInstance);
				m_materialTextureList->addRow(textureItem);
			}
		}

		m_containerMaterials->setEnable(true);
	}
	else
		m_containerMaterials->setEnable(false);
}

void MeshAssetEditor::browseMaterialTemplate()
{
	Ref< ui::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialTemplateInstance = m_editor->browseInstance(type_of< render::ShaderGraph >());
	if (materialTemplateInstance)
	{
		selectedItem->set(1, new ui::GridItem(materialTemplateInstance->getName()));
		selectedItem->setData(L"TEMPLATE", materialTemplateInstance);
		m_materialShaderList->requestUpdate();
	}
}

void MeshAssetEditor::removeMaterialTemplate()
{
	Ref< ui::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	selectedItem->set(1, new ui::GridItem(i18n::Text(L"MESHASSET_EDITOR_TEMPLATE_NOT_ASSIGNED")));
	selectedItem->setData(L"TEMPLATE", nullptr);
	m_materialShaderList->requestUpdate();
}

void MeshAssetEditor::createMaterialShader()
{
	Ref< ui::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	std::wstring materialName = selectedItem->get(0)->getText();
	std::wstring outputName = materialName;

	// Find model material to associate shader with.
	auto materials = m_model->getMaterials();
	auto it = std::find_if(materials.begin(), materials.end(), [&](const model::Material& material) {
		return material.getName() == materialName;
	});
	if (it == materials.end())
		return;

	// Query user about material name; default model's material name.
	ui::InputDialog::Field materialNameField(
		L"",
		outputName
	);

	ui::InputDialog materialNameDialog;
	if (materialNameDialog.create(m_materialShaderList, i18n::Text(L"MESHASSET_EDITOR_ENTER_NAME"), i18n::Text(L"MESHASSET_EDITOR_ENTER_NAME"), &materialNameField, 1))
	{
		if (materialNameDialog.showModal() == ui::DrOk)
			outputName = materialNameField.value;
		else
			outputName.clear();

		materialNameDialog.destroy();
	}

	if (outputName.empty())
		return;

	// Ensure no existing instance with given name.
	if (m_instance->getParent()->getInstance(outputName) != nullptr)
	{
		ui::MessageBox::show(
			m_materialShaderList,
			i18n::Text(L"MESHASSET_EDITOR_MESSAGE_ALREADY_EXIST"),
			i18n::Text(L"MESHASSET_EDITOR_CAPTION_ALREADY_EXIST"),
			ui::MbOk | ui::MbIconExclamation
		);
		return;
	}

	// Use material template if specified.
	Guid materialTemplate;
	const auto& materialTemplates = m_asset->getMaterialTemplates();
	auto it2 = materialTemplates.find(materialName);
	if (it2 != materialTemplates.end())
		materialTemplate = it2->second;

	// Set textures specified in MeshAsset into material maps.
	const auto& materialTextures = m_asset->getMaterialTextures();
	for (auto& m : materials)
	{
		model::Material::Map maps[] =
		{
			m.getDiffuseMap(),
			m.getSpecularMap(),
			m.getRoughnessMap(),
			m.getMetalnessMap(),
			m.getTransparencyMap(),
			m.getEmissiveMap(),
			m.getReflectiveMap(),
			m.getNormalMap(),
			m.getLightMap()
		};
			
		for (auto& map : maps)
		{
			auto it = materialTextures.find(map.name);
			if (it != materialTextures.end())
				map.texture = it->second;
		}

		m.setDiffuseMap(maps[0]);
		m.setSpecularMap(maps[1]);
		m.setRoughnessMap(maps[2]);
		m.setMetalnessMap(maps[3]);
		m.setTransparencyMap(maps[4]);
		m.setEmissiveMap(maps[5]);
		m.setReflectiveMap(maps[6]);
		m.setNormalMap(maps[7]);
		m.setLightMap(maps[8]);
	}
	m_model->setMaterials(materials);

	// Generate shader.
	Ref< render::ShaderGraph > materialShader = MaterialShaderGenerator().generate(
		m_editor->getSourceDatabase(),
		*m_model,
		*it,
		materialTemplate,
		haveVertexColors(*m_model)
	);
	if (materialShader)
	{
		materialShader = render::ShaderGraphStatic(materialShader).propagateConstantExternalValues();
		T_ASSERT(materialShader != nullptr);

		materialShader = render::ShaderGraphOptimizer(materialShader).removeUnusedBranches();
		T_ASSERT(materialShader != nullptr);

		Ref< db::Instance > materialShaderInstance = m_instance->getParent()->createInstance(outputName);
		if (materialShaderInstance)
		{
			materialShaderInstance->setObject(materialShader);
			if (materialShaderInstance->commit())
			{
				selectedItem->set(2, new ui::GridItem(materialShaderInstance->getName()));
				selectedItem->setData(L"INSTANCE", materialShaderInstance);
				m_materialShaderList->requestUpdate();
				m_editor->updateDatabaseView();
			}
		}
	}
}

void MeshAssetEditor::browseMaterialShader()
{
	Ref< ui::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialShaderInstance = m_editor->browseInstance(type_of< render::ShaderGraph >());
	if (materialShaderInstance)
	{
		selectedItem->set(2, new ui::GridItem(materialShaderInstance->getName()));
		selectedItem->setData(L"INSTANCE", materialShaderInstance);
		m_materialShaderList->requestUpdate();
	}
}

void MeshAssetEditor::removeMaterialShader()
{
	Ref< ui::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	selectedItem->set(2, new ui::GridItem(i18n::Text(L"MESHASSET_EDITOR_SHADER_NOT_ASSIGNED")));
	selectedItem->setData(L"INSTANCE", nullptr);
	m_materialShaderList->requestUpdate();
}

void MeshAssetEditor::createMaterialTexture()
{
}

void MeshAssetEditor::browseMaterialTexture()
{
	Ref< ui::GridRow > selectedItem = m_materialTextureList->getSelectedRow();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialTextureInstance = m_editor->browseInstance(type_of< render::ITexture >());
	if (materialTextureInstance)
	{
		selectedItem->set(1, new ui::GridItem(materialTextureInstance->getName()));
		selectedItem->setData(L"INSTANCE", materialTextureInstance);
		m_materialTextureList->requestUpdate();
	}
}

void MeshAssetEditor::removeMaterialTexture()
{
	Ref< ui::GridRow > selectedItem = m_materialTextureList->getSelectedRow();
	if (!selectedItem)
		return;

	selectedItem->set(1, new ui::GridItem(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_NOT_ASSIGNED")));
	selectedItem->setData(L"INSTANCE", nullptr);
	m_materialTextureList->requestUpdate();
}

void MeshAssetEditor::eventMeshTypeChange(ui::SelectionChangeEvent* event)
{
	m_asset->setMeshType(MeshAsset::MeshType(m_dropMeshType->getSelected()));
	updateFile();
}

void MeshAssetEditor::eventLodStepsChange(ui::ContentChangeEvent* event)
{
	m_staticLodSteps->setText(i18n::Format(L"MESHASSET_EDITOR_LOD_STEPS", m_sliderLodSteps->getValue()));
}

void MeshAssetEditor::eventBrowseClick(ui::ButtonClickEvent* event)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(m_editFileName, type_name(this), i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
		return;

	Path assetPath = FileSystem::getInstance().getAbsolutePath(m_assetPath);
	Path path = assetPath + Path(m_editFileName->getText());

	if (fileDialog.showModal(path) == ui::DrOk)
	{
		// Try get path relative to asset path.
		Path relPath;
		if (FileSystem::getInstance().getRelativePath(path, assetPath, relPath))
			path = relPath;

		m_editFileName->setText(path.getOriginal());
		m_asset->setFileName(path);

		updateModel();
		updateFile();
		updateMaterialList();
	}

	fileDialog.destroy();
}

void MeshAssetEditor::eventPreviewModelClick(ui::ButtonClickEvent* event)
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	Ref< PropertyGroup > params = new PropertyGroup();
	params->setProperty< PropertyString >(L"fileName", assetPath + L"/" + m_editFileName->getText());
	params->setProperty< PropertyFloat >(L"scale", m_asset->getScaleFactor());

	m_editor->openTool(L"traktor.model.ModelTool", params);
}

void MeshAssetEditor::eventEditModelClick(ui::ButtonClickEvent* event)
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	Path path = FileSystem::getInstance().getAbsolutePath(Path(assetPath + L"/" + m_editFileName->getText()));
	OS::getInstance().openFile(path.getPathName());
}

void MeshAssetEditor::eventMaterialShaderToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"MeshAssetEditor.BrowseTemplate")
		browseMaterialTemplate();
	else if (command == L"MeshAssetEditor.RemoveTemplate")
		removeMaterialTemplate();
	else if (command == L"MeshAssetEditor.CreateShader")
		createMaterialShader();
	else if (command == L"MeshAssetEditor.BrowseShader")
		browseMaterialShader();
	else if (command == L"MeshAssetEditor.RemoveShader")
		removeMaterialShader();
}

void MeshAssetEditor::eventMaterialShaderListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< ui::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	ui::Point mousePosition = event->getPosition();

	int32_t column = m_materialShaderList->getColumnIndex(mousePosition.x);
	if (column == 1)
	{
		// Template
		Ref< db::Instance > materialTemplateInstance = selectedItem->getData< db::Instance >(L"TEMPLATE");
		if (!materialTemplateInstance)
			browseMaterialTemplate();
		else
			m_editor->openEditor(materialTemplateInstance);
	}
	else if (column == 2)
	{
		// Shader
		Ref< db::Instance > materialShaderInstance = selectedItem->getData< db::Instance >(L"INSTANCE");
		if (!materialShaderInstance)
			browseMaterialShader();
		else
			m_editor->openEditor(materialShaderInstance);
	}
}

void MeshAssetEditor::eventMaterialTextureToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"MeshAssetEditor.CreateTexture")
		createMaterialTexture();
	else if (command == L"MeshAssetEditor.BrowseTexture")
		browseMaterialTexture();
	else if (command == L"MeshAssetEditor.RemoveTexture")
		removeMaterialTexture();
}

void MeshAssetEditor::eventMaterialTextureListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< ui::GridRow > selectedItem = m_materialTextureList->getSelectedRow();
	if (!selectedItem)
		return;

	ui::Point mousePosition = event->getPosition();

	int32_t column = m_materialTextureList->getColumnIndex(mousePosition.x);
	if (column == 1)
	{
		Ref< db::Instance > materialTextureInstance = selectedItem->getData< db::Instance >(L"INSTANCE");
		if (!materialTextureInstance)
			browseMaterialTexture();
		else
			m_editor->openEditor(materialTextureInstance);
	}
}

	}
}
