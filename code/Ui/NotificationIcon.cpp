#include "Ui/NotificationIcon.h"
#include "Ui/Application.h"
#include "Ui/Widget.h"
#include "Ui/Bitmap.h"
#include "Ui/Itf/INotificationIcon.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NotificationIcon", NotificationIcon, EventSubject)

NotificationIcon::NotificationIcon()
:	m_ni(0)
{
}

NotificationIcon::~NotificationIcon()
{
	T_ASSERT_M (!m_ni, L"NotificationIcon not destroyed");
}

bool NotificationIcon::create(const std::wstring& text, Bitmap* image)
{
	if (!image || !image->getIBitmap())
		return false;

	m_ni = Application::getInstance()->getWidgetFactory()->createNotificationIcon(this);
	if (!m_ni)
	{
		log::error << L"Failed to create native widget peer (NotificationIcon)" << Endl;
		return false;
	}

	if (!m_ni->create(text, image->getIBitmap()))
		return false;

	return true;
}

void NotificationIcon::destroy()
{
	if (m_ni)
	{
		m_ni->destroy();
		m_ni = 0;
	}
}

void NotificationIcon::addButtonDownEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiButtonDown, eventHandler);
}

void NotificationIcon::addButtonUpEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiButtonUp, eventHandler);
}

void NotificationIcon::addDoubleClickEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiDoubleClick, eventHandler);
}

	}
}
