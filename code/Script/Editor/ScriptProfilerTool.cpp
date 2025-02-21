/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Script/Editor/ScriptProfilerTool.h"
#include "Script/Editor/ScriptProfilerDialog.h"
#include "I18N/Text.h"
#include "Ui/StyleBitmap.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptProfilerTool", 0, ScriptProfilerTool, IEditorTool)

std::wstring ScriptProfilerTool::getDescription() const
{
	return i18n::Text(L"SCRIPT_PROFILER_TOOL");
}

Ref< ui::IBitmap > ScriptProfilerTool::getIcon() const
{
	return new ui::StyleBitmap(L"Script.Tool.Profiler");
}

bool ScriptProfilerTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool ScriptProfilerTool::launch(ui::Widget* parent, editor::IEditor* script, const PropertyGroup* param)
{
	// If profiler dialog is already created then we only need to show it.
	if (m_profilerDialog)
	{
		m_profilerDialog->show();
		return true;
	}

	// Create dialog and register ourself for close events.
	m_profilerDialog = new ScriptProfilerDialog(script);
	if (m_profilerDialog->create(parent))
	{
		m_profilerDialog->show();
		m_profilerDialog->addEventHandler< ui::CloseEvent >(this, &ScriptProfilerTool::eventCloseDialog);
	}
	else
		safeDestroy(m_profilerDialog);

	return true;
}

void ScriptProfilerTool::eventCloseDialog(ui::CloseEvent* event)
{
	safeDestroy(m_profilerDialog);
}

}
