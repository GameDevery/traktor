/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Editor/App/TextEditorDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/RichEdit/RichEdit.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.TextEditorDialog", TextEditorDialog, ui::ConfigDialog)

bool TextEditorDialog::create(ui::Widget* parent, const std::wstring& initialText)
{
	if (!ui::ConfigDialog::create(parent, i18n::Text(L"TEXT_EDIT"), ui::dpi96(500), ui::dpi96(400), ui::ConfigDialog::WsDefaultResizable, new ui::FloodLayout()))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	m_edit = new ui::custom::RichEdit();
	if (!m_edit->create(this, initialText, ui::WsClientBorder))
		return false;

	m_edit->setFont(ui::Font(L"Courier New", 14));

	update();

	return true;
}

std::wstring TextEditorDialog::getText() const
{
	return m_edit->getText();
}

	}
}
