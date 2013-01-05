#ifndef traktor_ui_FormWin32_H
#define traktor_ui_FormWin32_H

#include "Ui/Itf/IForm.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

class MenuBarWin32;

/*! \brief
 * \ingroup UIW32
 */
class FormWin32 : public WidgetWin32Impl< IForm >
{
public:
	FormWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void destroy();

	virtual void setIcon(IBitmap* icon);

	virtual void maximize();

	virtual void minimize();

	virtual void restore();

	virtual bool isMaximized() const;

	virtual bool isMinimized() const;

	/*! \brief Bar registration.
	 *
	 * As some widgets need to be notified about
	 * parent form's size we need to register those widgets
	 * explicitly.
	 * The form also needs to know about these widgets
	 * in order to calculate it's inner size.
	 */
	//@{

#if !defined(WINCE)

	void registerMenuBar(MenuBarWin32* menuBar);

	void unregisterMenuBar(MenuBarWin32* menuBar);

#endif

	//@}

private:
#if !defined(WINCE)
	MenuBarWin32* m_menuBar;
#else
	HWND m_hWndMenuBar;
#endif

	LRESULT eventInitMenuPopup(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_FormWin32_H
