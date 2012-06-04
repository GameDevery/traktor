#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
#include "Render/Resource/FragmentLinker.h"
#include "Render/Shader/External.h"
#include "Render/Shader/ShaderGraph.h"
#include "Terrain/TerrainResource.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainPipeline.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const float c_terrainNormalScale = 0.8f;

const Guid c_guidNormalMapSeed(L"{84F74E7F-4D02-40f6-A07A-EE9F5EF3CDB4}");
const Guid c_guidHeightMapSeed(L"{EA932687-BC1E-477f-BF70-A8715991258D}");
const Guid c_guidTerrainCoarseShaderSeed(L"{6643B92A-6676-41b9-9427-3569B2EA481B}");
const Guid c_guidTerrainDetailShaderSeed(L"{1AC67694-4CF8-44ac-B78E-B1E79C9632C8}");
const Guid c_guidSurfaceShaderSeed(L"{8481FC82-A8E8-49b8-906F-9F8F6365B1F5}");

const Guid c_guidTerrainCoarseShaderTemplate_VFetch(L"{A6C4532A-0540-4D42-93FC-964C7BFDD1FD}");
const Guid c_guidTerrainDetailShaderTemplate_VFetch(L"{68565BF3-8F72-8848-8FBA-395B9699F108}");
const Guid c_guidSurfaceShaderTemplate(L"{BAD675B3-9799-7D49-A045-BDA471DD5A3E}");

const Guid c_guidSurfaceShaderPlaceholder(L"{23790224-9E2A-4C43-9C3B-F659BE962E10}");

Guid combineGuids(const Guid& g1, const Guid& g2)
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = g1[i] ^ g2[i];
	return Guid(d);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainPipeline", 1, TerrainPipeline, editor::DefaultPipeline)

TypeInfoSet TerrainPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainAsset >());
	return typeSet;
}

bool TerrainPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	const TerrainAsset* terrainAsset = checked_type_cast< const TerrainAsset*, false >(sourceAsset);

	pipelineDepends->addDependency(terrainAsset->getHeightfield(), editor::PdfUse | editor::PdfBuild);
	pipelineDepends->addDependency(terrainAsset->getSurfaceShader(), editor::PdfUse);

	pipelineDepends->addDependency(c_guidTerrainCoarseShaderTemplate_VFetch, editor::PdfUse);
	pipelineDepends->addDependency(c_guidTerrainDetailShaderTemplate_VFetch, editor::PdfUse);
	pipelineDepends->addDependency(c_guidSurfaceShaderTemplate, editor::PdfUse);
	pipelineDepends->addDependency(c_guidSurfaceShaderPlaceholder, editor::PdfUse);

	// Synthesize ids.
	Guid normalMapGuid = combineGuids(c_guidNormalMapSeed, outputGuid);
	Guid heightMapGuid = combineGuids(c_guidHeightMapSeed, outputGuid);

	// Create normal map.
	Ref< hf::HeightfieldTextureAsset > normalMapAsset = new hf::HeightfieldTextureAsset();
	normalMapAsset->m_heightfield = terrainAsset->getHeightfield();
	normalMapAsset->m_output = hf::HeightfieldTextureAsset::OtNormals;
	normalMapAsset->m_scale = 1.0f;
	pipelineDepends->addDependency(normalMapAsset, outputPath + L"/Normals", normalMapGuid, editor::PdfBuild);

	// Create height map.
	Ref< hf::HeightfieldTextureAsset > heightMapAsset = new hf::HeightfieldTextureAsset();
	heightMapAsset->m_heightfield = terrainAsset->getHeightfield();
	heightMapAsset->m_output = hf::HeightfieldTextureAsset::OtHeights;
	heightMapAsset->m_scale = 1.0f;
	pipelineDepends->addDependency(heightMapAsset, outputPath + L"/Heights", heightMapGuid, editor::PdfBuild);

	pipelineDepends->addDependency< render::ShaderGraph >();
	return true;
}

namespace
{

	class FragmentReaderAdapter : public render::FragmentLinker::FragmentReader
	{
	public:
		FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder, const render::ShaderGraph* surfaceShaderImpl)
		:	m_pipelineBuilder(pipelineBuilder)
		,	m_surfaceShaderImpl(surfaceShaderImpl)
		{
		}

		virtual Ref< const render::ShaderGraph > read(const Guid& fragmentGuid)
		{
			if (fragmentGuid == c_guidSurfaceShaderPlaceholder)
				return m_surfaceShaderImpl;
			else
				return m_pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(fragmentGuid);
		}

	private:
		Ref< editor::IPipelineBuilder > m_pipelineBuilder;
		Ref< const render::ShaderGraph > m_surfaceShaderImpl;
	};

}

