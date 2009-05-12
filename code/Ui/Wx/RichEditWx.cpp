#include "Ui/Wx/RichEditWx.h"

namespace traktor
{
	namespace ui
	{

RichEditWx::RichEditWx(EventSubject* owner) :
	WidgetWxImpl< IRichEdit, wxTextCtrl >(owner)
{
}

bool RichEditWx::create(IWidget* parent, int style)
{
	int wxStyle = wxTE_RICH2;
	if (style & WsClientBorder)
		wxStyle |= wxSUNKEN_BORDER;
	else if (!(style & WsBorder))
		wxStyle |= wxNO_BORDER;

	m_window = new wxTextCtrl();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		_T(""),
		wxDefaultPosition,
		wxDefaultSize,
		wxStyle
	))
	{
		m_window->Destroy();
		return false;
	}
	
	if (!WidgetWxImpl< IRichEdit, wxTextCtrl >::create(style))
		return false;

	return true;
}

int RichEditWx::addAttribute(const Color& textColor, const Color& backColor, bool bold, bool italic, bool underline)
{
	//Font font = getFont();
	//
	//font.setBold(bold);
	//font.setItalic(italic);
	//font.setUnderline(underline);
	//
	//wxTextAttr attr(
	//	wxColour(
	//		unsigned char(textColor->getRed() * 255.0f),
	//		unsigned char(textColor->getGreen() * 255.0f),
	//		unsigned char(textColor->getBlue() * 255.0f)
	//	),
	//	wxColour(
	//		unsigned char(backColor->getRed() * 255.0f),
	//		unsigned char(backColor->getGreen() * 255.0f),
	//		unsigned char(backColor->getBlue() * 255.0f)
	//	),
	//	font.getWxFont()
	//);
	//
	//m_attributes.push_back(attr);
	
	return int(m_attributes.size() - 1);
}

void RichEditWx::setAttribute(int start, int length, int attribute)
{
	m_window->SetStyle(start, start + length, m_attributes[attribute]);
}

void RichEditWx::clear(bool attributes, bool content)
{
	if (attributes)
		m_attributes.clear();
	if (content)
		setText(L"");
}

void RichEditWx::insert(const std::wstring& text)
{
	m_window->WriteText(wstots(text).c_str());
}

int RichEditWx::getCaretOffset() const
{
	return m_window->GetInsertionPoint();
}

int RichEditWx::getLineFromOffset(int offset) const
{
	long line;
	
	if (!m_window->PositionToXY(offset, 0, &line))
		return 0;
		
	return line;
}

int RichEditWx::getLineCount() const
{
	return m_window->GetNumberOfLines();
}

int RichEditWx::getLineOffset(int line) const
{
	return m_window->XYToPosition(0, line);
}

int RichEditWx::getLineLength(int line) const
{
	return m_window->GetLineLength(line);
}

std::wstring RichEditWx::getLine(int line) const
{
	return tstows((const wxChar *)m_window->GetLineText(line));
}

bool RichEditWx::redo()
{
	m_window->Redo();
	return true;
}

bool RichEditWx::undo()
{
	m_window->Undo();
	return true;
}

void RichEditWx::onUpdate(wxCommandEvent& event)
{
	CommandEvent cmdEvent(m_owner, 0);
	m_owner->raiseEvent(EiSelectionChange, &cmdEvent);
}

	}
}
