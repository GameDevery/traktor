#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spray/EffectData.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterData.h"
#include "Spray/SourceData.h"
#include "Spray/ModifierData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EmitterData", 7, EmitterData, ISerializable)

EmitterData::EmitterData()
:	m_middleAge(0.2f)
,	m_cullNearDistance(0.25f)
,	m_cullMeshDistance(100.0f)
,	m_fadeNearRange(1.0f)
,	m_viewOffset(0.0f)
,	m_warmUp(0.0f)
,	m_sort(false)
,	m_worldSpace(true)
,	m_meshOrientationFromVelocity(true)
{
}

Ref< Emitter > EmitterData::createEmitter(resource::IResourceManager* resourceManager, const world::IEntityBuilder* entityBuilder) const
{
	if (!m_source)
		return nullptr;

	resource::Proxy< render::Shader > shader;
	resource::Proxy< mesh::InstanceMesh > mesh;

	if (
		!resourceManager->bind(m_shader, shader) &&
		!resourceManager->bind(m_mesh, mesh) &&
		!m_effect
	)
		return nullptr;

	Ref< const Source > source = m_source->createSource(resourceManager);
	if (!source)
		return nullptr;

	RefArray< const Modifier > modifiers;
	for (auto modifierData : m_modifiers)
	{
		Ref< const Modifier > modifier = modifierData->createModifier(resourceManager);
		if (modifier)
			modifiers.push_back(modifier);
	}

	Ref< const Effect > effect;
	if (m_effect)
		effect = m_effect->createEffect(resourceManager, entityBuilder);

	return new Emitter(
		source,
		modifiers,
		shader,
		mesh,
		effect,
		m_middleAge,
		m_cullNearDistance,
		m_cullMeshDistance,
		m_fadeNearRange,
		m_viewOffset,
		m_warmUp,
		m_sort,
		m_worldSpace,
		m_meshOrientationFromVelocity
	);
}

void EmitterData::serialize(ISerializer& s)
{
	s >> MemberRef< SourceData >(L"source", m_source);
	s >> MemberRefArray< ModifierData >(L"modifiers", m_modifiers);
	s >> resource::Member< render::Shader >(L"shader", m_shader);

	if (s.getVersion() >= 3)
		s >> resource::Member< mesh::InstanceMesh >(L"mesh", m_mesh);

	if (s.getVersion() >= 6)
		s >> MemberRef< EffectData >(L"effect", m_effect);

	s >> Member< float >(L"middleAge", m_middleAge);
	s >> Member< float >(L"cullNearDistance", m_cullNearDistance);

	if (s.getVersion() >= 5)
		s >> Member< float >(L"cullMeshDistance", m_cullMeshDistance);

	s >> Member< float >(L"fadeNearRange", m_fadeNearRange);

	if (s.getVersion() >= 7)
		s >> Member< float >(L"viewOffset", m_viewOffset);

	s >> Member< float >(L"warmUp", m_warmUp);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"sort", m_sort);

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"worldSpace", m_worldSpace);

	if (s.getVersion() >= 4)
		s >> Member< bool >(L"meshOrientationFromVelocity", m_meshOrientationFromVelocity);
}

	}
}
