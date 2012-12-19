#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Resound/SimultaneousGrain.h"
#include "Sound/Resound/SimultaneousGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SimultaneousGrainData", 0, SimultaneousGrainData, IGrainData)

Ref< IGrain > SimultaneousGrainData::createInstance(resource::IResourceManager* resourceManager) const
{
	RefArray< IGrain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i] = m_grains[i]->createInstance(resourceManager);
		if (!grains[i])
			return 0;
	}

	return new SimultaneousGrain(grains);
}

bool SimultaneousGrainData::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IGrainData >(L"grains", m_grains);
}

	}
}
