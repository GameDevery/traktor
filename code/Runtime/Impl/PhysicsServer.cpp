/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Runtime/Impl/PhysicsServer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Timer/Profiler.h"
#include "Physics/PhysicsManager.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityBuilder.h"

namespace traktor::runtime
{
	namespace
	{

const float c_timeScale = 1.25f;	//< Make simulation 25% faster than normal; empirically measured to make simulation feel more natural.

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.PhysicsServer", PhysicsServer, IPhysicsServer)

bool PhysicsServer::create(const PropertyGroup* defaultSettings, const PropertyGroup* settings)
{
	const std::wstring physicsType = defaultSettings->getProperty< std::wstring >(L"Physics.Type");

	Ref< physics::PhysicsManager > physicsManager = dynamic_type_cast< physics::PhysicsManager* >(TypeInfo::createInstance(physicsType.c_str()));
	if (!physicsManager)
		return false;

	physics::PhysicsCreateDesc pcd;
	pcd.timeScale = defaultSettings->getProperty< float >(L"Physics.TimeScale", 1.0f) * c_timeScale;
	pcd.simulationFrequency = defaultSettings->getProperty< float >(L"Physics.SimulationFrequency", 240.0f);
	pcd.solverIterations = defaultSettings->getProperty< int32_t >(L"Physics.SolverIterations", 10);
	if (!physicsManager->create(pcd))
	{
		log::error << L"Physics server failed; unable to create physics manager." << Endl;
		return false;
	}

	m_physicsManager = physicsManager;
	return true;
}

void PhysicsServer::destroy()
{
	safeDestroy(m_physicsManager);
}

int32_t PhysicsServer::reconfigure(const PropertyGroup* settings)
{
	return CrUnaffected;
}

void PhysicsServer::update(float simulationDeltaTime)
{
	T_PROFILER_SCOPE(L"PhysicsServer update");
	m_physicsManager->update(simulationDeltaTime, true);
}

physics::PhysicsManager* PhysicsServer::getPhysicsManager()
{
	return m_physicsManager;
}

}
