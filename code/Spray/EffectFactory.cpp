#include "Core/Io/Reader.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Spray/Effect.h"
#include "Spray/EffectData.h"
#include "Spray/EffectFactory.h"
#include "Spray/PointSet.h"
#include "Spray/PointSetResource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectFactory", EffectFactory, resource::IResourceFactory)

EffectFactory::EffectFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet EffectFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Effect >());
	typeSet.insert(&type_of< PointSet >());
	return typeSet;
}

bool EffectFactory::isCacheable() const
{
	return true;
}

Ref< Object > EffectFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	if (is_type_a< Effect >(resourceType))
	{
		Ref< EffectData > effectData = m_db->getObjectReadOnly< EffectData >(guid);
		if (effectData)
			return effectData->createEffect(resourceManager);
		else
			return 0;
	}
	else if (is_type_a< PointSet >(resourceType))
	{
		Ref< db::Instance > instance = m_db->getInstance(guid);
		if (!instance)
			return 0;
	
		Ref< IStream > stream = instance->readData(L"Data");
		if (!stream)
			return 0;
			
		Ref< PointSet > pointSet = new PointSet();
		if (!pointSet->read(stream))
			return 0;

		return pointSet;
	}
	else
		return 0;
}

	}
}
