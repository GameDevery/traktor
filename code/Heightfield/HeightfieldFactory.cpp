#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFactory.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/HeightfieldResource.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldFactory", HeightfieldFactory, resource::IResourceFactory)

const TypeInfoSet HeightfieldFactory::getResourceTypes() const
{
	return makeTypeInfoSet< HeightfieldResource >();
}

const TypeInfoSet HeightfieldFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Heightfield >();
}

bool HeightfieldFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > HeightfieldFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< HeightfieldResource > resource = instance->getObject< HeightfieldResource >();
	if (!resource)
		return nullptr;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return nullptr;

	Ref< Heightfield > heightfield = HeightfieldFormat().read(stream, resource->getWorldExtent());

	stream->close();
	return heightfield;
}

	}
}
