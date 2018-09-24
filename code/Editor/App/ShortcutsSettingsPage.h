/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_ShortcutsSettingsPage_H
#define traktor_editor_ShortcutsSettingsPage_H

#include "Editor/ISettingsPage.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{

class Button;
class GridView;
class ShortcutEdit;

	}

	namespace editor
	{

/*! \brief Interface for settings pages.
 * \ingroup Editor
 */
class ShortcutsSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool apply(PropertyGroup* settings) T_OVERRIDE T_FINAL;

private:
	Ref< const PropertyGroup > m_originalSettings;
	Ref< ui::GridView > m_gridShortcuts;
	Ref< ui::ShortcutEdit > m_editShortcut;
	Ref< ui::Button > m_buttonResetAll;

	void updateShortcutGrid();

	void eventShortcutSelect(ui::SelectionChangeEvent* event);

	void eventShortcutModified(ui::ContentChangeEvent* event);

	void eventResetAll(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_ShortcutsSettingsPage_H
