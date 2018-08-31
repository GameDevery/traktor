/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundSettingsPage_H
#define traktor_sound_SoundSettingsPage_H

#include "Editor/ISettingsPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edit;

		namespace custom
		{

class DropDown;

		}
	}

	namespace sound
	{

/*! \brief Editor sound settings page.
 * \ingroup Sound
 */
class T_DLLCLASS SoundSettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool apply(PropertyGroup* settings) T_OVERRIDE T_FINAL;

private:
	Ref< ui::custom::DropDown > m_dropSoundDriver;
	Ref< ui::Edit > m_editVirtualChannels;
	Ref< ui::custom::DropDown > m_dropSampleRate;
	Ref< ui::custom::DropDown > m_dropBitsPerSample;
	Ref< ui::custom::DropDown > m_dropHwChannels;
	Ref< ui::Edit > m_editFrameSamples;
	Ref< ui::Edit > m_editMixerFrames;
};

	}
}

#endif	// traktor_sound_SoundSettingsPage_H
