#include <list>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Thread/Acquire.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/IMeshResource.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshPipeline.h"
#include "Mesh/Editor/Blend/BlendMeshConverter.h"
#include "Mesh/Editor/Indoor/IndoorMeshConverter.h"
#include "Mesh/Editor/Instance/InstanceMeshConverter.h"
#include "Mesh/Editor/Lod/AutoLodMeshConverter.h"
#include "Mesh/Editor/Partition/PartitionMeshConverter.h"
#include "Mesh/Editor/Proc/ProcMeshConverter.h"
#include "Mesh/Editor/Skinned/SkinnedMeshConverter.h"
#include "Mesh/Editor/Static/StaticMeshConverter.h"
#include "Mesh/Editor/Stream/StreamMeshConverter.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CullDistantFaces.h"
#include "Model/Operations/Transform.h"
#include "Render/Editor/IProgramCompiler.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphHash.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Editor/Texture/TextureSet.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const static Guid c_guidVertexInterfaceGuid(L"{0A9BE5B4-4B45-B84A-AE16-57F6483436FC}");

class FragmentReaderAdapter : public render::FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder)
	:	m_pipelineBuilder(pipelineBuilder)
	{
	}

	virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid) const
	{
		Ref< const render::ShaderGraph > shaderGraph = m_pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
		if (!shaderGraph)
			return nullptr;

		if (render::ShaderGraphValidator(shaderGraph, fragmentGuid).validateIntegrity())
			return shaderGraph;
		else
			return nullptr;
	}

private:
	Ref< editor::IPipelineBuilder > m_pipelineBuilder;
};

bool haveVertexColors(const model::Model& model)
{
	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
	{
		if (model.getVertex(i).getColor() != model::c_InvalidIndex)
			return true;
	}
	return false;
}

Guid getVertexShaderGuid(MeshAsset::MeshType meshType)
{
	switch (meshType)
	{
	case MeshAsset::MtBlend:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	case MeshAsset::MtIndoor:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	case MeshAsset::MtInstance:
		return Guid(L"{A714A83F-8442-6F48-A2A7-6EFA95EB75F3}");

	case MeshAsset::MtLod:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	case MeshAsset::MtPartition:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	case MeshAsset::MtSkinned:
		return Guid(L"{69A3CF2E-9B63-0440-9410-70AB4AE127CE}");

	case MeshAsset::MtStatic:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	case MeshAsset::MtStream:
		return Guid(L"{14AE48E1-723D-0944-821C-4B73AC942437}");

	default:
		return Guid();
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshPipeline", 33, MeshPipeline, editor::IPipeline)

MeshPipeline::MeshPipeline()
:	m_promoteHalf(false)
,	m_enableCustomShaders(true)
,	m_enableCustomTemplates(true)
,	m_editor(false)
{
}

bool MeshPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");
	m_promoteHalf = settings->getProperty< bool >(L"MeshPipeline.PromoteHalf", false);
	m_enableCustomShaders = settings->getProperty< bool >(L"MeshPipeline.EnableCustomShaders", true);
	m_enableCustomTemplates = settings->getProperty< bool >(L"MeshPipeline.EnableCustomTemplates", true);
	m_includeOnlyTechniques = settings->getProperty< std::set< std::wstring > >(L"ShaderPipeline.IncludeOnlyTechniques");
	m_programCompilerTypeName = settings->getProperty< std::wstring >(L"ShaderPipeline.ProgramCompiler");
	m_platform = settings->getProperty< std::wstring >(L"ShaderPipeline.Platform", L"");
	m_editor = settings->getProperty< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

void MeshPipeline::destroy()
{
	m_programCompiler = nullptr;
}

TypeInfoSet MeshPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< MeshAsset >();
}

uint32_t MeshPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool MeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const MeshAsset > asset = checked_type_cast< const MeshAsset* >(sourceAsset);
	T_ASSERT(asset);

	if (!asset->getFileName().empty())
		pipelineDepends->addDependency(Path(m_assetPath), asset->getFileName().getOriginal());

	// Determine vertex shader guid.
	Guid vertexShaderGuid = getVertexShaderGuid(asset->getMeshType());
	if (!vertexShaderGuid.isValid())
	{
		log::error << L"Mesh pipeline failed; unknown mesh asset type" << Endl;
		return false;
	}
	pipelineDepends->addDependency(vertexShaderGuid, editor::PdfUse);

	// Add dependencies to generator fragments.
	MaterialShaderGenerator().addDependencies(pipelineDepends);

	// Add dependencies to material templates.
	if (m_enableCustomTemplates)
	{
		const std::map< std::wstring, Guid >& materialTemplates = asset->getMaterialTemplates();
		for (std::map< std::wstring, Guid >::const_iterator i = materialTemplates.begin(); i != materialTemplates.end(); ++i)
			pipelineDepends->addDependency(i->second, editor::PdfUse);
	}

	// Add dependencies to "fixed" material shaders.
	if (m_enableCustomShaders)
	{
		const std::map< std::wstring, Guid >& materialShaders = asset->getMaterialShaders();
		for (std::map< std::wstring, Guid >::const_iterator i = materialShaders.begin(); i != materialShaders.end(); ++i)
			pipelineDepends->addDependency(i->second, editor::PdfUse);
	}

	// Add dependencies to material textures.
	const std::map< std::wstring, Guid >& materialTextures = asset->getMaterialTextures();
	for (std::map< std::wstring, Guid >::const_iterator i = materialTextures.begin(); i != materialTextures.end(); ++i)
		pipelineDepends->addDependency(i->second, editor::PdfBuild | editor::PdfResource);

	pipelineDepends->addDependency(asset->getTextureSet(), editor::PdfBuild);

	pipelineDepends->addDependency< render::ShaderGraph >();
	return true;
}

