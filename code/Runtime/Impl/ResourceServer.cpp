#include "Ai/NavMeshFactory.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/Impl/ResourceServer.h"
#include "Animation/AnimationResourceFactory.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Spark/MovieResourceFactory.h"
#include "Heightfield/HeightfieldFactory.h"
#include "Mesh/MeshFactory.h"
#include "Spray/EffectFactory.h"
#include "Video/VideoFactory.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "Resource/ResourceManager.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ResourceServer", ResourceServer, IResourceServer)

bool ResourceServer::create(const PropertyGroup* settings, db::Database* database)
{
	m_resourceManager = new resource::ResourceManager(database, settings->getProperty< bool >(L"Resource.Verbose", false));
	return true;
}

void ResourceServer::destroy()
{
	safeDestroy(m_resourceManager);
}

void ResourceServer::createResourceFactories(IEnvironment* environment)
{
	render::IRenderSystem* renderSystem = environment->getRender()->getRenderSystem();

	m_resourceManager->addFactory(new ai::NavMeshFactory());
	m_resourceManager->addFactory(new animation::AnimationResourceFactory());
	m_resourceManager->addFactory(new mesh::MeshFactory(renderSystem));
	m_resourceManager->addFactory(new hf::HeightfieldFactory());
	m_resourceManager->addFactory(new video::VideoFactory(renderSystem));
	m_resourceManager->addFactory(new weather::CloudMaskFactory());

	if (environment->getWorld())
	{
		const world::IEntityBuilder* entityBuilder = environment->getWorld()->getEntityBuilder();
		m_resourceManager->addFactory(new spray::EffectFactory(entityBuilder));
	}
}

int32_t ResourceServer::reconfigure(const PropertyGroup* settings)
{
	return CrUnaffected;
}

void ResourceServer::performCleanup()
{
	m_resourceManager->unloadUnusedResident();
}

resource::IResourceManager* ResourceServer::getResourceManager()
{
	return m_resourceManager;
}

	}
}
