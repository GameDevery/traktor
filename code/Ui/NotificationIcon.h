#pragma once

#include "Ui/Associative.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Widget;
class IBitmap;
class INotificationIcon;

/*! Notification icon.
 * \ingroup UI
 */
class T_DLLCLASS NotificationIcon
:	public EventSubject
,	public Associative
{
	T_RTTI_CLASS;

public:
	NotificationIcon();

	virtual ~NotificationIcon();

	bool create(const std::wstring& text, IBitmap* image);

	void destroy();

	void setImage(IBitmap* image);

private:
	INotificationIcon* m_ni;
};

	}
}

