#ifndef traktor_ui_DialogWx_H
#define traktor_ui_DialogWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IDialog.h"

namespace traktor
{
	namespace ui
	{

class DialogWx : public WidgetWxImpl< IDialog, wxDialog >
{
public:
	DialogWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void setIcon(drawing::Image* icon);
	
	virtual int showModal();

	virtual void endModal(int result);

	virtual void setMinSize(const Size& minSize);
};

	}
}

#endif	// traktor_ui_DialogWx_H
