#pragma once

#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{

class IWidget;
class ISystemBitmap;

/*! \brief NotificationIcon interface.
 * \ingroup UI
 */
class INotificationIcon
{
public:
	virtual bool create(const std::wstring& text, ISystemBitmap* image) = 0;

	virtual void destroy() = 0;

	virtual void setImage(ISystemBitmap* image) = 0;
};

	}
}

