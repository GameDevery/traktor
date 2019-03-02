#pragma once

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief WebBrowser interface.
 * \ingroup UI
 */
class IWebBrowser : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& url) = 0;

	virtual void navigate(const std::wstring& url) = 0;

	virtual void forward() = 0;

	virtual void back() = 0;

	virtual void reload(bool forced) = 0;

	virtual bool ready() const = 0;
};

	}
}

