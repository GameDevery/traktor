/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "UiKit/Editor/PreviewEditor.h"

#include "Core/Class/IRuntimeClass.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Ui/Application.h"
#include "Ui/AspectLayout.h"
#include "Ui/Container.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "UiKit/Editor/PreviewControl.h"
#include "UiKit/Editor/Scaffolding.h"

namespace traktor::uikit
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.uikit.PreviewEditor", PreviewEditor, editor::IEditorPage)

PreviewEditor::PreviewEditor(editor::IEditor* editor, editor::IDocument* document)
	: m_editor(editor)
	, m_document(document)
{
}

bool PreviewEditor::create(ui::Container* parent)
{
	Ref< Scaffolding > ws = mandatory_non_null_type_cast< Scaffolding* >(m_document->getInstance(0)->getObject());

	m_container = new ui::Container();
	m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0_ut, 0_ut));

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(m_container);
	toolBar->addItem(new ui::ToolBarButton(
		L"Debug wires",
		ui::Command(L"UiKit.ToggleDebugWires"),
		ui::ToolBarButton::BsText | ui::ToolBarButton::BsToggle));
	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >([&](ui::ToolBarButtonClickEvent* event) {
		if (event->getCommand() == L"UiKit.ToggleDebugWires")
			m_previewControl->setDebugWires(
				mandatory_non_null_type_cast< ui::ToolBarButton* >(event->getItem())->isToggled());
	});

	// Create aspect container.
	Ref< ui::Container > container = new ui::Container();
	container->create(m_container, ui::WsNone, new ui::AspectLayout(16.0f / 9.0f));

	// Create preview control.
	m_previewControl = new PreviewControl(m_editor);
	if (!m_previewControl->create(container))
		return false;
	m_previewControl->addEventHandler< ui::SizeEvent >(this, &PreviewEditor::eventPreviewSize);

	// Create status bar.
	m_statusBar = new ui::StatusBar();
	m_statusBar->create(m_container, ui::WsDoubleBuffer);
	m_statusBar->addColumn(-1);

	// Bind widget scaffolding.
	m_previewControl->setScaffolding(ws);
	return true;
}

void PreviewEditor::destroy()
{
	if (m_previewControl)
		m_previewControl->setScaffolding(nullptr);

	safeDestroy(m_previewControl);
}

bool PreviewEditor::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool PreviewEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void PreviewEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	m_previewControl->handleDatabaseEvent(database, eventId);
}

void PreviewEditor::eventPreviewSize(ui::SizeEvent* event)
{
	ui::Size innerSize = m_previewControl->getInnerRect().getSize();

	StringOutputStream ss;
	ss << innerSize.cx << L" * " << innerSize.cy;
	m_statusBar->setText(0, ss.str());
}

}
