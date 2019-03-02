#pragma once

#include <list>
#include <string>
#include "Core/Config.h"
#include "Ui/Enums.h"
#include "Ui/Rect.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class IContainer;
class IDialog;
class IEventLoop;
class IForm;
class INotificationIcon;
class IPathDialog;
class IToolForm;
class IUserWidget;
class IWebBrowser;
class ISystemBitmap;
class IClipboard;

/*! \brief Widget factory interface.
 * \ingroup UI
 */
class IWidgetFactory
{
public:
	virtual ~IWidgetFactory() {}

	virtual IEventLoop* createEventLoop(EventSubject* owner) = 0;

	virtual IContainer* createContainer(EventSubject* owner) = 0;

	virtual IDialog* createDialog(EventSubject* owner) = 0;

	virtual IForm* createForm(EventSubject* owner) = 0;

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner) = 0;

	virtual IPathDialog* createPathDialog(EventSubject* owner) = 0;

	virtual IToolForm* createToolForm(EventSubject* owner) = 0;

	virtual IUserWidget* createUserWidget(EventSubject* owner) = 0;

	virtual IWebBrowser* createWebBrowser(EventSubject* owner) = 0;

	virtual ISystemBitmap* createBitmap() = 0;

	virtual IClipboard* createClipboard() = 0;

	virtual int32_t getSystemDPI() const = 0;

	virtual void getSystemFonts(std::list< std::wstring >& outFonts) = 0;

	virtual void getDesktopRects(std::list< Rect >& outRects) const = 0;
};

	}
}

