#include "Database/Instance.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/ShaderResource.h"
#include "Render/Resource/TextureLinker.h"
#include "Render/Resource/TextureProxy.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class TextureReaderAdapter : public TextureLinker::TextureReader
{
public:
	TextureReaderAdapter(resource::IResourceManager* resourceManager)
	:	m_resourceManager(resourceManager)
	{
	}

	virtual Ref< ITexture > read(const Guid& textureGuid)
	{
		resource::Proxy< ITexture > texture;
		if (m_resourceManager->bind(resource::Id< ITexture >(textureGuid), texture))
			return new TextureProxy(texture);
		else
			return (ITexture*)nullptr;
	}

private:
	resource::IResourceManager* m_resourceManager;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderFactory", ShaderFactory, resource::IResourceFactory)

ShaderFactory::ShaderFactory(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ShaderFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ShaderResource >();
}

const TypeInfoSet ShaderFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Shader >();
}

bool ShaderFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > ShaderFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< ShaderResource > shaderResource = instance->getObject< ShaderResource >();
	if (!shaderResource)
		return nullptr;

	std::wstring shaderName = instance->getName();
	Ref< Shader > shader = new Shader();

	// Create combination parameter mapping.
	for (auto parameterBit : shaderResource->getParameterBits())
		shader->m_parameterBits[getParameterHandle(parameterBit.first)] = parameterBit.second;

	// Create shader techniques.
	for (auto resourceTechnique : shaderResource->getTechniques())
	{
		std::wstring programName = shaderName + L"." + resourceTechnique.name;

		Shader::Technique& technique = shader->m_techniques[getParameterHandle(resourceTechnique.name)];
		technique.mask = resourceTechnique.mask;

		for (const auto& resourceCombination : resourceTechnique.combinations)
		{
			if (!resourceCombination.program)
				continue;

			Ref< ProgramResource > programResource = checked_type_cast< ProgramResource* >(resourceCombination.program);
			if (!programResource)
				return nullptr;

			Shader::Combination combination;
			combination.mask = resourceCombination.mask;
			combination.value = resourceCombination.value;
			combination.priority = resourceCombination.priority;
			combination.program = m_renderSystem->createProgram(programResource, programName.c_str());
			if (!combination.program)
				return nullptr;

			// Set implicit texture uniforms.
			TextureReaderAdapter textureReader(resourceManager);
			if (!TextureLinker(textureReader).link(resourceCombination, combination.program))
				return nullptr;

			// Set uniform default values.
			for (const auto& ius : resourceCombination.initializeUniformScalar)
				combination.program->setFloatParameter(getParameterHandle(ius.name), ius.value);
			for (const auto& iuv : resourceCombination.initializeUniformVector)
				combination.program->setVectorParameter(getParameterHandle(iuv.name), iuv.value);

			technique.combinations.push_back(combination);
		}
	}

	return shader;
}

	}
}
