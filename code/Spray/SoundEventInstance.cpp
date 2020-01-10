#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spray/SoundEventInstance.h"
#include "World/Entity.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundEventInstance", SoundEventInstance, world::IEntityEventInstance)

SoundEventInstance::SoundEventInstance(
	world::Entity* sender,
	const Transform& Toffset,
	sound::ISoundPlayer* soundPlayer,
	const resource::Proxy< sound::Sound >& sound,
	bool positional,
	bool follow,
	bool autoStopFar
)
:	m_sender(sender)
,	m_Toffset(Toffset)
,	m_soundPlayer(soundPlayer)
,	m_sound(sound)
,	m_positional(positional)
,	m_follow(follow)
,	m_autoStopFar(autoStopFar)
{
	if (m_positional)
	{
		Transform T = Transform::identity();
		if (m_sender)
			T = m_sender->getTransform();
		m_handle = m_soundPlayer->play(m_sound, (T * m_Toffset).translation(), 16, m_autoStopFar);
	}
	else
		m_handle = m_soundPlayer->play(m_sound, 16);
}

bool SoundEventInstance::update(const world::UpdateParams& update)
{
	if (!m_handle || !m_handle->isPlaying())
	{
		m_handle = nullptr;
		return false;
	}

	if (m_positional && m_follow)
	{
		Transform T = Transform::identity();
		if (m_sender)
			T = m_sender->getTransform();
		m_handle->setPosition((T * m_Toffset).translation());
	}

	return true;
}

void SoundEventInstance::attach(world::IWorldRenderer* worldRenderer)
{
}

void SoundEventInstance::cancel(world::CancelType when)
{
	if (m_handle)
	{
		m_handle->stop();
		m_handle = nullptr;
	}
}

	}
}
