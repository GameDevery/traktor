/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphData.h"
#include "Render/Image2/ImageGraphFactory.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageGraphFactory", ImageGraphFactory, resource::IResourceFactory)

ImageGraphFactory::ImageGraphFactory(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ImageGraphFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ImageGraphData >();
}

const TypeInfoSet ImageGraphFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< ImageGraph >();
}

bool ImageGraphFactory::isCacheable(const TypeInfo& productType) const
{
	return false;
}

Ref< Object > ImageGraphFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< ImageGraphData > imageGraphData = instance->getObject< ImageGraphData >();
    if (imageGraphData)
        return imageGraphData->createInstance(resourceManager, m_renderSystem);
    else
        return nullptr;
}

}
