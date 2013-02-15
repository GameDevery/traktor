#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Theater/TrackData.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TrackData", 2, TrackData, ISerializable)

TrackData::TrackData()
:	m_loopStart(0.0f)
,	m_loopEnd(0.0f)
,	m_loopEase(0.0f)
{
}

void TrackData::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< world::EntityData > TrackData::getEntityData() const
{
	return m_entityData;
}

void TrackData::setLookAtEntityData(world::EntityData* entityData)
{
	m_lookAtEntityData = entityData;
}

Ref< world::EntityData > TrackData::getLookAtEntityData() const
{
	return m_lookAtEntityData;
}

const TransformPath& TrackData::getPath() const
{
	return m_path;
}

TransformPath& TrackData::getPath()
{
	return m_path;
}

float TrackData::getLoopStart() const
{
	return m_loopStart;
}

float TrackData::getLoopEnd() const
{
	return m_loopEnd;
}

float TrackData::getLoopEase() const
{
	return m_loopEase;
}

bool TrackData::serialize(ISerializer& s)
{
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);

	if (s.getVersion() >= 2)
		s >> MemberRef< world::EntityData >(L"lookAtEntityData", m_lookAtEntityData);

	s >> MemberComposite< TransformPath >(L"path", m_path);
	
	if (s.getVersion() >= 1)
	{
		s >> Member< float >(L"loopStart", m_loopStart);
		s >> Member< float >(L"loopEnd", m_loopEnd);
		s >> Member< float >(L"loopEase", m_loopEase);
	}

	return true;
}

	}
}
