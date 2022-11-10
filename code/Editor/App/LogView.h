/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"
#include "Ui/LogList/LogList.h"

namespace traktor
{

class ILogTarget;

	namespace ui
	{

class LogActivateEvent;
class Menu;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;

	}

	namespace editor
	{

class IEditor;

class LogView
:	public ui::Container
,	public ui::LogList::ISymbolLookup
{
	T_RTTI_CLASS;

public:
	LogView(IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	ILogTarget* getLogTarget() const { return m_logTarget; }

private:
	IEditor* m_editor;
	Ref< ui::ToolBarButton > m_toolToggleInfo;
	Ref< ui::ToolBarButton > m_toolToggleWarning;
	Ref< ui::ToolBarButton > m_toolToggleError;
	Ref< ui::ToolBar > m_toolFilter;
	Ref< ui::LogList > m_log;
	Ref< ui::Menu > m_popup;
	Ref< ILogTarget > m_logTarget;

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventLogActivate(ui::LogActivateEvent* event);

	virtual bool lookupLogSymbol(const Guid& symbolId, std::wstring& outSymbol) const override final;
};

	}
}