bool TerrainPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const TerrainAsset* terrainAsset = checked_type_cast< const TerrainAsset*, false >(sourceAsset);

	Guid normalMapGuid = combineGuids(c_guidNormalMapSeed, outputGuid);
	Guid heightMapGuid = combineGuids(c_guidHeightMapSeed, outputGuid);
	Guid terrainCoarseShaderGuid = combineGuids(c_guidTerrainCoarseShaderSeed, outputGuid);
	Guid terrainDetailShaderGuid = combineGuids(c_guidTerrainDetailShaderSeed, outputGuid);
	Guid surfaceShaderGuid = combineGuids(c_guidSurfaceShaderSeed, outputGuid);

	// Read surface shader and prepare with proper input and output ports.
	Ref< const render::ShaderGraph > assetSurfaceShader = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(terrainAsset->getSurfaceShader());
	if (!assetSurfaceShader)
	{
		log::error << L"Terrain pipeline failed; unable to get terrain template shader" << Endl;
		return false;
	}

	Ref< render::ShaderGraph > surfaceShaderImpl = DeepClone(assetSurfaceShader).create< render::ShaderGraph >();

	// Read shader templates.
	Ref< const render::ShaderGraph > terrainCoarseShaderTemplate = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_guidTerrainCoarseShaderTemplate_VFetch);
	if (!terrainCoarseShaderTemplate)
	{
		log::error << L"Terrain pipeline failed; unable to get terrain coarse template shader" << Endl;
		return false;
	}

	Ref< const render::ShaderGraph > terrainDetailShaderTemplate = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_guidTerrainDetailShaderTemplate_VFetch);
	if (!terrainDetailShaderTemplate)
	{
		log::error << L"Terrain pipeline failed; unable to get terrain detail template shader" << Endl;
		return false;
	}

	Ref< const render::ShaderGraph > surfaceShaderTemplate = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(c_guidSurfaceShaderTemplate);
	if (!surfaceShaderTemplate)
	{
		log::error << L"Terrain pipeline failed; unable to get surface template shader" << Endl;
		return false;
	}

	// Resolve fragments in templates and insert surface shader at placeholders.
	FragmentReaderAdapter fragmentReader(pipelineBuilder, surfaceShaderImpl);

	Ref< render::ShaderGraph > terrainCoarseShader = render::FragmentLinker(fragmentReader).resolve(terrainCoarseShaderTemplate, true);
	if (!terrainCoarseShader)
	{
		log::error << L"Terrain pipeline failed; unable to link terrain coarse shader" << Endl;
		return false;
	}

	Ref< render::ShaderGraph > terrainDetailShader = render::FragmentLinker(fragmentReader).resolve(terrainDetailShaderTemplate, true);
	if (!terrainDetailShader)
	{
		log::error << L"Terrain pipeline failed; unable to link terrain detail shader" << Endl;
		return false;
	}

	Ref< render::ShaderGraph > surfaceShader = render::FragmentLinker(fragmentReader).resolve(surfaceShaderTemplate, true);
	if (!surfaceShader)
	{
		log::error << L"Terrain pipeline failed; unable to link surface shader" << Endl;
		return false;
	}

	// Build shaders.
	std::wstring shaderPath = Path(outputPath).getPathOnly() + L"/" + outputGuid.format();

	if (!pipelineBuilder->buildOutput(
		terrainCoarseShader,
		0,
		shaderPath + L"/Coarse",
		terrainCoarseShaderGuid
	))
	{
		log::error << L"Terrain pipeline failed; unable to build coarse shader" << Endl;
		return false;
	}

	if (!pipelineBuilder->buildOutput(
		terrainDetailShader,
		0,
		shaderPath + L"/Detail",
		terrainDetailShaderGuid
	))
	{
		log::error << L"Terrain pipeline failed; unable to build detail shader" << Endl;
		return false;
	}

	if (!pipelineBuilder->buildOutput(
		surfaceShader,
		0,
		shaderPath + L"/Surface",
		surfaceShaderGuid
	))
	{
		log::error << L"Terrain pipeline failed; unable to build surface shader" << Endl;
		return false;
	}

	// Create output resource.
	Ref< TerrainResource > terrainResource = new TerrainResource();
	terrainResource->m_heightfield = terrainAsset->getHeightfield();
	terrainResource->m_normalMap = resource::Id< render::ISimpleTexture >(normalMapGuid);
	terrainResource->m_heightMap = resource::Id< render::ISimpleTexture >(heightMapGuid);
	terrainResource->m_terrainCoarseShader = resource::Id< render::Shader >(terrainCoarseShaderGuid);
	terrainResource->m_terrainDetailShader = resource::Id< render::Shader >(terrainDetailShaderGuid);
	terrainResource->m_surfaceShader = resource::Id< render::Shader >(surfaceShaderGuid);

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(terrainResource);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

	}
}
