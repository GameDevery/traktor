#ifndef traktor_ui_Window_H
#define traktor_ui_Window_H

#define _WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(WINCE)
#include <aygshell.h>
#endif
#include <map>

namespace traktor
{
	namespace ui
	{

#define WM_REFLECTED_COMMAND	(WM_USER + 1000)
#define WM_REFLECTED_NOTIFY		(WM_USER + 1001)
#define WM_REFLECTED_HSCROLL	(WM_USER + 1002)
#define WM_REFLECTED_VSCROLL	(WM_USER + 1003)

struct IMessageHandler
{
	virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass) = 0;
};

template < typename ClassType >
struct MethodMessageHandler : public IMessageHandler
{
	typedef LRESULT (ClassType::*MethodType)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	ClassType* m_object;
	MethodType m_method;

	MethodMessageHandler(ClassType* object, MethodType method) :
		m_object(object),
		m_method(method)
	{
	}

	virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
	{
		return (m_object->*m_method)(hWnd, message, wParam, lParam, pass);
	}
};

class Window
{
public:
	Window();

	virtual ~Window();

	bool create(
		HWND hParentWnd,
		LPCTSTR className,
		LPCTSTR text,
		UINT style,
		UINT styleEx,
		int left,
		int top,
		int width,
		int height,
		int id = 0,
		bool subClass = false
	);

	bool subClass(HWND hWnd);

	void setFont(HFONT hFont);

	HFONT getFont() const;

	LRESULT sendMessage(UINT message, WPARAM wParam, LPARAM lParam) const;

	operator HWND () const;

	void registerMessageHandler(UINT message, IMessageHandler* messageHandler);

	static void registerDefaultClass();

	static void unregisterDefaultClass();

	static void registerDialogClass();

	static void unregisterDialogClass();

private:
	HWND m_hWnd;
	HFONT m_hFont;
	WNDPROC m_originalWndProc;
	std::map< UINT, IMessageHandler* > m_messageHandlers;

	static LRESULT invokeMessageHandlers(HWND hWnd, DWORD dwIndex, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK wndProcSubClass(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

	}
}

#endif	// traktor_ui_Window_H
