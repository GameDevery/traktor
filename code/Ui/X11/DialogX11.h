#pragma once

#include "Ui/Itf/IDialog.h"
#include "Ui/X11/WidgetX11Impl.h"

namespace traktor
{
	namespace ui
	{

class DialogX11 : public WidgetX11Impl< IDialog >
{
public:
	DialogX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) override final;

	virtual void destroy() override final;

	virtual void setIcon(ISystemBitmap* icon) override final;

	virtual int showModal() override final;

	virtual void endModal(int result) override final;

	virtual void setMinSize(const Size& minSize) override final;

	virtual void setText(const std::wstring& text) override final;

	virtual void setVisible(bool visible) override final;

private:
	Atom m_atomWmDeleteWindow;
	Window m_parent;
	int32_t m_result;
	bool m_modal;
};

	}
}

