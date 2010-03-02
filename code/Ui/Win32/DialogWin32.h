#ifndef traktor_ui_DialogWin32_H
#define traktor_ui_DialogWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IDialog.h"

namespace traktor
{
	namespace ui
	{

class DialogWin32 : public WidgetWin32Impl< IDialog >
{
public:
	DialogWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void setIcon(drawing::Image* icon);
	
	virtual int showModal();

	virtual void endModal(int result);

	virtual void setMinSize(const Size& minSize);

	virtual void setVisible(bool visible);

private:
	bool m_modal;
	Size m_minSize;
	bool m_centerDesktop;
	int32_t m_result;

#if !defined(WINCE)

	LRESULT eventSizing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

#endif

	LRESULT eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventEndModal(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_DialogWin32_H
