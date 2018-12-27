/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundCategory_H
#define traktor_sound_SoundCategory_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundCategory : public ISerializable
{
	T_RTTI_CLASS;

public:
	SoundCategory();

	virtual void serialize(ISerializer& s) override final;

	const Guid& getParent() const { return m_parent; }

	const std::wstring& getConfigurationId() const { return m_configurationId; }

	float getGain() const { return m_gain; }

	float getPresence() const { return m_presence; }

	float getPresenceRate() const { return m_presenceRate; }

	float getRange() const { return m_range; }

private:
	Guid m_parent;
	std::wstring m_configurationId;
	float m_gain;
	float m_presence;
	float m_presenceRate;
	float m_range;
};

	}
}

#endif	// traktor_sound_SoundCategory_H