bool MeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* /*sourceInstance*/,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t /*reason*/
) const
{
	std::map< std::wstring, Guid > materialTextures;
	std::map< std::wstring, model::Material > materials;
	RefArray< model::Model > models;
	uint32_t polygonCount = 0;
	Aabb3 boundingBox;

	Ref< render::IProgramCompiler > programCompiler = getProgramCompiler();
	if (!programCompiler)
		return false;

	auto asset = mandatory_non_null_type_cast< const MeshAsset* >(sourceAsset);
	auto& materialTemplates = asset->getMaterialTemplates();
	auto& materialShaders = asset->getMaterialShaders();

	// Create list of texture references.
	const auto& textureSetId = asset->getTextureSet();
	if (textureSetId.isNotNull())
	{
		Ref< const render::TextureSet > textureSet = pipelineBuilder->getObjectReadOnly< render::TextureSet >(textureSetId);
		if (!textureSet)
		{
			log::error << L"Missing texture set reference." << Endl;
			return false;
		}
		materialTextures = textureSet->get();
	}

	// Explicit material textures override those from a texture set.
	for (const auto& mt : asset->getMaterialTextures())
		materialTextures[mt.first] = mt.second;

	// Create mesh converter.
	Ref< IMeshConverter > converter;
	switch (asset->getMeshType())
	{
	case MeshAsset::MtBlend:
		converter = new BlendMeshConverter();
		break;

	case MeshAsset::MtIndoor:
		converter = new IndoorMeshConverter();
		break;

	case MeshAsset::MtInstance:
		converter = new InstanceMeshConverter();
		break;

	case MeshAsset::MtLod:
		converter = new AutoLodMeshConverter();
		break;

	case MeshAsset::MtPartition:
		converter = new PartitionMeshConverter();
		break;

	case MeshAsset::MtProc:
		converter = new ProcMeshConverter();
		break;

	case MeshAsset::MtSkinned:
		converter = new SkinnedMeshConverter();
		break;

	case MeshAsset::MtStatic:
		converter = new StaticMeshConverter();
		break;

	case MeshAsset::MtStream:
		converter = new StreamMeshConverter();
		break;

	default:
		log::error << L"Mesh pipeline failed; unknown mesh asset type." << Endl;
		return false;
	}

	// Create list of model operations we need to perform on model before converting it.
	RefArray< const model::IModelOperation > operations;
	if (!converter->getOperations(asset, operations))
	{
		log::error << L"Mesh pipeline failed; unable to create model operations." << Endl;
		return false;
	}

	// Scale model according to scale factor in asset.
	operations.push_back(new model::Transform(
		scale(asset->getScaleFactor(), asset->getScaleFactor(), asset->getScaleFactor())
	));

	// Recalculate normals regardless if already exist in model.
	if (asset->getRenormalize())
		operations.push_back(new model::CalculateTangents(true));

	// We allow models to be passed as build parameters in case models
	// are procedurally generated.
	if (buildParams)
	{
		// Create a mutable copy of model since we modify it.
		Ref< model::Model > model = DeepClone(checked_type_cast< const ISerializable* >(buildParams)).create< model::Model >();
		if (model->getPolygonCount() == 0)
		{
			log::error << L"Mesh pipeline failed; no polygons in parametric source model." << Endl;
			return false;
		}

		for (auto operation : operations)
			operation->apply(*model);

		models.push_back(model);
	}
	else
	{
		log::info << L"Loading model \"" << asset->getFileName().getFileName() << L"\"..." << Endl;

		// Load and prepare models through model cache.
		Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + asset->getFileName());
		Ref< model::Model > model = model::ModelCache(m_modelCachePath).get(filePath, asset->getImportFilter());
		if (!model)
		{
			log::error << L"Mesh pipeline failed; unable to read source model (" << asset->getFileName().getOriginal() << L")." << Endl;
			return false;
		}

		if (model->getPolygonCount() == 0)
		{
			log::error << L"Mesh pipeline failed; no polygons in source model (" << asset->getFileName().getOriginal() << L")." << Endl;
			return false;
		}

		for (auto operation : operations)
			operation->apply(*model);		

		models.push_back(model);
	}

	if (models.empty())
	{
		log::error << L"Mesh pipeline failed; no model." << Endl;
		return false;
	}

	// Merge all materials into a single list (duplicates will be overridden).
	for (auto model : models)
	{
		if (asset->getCenter())
		{
			Aabb3 boundingBox = model->getBoundingBox();
			model::Transform(translate(-boundingBox.getCenter())).apply(*model);
		}

		const AlignedVector< model::Material >& modelMaterials = model->getMaterials();
		if (model->getMaterials().empty())
		{
			log::error << L"Mesh pipeline failed; no materials in source model(s)." << Endl;
			return false;
		}

		// Merge materials, set textures specified in MeshAsset into material maps.
		for (const auto& modelMaterial : modelMaterials)
		{
			const auto& name = modelMaterial.getName();

			auto& m = materials[name];
			m = modelMaterial;

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

		boundingBox.contain(model->getBoundingBox());
		polygonCount += model->getPolygonCount();
	}

	// Build materials.
	AlignedVector< render::VertexElement > vertexElements;
	uint32_t vertexElementOffset = 0;

	std::map< uint32_t, Ref< render::ShaderGraph > > materialTechniqueShaderGraphs;		//< Collection of all material technique fragments; later merged into single shader.
	std::map< std::wstring, std::list< MeshMaterialTechnique > > materialTechniqueMap;	//< Map from model material to technique fragments. ["Model material":["Default":hash0, "Depth":hash1, ...]]

	Guid vertexShaderGuid = getVertexShaderGuid(asset->getMeshType());
	T_ASSERT(vertexShaderGuid.isValid());

	Guid materialGuid = vertexShaderGuid.permutation(outputGuid);
	T_ASSERT(materialGuid.isValid());

	MaterialShaderGenerator generator;

	int32_t maxInstanceCount = 0;
	int32_t jointCount = models[0]->getJointCount();
	bool vertexColor = haveVertexColors(*models[0]);

	for (const auto& materialPair : materials)
	{
		Ref< const render::ShaderGraph > materialShaderGraph;

		auto it = materialShaders.find(materialPair.first);
		if (
			m_enableCustomShaders &&
			it != materialShaders.end()
		)
		{
			if (it->second.isNull())
			{
				log::info << L"Material \"" << materialPair.first << L"\" disabled; skipped." << Endl;
				continue;
			}

			materialShaderGraph = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(it->second);
			if (!materialShaderGraph)
			{
				log::error << L"Mesh pipeline failed; unable to read material shader \"" << materialPair.first << L"\"." << Endl;
				return false;
			}
		}
		else
		{
			Guid materialTemplate;

			if (m_enableCustomTemplates)
			{
				auto it = materialTemplates.find(materialPair.first);
				if (it != materialTemplates.end())
					materialTemplate = it->second;
			}

			materialShaderGraph = generator.generate(
				pipelineBuilder->getSourceDatabase(),
				*models[0],
				materialPair.second,
				materialTemplate,
				vertexColor
			);
			if (!materialShaderGraph)
			{
				log::error << L"Mesh pipeline failed; unable to generate material shader \"" << materialPair.first << L"\"." << Endl;
				return false;
			}
		}

		// Set vertex fragment reference.
		RefArray< render::External > externals;
		materialShaderGraph->findNodesOf< render::External >(externals);
		for (auto external : externals)
		{
			if (external->getFragmentGuid() == c_guidVertexInterfaceGuid)
				external->setFragmentGuid(vertexShaderGuid);
		}

		// Resolve all local variables.
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph).getVariableResolved(render::ShaderGraphStatic::VrtLocal);
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to resolve local variables." << Endl;
			return false;
		}

		// Link shader fragments.
		FragmentReaderAdapter fragmentReader(pipelineBuilder);
		materialShaderGraph = render::FragmentLinker(fragmentReader).resolve(materialShaderGraph, true);
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to link shader fragments, material shader \"" << materialPair.first << L"\"." << Endl;
			return false;
		}

		// Resolve all global variables.
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph).getVariableResolved(render::ShaderGraphStatic::VrtGlobal);
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to resolve global variables." << Endl;
			return false;
		}

		// Get connected permutation.
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph).getConnectedPermutation();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to freeze connected conditionals, material shader \"" << materialPair.first << L"\"." << Endl;
			return false;
		}

		// Extract platform permutation.
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph).getPlatformPermutation(m_platform);
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to get platform \"" << m_platform << L"\" permutation." << Endl;
			return false;
		}

		// Extract renderer permutation.
		const wchar_t* rendererSignature = programCompiler->getRendererSignature();
		T_ASSERT(rendererSignature);

		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph).getRendererPermutation(rendererSignature);
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to get renderer permutation." << Endl;
			return false;
		}

		// Freeze types, get typed permutation.
		materialShaderGraph = render::ShaderGraphStatic(materialShaderGraph).getTypePermutation();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to freeze types, material shader \"" << materialPair.first << L"\"." << Endl;
			return false;
		}

		// Merge identical branches.
		materialShaderGraph = render::ShaderGraphOptimizer(materialShaderGraph).mergeBranches();
		if (!materialShaderGraph)
		{
			log::error << L"MeshPipeline failed; unable to merge branches, material shader \"" << materialPair.first << L"\"." << Endl;
			return false;
		}

		// Update bone count from model.
		for (auto node : materialShaderGraph->getNodes())
		{
			if (render::IndexedUniform* indexedUniform = dynamic_type_cast< render::IndexedUniform* >(node))
			{
				if (
					indexedUniform->getParameterName() == L"Mesh_Joints" ||
					indexedUniform->getParameterName() == L"Mesh_LastJoints"
				)
				{
					// Quantize joint count to reduce number of vertex shader permutations as it
					// will cost more than excessive parameters.
					int32_t uniformJointCount = alignUp(jointCount, 4);
					if (uniformJointCount * 2 != indexedUniform->getLength())
						indexedUniform->setLength(uniformJointCount * 2);		// Each bone is represented of a quaternion and a vector thus multiply by 2.
				}
				else if (indexedUniform->getParameterName() == L"InstanceWorld")
				{
					// Determine how many instances we can use when rendering instanced meshed
					// based on how many entries in uniform.
					if (maxInstanceCount <= 0 || maxInstanceCount > indexedUniform->getLength() / 2)
						maxInstanceCount = indexedUniform->getLength() / 2;		// Length of uniform is twice of max number of instances.
				}
			}
		}

		// Extract each material technique.
		std::set< std::wstring > materialTechniqueNames = render::ShaderGraphTechniques(materialShaderGraph).getNames();
		if (!m_includeOnlyTechniques.empty())
		{
			std::set< std::wstring > keepTechniqueNames;
			for (const auto& includeOnlyTechniques : m_includeOnlyTechniques)
			{
				WildCompare wc(includeOnlyTechniques);
				for (const auto& materialTechniqueName : materialTechniqueNames)
				{
					if (wc.match(materialTechniqueName))
						keepTechniqueNames.insert(materialTechniqueName);
				}
			}
			materialTechniqueNames = keepTechniqueNames;
		}

		for (const auto& materialTechniqueName : materialTechniqueNames)
		{
			Ref< render::ShaderGraph > materialTechniqueShaderGraph = render::ShaderGraphTechniques(materialShaderGraph).generate(materialTechniqueName);

			uint32_t hash = render::ShaderGraphHash::calculate(materialTechniqueShaderGraph);
			std::wstring shaderTechniqueName = str(L"M/%s/%08x", materialTechniqueName.c_str(), hash);

			for (auto node : materialTechniqueShaderGraph->getNodes())
			{
				if (auto vertexOutputNode = dynamic_type_cast< render::VertexOutput* >(node))
					vertexOutputNode->setTechnique(shaderTechniqueName);
				if (auto pixelOutputNode = dynamic_type_cast< render::PixelOutput* >(node))
					pixelOutputNode->setTechnique(shaderTechniqueName);
			}

			materialTechniqueShaderGraphs[hash] = materialTechniqueShaderGraph;

			MeshMaterialTechnique mt;
			mt.worldTechnique = materialTechniqueName;
			mt.shaderTechnique = shaderTechniqueName;
			mt.hash = hash;
			materialTechniqueMap[materialPair.first].push_back(mt);
		}

		// Build vertex declaration from shader vertex inputs.
		RefArray< render::VertexInput > vertexInputNodes;
		materialShaderGraph->findNodesOf< render::VertexInput >(vertexInputNodes);
		for (auto vertexInputNode : vertexInputNodes)
		{
			render::DataType elementDataType = vertexInputNode->getDataType();
			if (m_promoteHalf)
			{
				if (elementDataType == render::DtHalf2)
					elementDataType = render::DtFloat2;
				else if (elementDataType == render::DtHalf4)
					elementDataType = render::DtFloat4;
			}

			// Is it already added to vertex declaration?
			bool elementDeclared = false;
			for (const auto& vertexElement : vertexElements)
			{
				if (
					vertexInputNode->getDataUsage() == vertexElement.getDataUsage() &&
					vertexInputNode->getIndex() == vertexElement.getIndex()
				)
				{
					if (elementDataType != vertexElement.getDataType())
						log::warning << L"Identical vertex input usage but different types (" << render::getDataTypeName(elementDataType) << L" and " << render::getDataTypeName(vertexElement.getDataType()) << L")" << Endl;
					elementDeclared = true;
					break;
				}
			}
			if (!elementDeclared)
			{
				render::VertexElement element(
					vertexInputNode->getDataUsage(),
					elementDataType,
					vertexElementOffset,
					vertexInputNode->getIndex()
				);
				vertexElements.push_back(element);
				vertexElementOffset += element.getSize();
			}
		}
	}

	// Merge all shader technique fragments into a single material shader.
	Ref< render::ShaderGraph > materialShaderGraph = new render::ShaderGraph();
	for (std::map< uint32_t, Ref< render::ShaderGraph > >::iterator i = materialTechniqueShaderGraphs.begin(); i != materialTechniqueShaderGraphs.end(); ++i)
	{
		Ref< render::ShaderGraph > materialTechniqueShaderGraph = DeepClone(i->second).create< render::ShaderGraph >();
		for (auto node : materialTechniqueShaderGraph->getNodes())
			materialShaderGraph->addNode(node);
		for (auto edge : materialTechniqueShaderGraph->getEdges())
			materialShaderGraph->addEdge(edge);
	}

	// Build material shader.
	std::wstring materialPath = Path(outputPath).getPathOnly() + L"/" + outputGuid.format() + L"/Shader";
	if (!pipelineBuilder->buildAdHocOutput(
		materialShaderGraph,
		materialPath,
		materialGuid
	))
	{
		log::error << L"Mesh pipeline failed; unable to build material shader." << Endl;
		return false;
	}

	// Create render mesh.
	Ref< IMeshResource > resource = converter->createResource();
	if (!resource)
	{
		log::error << L"Mesh pipeline failed; unable to create mesh resource." << Endl;
		return false;
	}

	log::info << L"Creating mesh resource \"" << type_name(resource) << L"\"..." << Endl;

	// Create output instance.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Mesh pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	// Open asset data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Mesh pipeline failed; unable to create mesh data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	int64_t dataSize = stream->tell();

	// Convert mesh asset.
	if (!converter->convert(
		asset,
		models,
		materialGuid,
		materialTechniqueMap,
		vertexElements,
		maxInstanceCount,
		resource,
		stream
	))
	{
		log::error << L"Mesh pipeline failed; unable to convert mesh." << Endl;
		return false;
	}

	dataSize = stream->tell() - dataSize;
	stream->close();

	// Commit resource.
	outputInstance->setObject(resource);
	if (!outputInstance->commit())
	{
		log::error << L"Mesh pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > MeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

render::IProgramCompiler* MeshPipeline::getProgramCompiler() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_programCompilerLock);

	if (m_programCompiler)
		return m_programCompiler;

	const TypeInfo* programCompilerType = TypeInfo::find(m_programCompilerTypeName.c_str());
	if (!programCompilerType)
	{
		log::error << L"Mesh pipeline; unable to find program compiler type \"" << m_programCompilerTypeName << L"\"." << Endl;
		return nullptr;
	}

	m_programCompiler = dynamic_type_cast< render::IProgramCompiler* >(programCompilerType->createInstance());
	if (!m_programCompiler)
	{
		log::error << L"Mesh pipeline; unable to instanciate program compiler \"" << m_programCompilerTypeName << L"\"." << Endl;
		return nullptr;
	}

	return m_programCompiler;
}

	}
}
