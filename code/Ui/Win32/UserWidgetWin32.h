#pragma once

#include "Ui/Itf/IUserWidget.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class UserWidgetWin32 : public WidgetWin32Impl< IUserWidget >
{
public:
	UserWidgetWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

private:
	LRESULT eventButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

