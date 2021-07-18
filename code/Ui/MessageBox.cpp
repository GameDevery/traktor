#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Image.h"
#include "Ui/MessageBox.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MessageBox", MessageBox, ConfigDialog)

bool MessageBox::create(Widget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	int dialogStyle = WsCenterParent | WsSystemBox | WsCloseBox | WsCaption;
	if (style & MbYesNo)
		dialogStyle |= ConfigDialog::WsYesNoButtons;

	const bool haveIcon = (bool)((style & (MbIconExclamation | MbIconHand | MbIconError | MbIconQuestion | MbIconInformation)) != 0);

	if (!ConfigDialog::create(
		parent,
		caption,
		dpi96(200),
		dpi96(100),
		dialogStyle,
		new TableLayout(haveIcon ? L"*,*" : L"*", L"*", dpi96(16), dpi96(16))
	))
		return false;

	if (haveIcon)
	{
		Ref< IBitmap > icon;

		if ((style & MbIconExclamation) != 0)
			icon = new StyleBitmap(L"UI.IconExclamation");
		else if ((style & MbIconHand) != 0)
			icon = new StyleBitmap(L"UI.IconHand");
		else if ((style & MbIconError) != 0)
			icon = new StyleBitmap(L"UI.IconError");
		else if ((style & MbIconQuestion) != 0)
			icon = new StyleBitmap(L"UI.IconQuestion");
		else if ((style & MbIconInformation) != 0)
			icon = new StyleBitmap(L"UI.IconInformation");

		T_ASSERT(icon != nullptr);

		Ref< Image > image = new Image();
		image->create(this, icon, Image::WsTransparent);
		image->setVerticalAlign(AnCenter);
	}

	Ref< Static > staticMessage = new Static();
	staticMessage->create(this, message);
	staticMessage->setVerticalAlign(AnCenter);

	fit(Container::FaBoth);
	return true;
}

int MessageBox::show(Widget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	MessageBox mb;

	if (!mb.create(parent, message, caption, style))
		return DrCancel;

	int result = mb.showModal();

	mb.destroy();
	return result;
}

int MessageBox::show(const std::wstring& message, const std::wstring& caption, int style)
{
	return show(nullptr, message, caption, style);
}

	}
}
