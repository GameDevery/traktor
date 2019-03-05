#include "Amalgam/App/Win32/ErrorDialog.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/Image.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/LogList/LogList.h"

// Resources
#include "Resources/Error.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ErrorDialog", ErrorDialog, ui::Dialog)

bool ErrorDialog::create()
{
	if (!ui::Dialog::create(
		0,
		L"Error",
		ui::dpi96(700),
		ui::dpi96(450),
		ui::Dialog::WsDefaultResizable,
		new ui::TableLayout(L"*,100%", L"100%", 0, 0)
	))
		return false;

	Ref< ui::Image > imageError = new ui::Image();
	imageError->create(this, ui::Bitmap::load(c_ResourceError, sizeof(c_ResourceError), L"image"), ui::Image::WsTransparent);

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 4, 4));

	Ref< ui::Container > containerText = new ui::Container();
	containerText->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 4));

	Ref< ui::Static > staticText = new ui::Static();
	staticText->create(containerText, L"Unfortunately there has been an error");

	m_listLog = new ui::LogList();
	m_listLog->create(containerText, ui::WsNone, 0);

	Ref< ui::Container > containerButtons = new ui::Container();
	containerButtons->create(container, ui::WsNone, new ui::TableLayout(L"100%,*,*,*", L"*", 0, 4));

	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerButtons, L"");

	Ref< ui::Button > buttonCopy = new ui::Button();
	buttonCopy->create(containerButtons, L"Copy to clipboard");
	buttonCopy->addEventHandler< ui::ButtonClickEvent >(this, &ErrorDialog::eventButtonClickCopy);

	Ref< ui::Button > buttonUpload = new ui::Button();
	buttonUpload->create(containerButtons, L"Upload crash dump");
	buttonUpload->addEventHandler< ui::ButtonClickEvent >(this, &ErrorDialog::eventButtonClickUpload);
	buttonUpload->setEnable(false);

	Ref< ui::Button > buttonQuit = new ui::Button();
	buttonQuit->create(containerButtons, L"Quit");
	buttonQuit->addEventHandler< ui::ButtonClickEvent >(this, &ErrorDialog::eventButtonClickQuit);

	return true;
}

void ErrorDialog::addErrorString(const std::wstring& errorString)
{
	m_listLog->add(0, ui::LogList::LvInfo, errorString);
}

void ErrorDialog::eventButtonClickCopy(ui::ButtonClickEvent* event)
{
	m_listLog->copyLog();
}

void ErrorDialog::eventButtonClickUpload(ui::ButtonClickEvent* event)
{
}

void ErrorDialog::eventButtonClickQuit(ui::ButtonClickEvent* event)
{
	endModal(0);
}

	}
}
