#include "Editor/App/TextEditorDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleBitmap.h"
#include "Ui/RichEdit/RichEdit.h"

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

	m_edit = new ui::RichEdit();
	if (!m_edit->create(this, initialText, ui::WsDoubleBuffer))
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
