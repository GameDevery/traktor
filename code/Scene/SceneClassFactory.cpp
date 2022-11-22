/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Scene/ISceneController.h"
#include "Scene/Scene.h"
#include "Scene/SceneClassFactory.h"
#include "World/Entity.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneClassFactory", 0, SceneClassFactory, IRuntimeClassFactory)

void SceneClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classISceneController = new AutoRuntimeClass< ISceneController >();
	registrar->registerClass(classISceneController);

	auto classScene = new AutoRuntimeClass< Scene >();
	classScene->addProperty("rootEntity", &Scene::getRootEntity);
	classScene->addProperty("controller", &Scene::getController);
	registrar->registerClass(classScene);
}

}
