#include "Amalgam/IEnvironment.h"
#include "Amalgam/Impl/LibraryHelper.h"
#include "Amalgam/Impl/PhysicsServer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Physics/MeshFactory.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/EntityFactory.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.PhysicsServer", PhysicsServer, IPhysicsServer)

bool PhysicsServer::create(const PropertyGroup* settings, float simulationDeltaTime)
{
	std::wstring physicsType = settings->getProperty< PropertyString >(L"Physics.Type");

	Ref< physics::PhysicsManager > physicsManager = loadAndInstantiate< physics::PhysicsManager >(physicsType);
	if (!physicsManager)
		return false;

	if (!physicsManager->create(simulationDeltaTime))
	{
		log::error << L"Physics server failed; unable to create physics manager" << Endl;
		return false;
	}

	m_physicsManager = physicsManager;
	return true;
}

void PhysicsServer::destroy()
{
	safeDestroy(m_physicsManager);
}

void PhysicsServer::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	db::Database* database = environment->getDatabase();

	resourceManager->addFactory(new physics::MeshFactory(database));
}

void PhysicsServer::createEntityFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	world::IEntityBuilder* entityBuilder = environment->getWorld()->getEntityBuilder();

	entityBuilder->addFactory(new physics::EntityFactory(resourceManager, m_physicsManager));
}

int32_t PhysicsServer::reconfigure(const PropertyGroup* settings)
{
	return CrUnaffected;
}

void PhysicsServer::update()
{
	m_physicsManager->update();
}

physics::PhysicsManager* PhysicsServer::getPhysicsManager()
{
	return m_physicsManager;
}

	}
}
