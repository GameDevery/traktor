#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterData.h"
#include "Spray/SourceData.h"
#include "Spray/ModifierData.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EmitterData", 1, EmitterData, ISerializable)

EmitterData::EmitterData()
:	m_middleAge(0.2f)
,	m_cullNearDistance(0.25f)
,	m_fadeNearRange(1.0f)
,	m_warmUp(0.0f)
,	m_sort(false)
{
}

Ref< Emitter > EmitterData::createEmitter(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	Ref< Source > source = m_source->createSource(resourceManager);
	if (!source)
		return 0;

	RefArray< Modifier > modifiers;
	for (RefArray< ModifierData >::const_iterator i = m_modifiers.begin(); i != m_modifiers.end(); ++i)
	{
		Ref< Modifier > modifier = (*i)->createModifier(resourceManager);
		if (modifier)
			modifiers.push_back(modifier);
		else
			return 0;
	}

	return new Emitter(
		source,
		modifiers,
		shader,
		m_middleAge,
		m_cullNearDistance,
		m_fadeNearRange,
		m_warmUp,
		m_sort
	);
}

bool EmitterData::serialize(ISerializer& s)
{
	s >> MemberRef< SourceData >(L"source", m_source);
	s >> MemberRefArray< ModifierData >(L"modifiers", m_modifiers);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< float >(L"middleAge", m_middleAge);
	s >> Member< float >(L"cullNearDistance", m_cullNearDistance);
	s >> Member< float >(L"fadeNearRange", m_fadeNearRange);
	s >> Member< float >(L"warmUp", m_warmUp);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"sort", m_sort);

	return true;
}

	}
}
