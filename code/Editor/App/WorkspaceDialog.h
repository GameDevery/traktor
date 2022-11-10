/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef traktor_editor_WorkspaceDialog_H
#define traktor_editor_WorkspaceDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;

	namespace ui
	{

class Edit;

	}

	namespace editor
	{

class WorkspaceDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, PropertyGroup* settings);

private:
	Ref< PropertyGroup > m_settings;
	Ref< ui::Edit > m_editSystemRoot;
	Ref< ui::Edit > m_editSourceDatabase;
	Ref< ui::Edit > m_editOutputDatabase;
	Ref< ui::Edit > m_editAssetPath;

	void eventDialogClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_SettingsDialog_H
