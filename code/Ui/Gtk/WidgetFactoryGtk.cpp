#include "Ui/Gtk/WidgetFactoryGtk.h"
#include "Ui/Gtk/ButtonGtk.h"
#include "Ui/Gtk/ContainerGtk.h"
#include "Ui/Gtk/DialogGtk.h"
#include "Ui/Gtk/FormGtk.h"
#include "Ui/Gtk/ListBoxGtk.h"
#include "Ui/Gtk/MenuBarGtk.h"

namespace traktor
{
	namespace ui
	{

IButton* WidgetFactoryGtk::createButton(EventSubject* owner)
{
	return new ButtonGtk(owner);
}

ICheckBox* WidgetFactoryGtk::createCheckBox(EventSubject* owner)
{
	return 0;
}

IComboBox* WidgetFactoryGtk::createComboBox(EventSubject* owner)
{
	return 0;
}

IContainer* WidgetFactoryGtk::createContainer(EventSubject* owner)
{
	return new ContainerGtk(owner);
}

IDialog* WidgetFactoryGtk::createDialog(EventSubject* owner)
{
	return new DialogGtk(owner);
}

IDropDown* WidgetFactoryGtk::createDropDown(EventSubject* owner)
{
	return 0;
}

IEdit* WidgetFactoryGtk::createEdit(EventSubject* owner)
{
	return 0;
}

IFileDialog* WidgetFactoryGtk::createFileDialog(EventSubject* owner)
{
	return 0;
}

IForm* WidgetFactoryGtk::createForm(EventSubject* owner)
{
	return new FormGtk(owner);
}

IListBox* WidgetFactoryGtk::createListBox(EventSubject* owner)
{
	return new ListBoxGtk(owner);
}

IListView* WidgetFactoryGtk::createListView(EventSubject* owner)
{
	return 0;
}

IMenuBar* WidgetFactoryGtk::createMenuBar(EventSubject* owner)
{
	return new MenuBarGtk();
}

IMessageBox* WidgetFactoryGtk::createMessageBox(EventSubject* owner)
{
	return 0;
}

INotificationIcon* WidgetFactoryGtk::createNotificationIcon(EventSubject* owner)
{
	return 0;
}

IPanel* WidgetFactoryGtk::createPanel(EventSubject* owner)
{
	return 0;
}

IPopupMenu* WidgetFactoryGtk::createPopupMenu(EventSubject* owner)
{
	return 0;
}

IRadioButton* WidgetFactoryGtk::createRadioButton(EventSubject* owner)
{
	return 0;
}

IRichEdit* WidgetFactoryGtk::createRichEdit(EventSubject* owner)
{
	return 0;
}

IScrollBar* WidgetFactoryGtk::createScrollBar(EventSubject* owner)
{
	return 0;
}

ISlider* WidgetFactoryGtk::createSlider(EventSubject* owner)
{
	return 0;
}

IStatic* WidgetFactoryGtk::createStatic(EventSubject* owner)
{
	return 0;
}

IToolForm* WidgetFactoryGtk::createToolForm(EventSubject* owner)
{
	return 0;
}

ITreeView* WidgetFactoryGtk::createTreeView(EventSubject* owner)
{
	return 0;
}

IUserWidget* WidgetFactoryGtk::createUserWidget(EventSubject* owner)
{
	return 0;
}

INative* WidgetFactoryGtk::createNative(EventSubject* owner)
{
	return 0;
}

IBitmap* WidgetFactoryGtk::createBitmap()
{
	return 0;
}

IClipboard* WidgetFactoryGtk::createClipboard()
{
	return 0;
}

bool WidgetFactoryGtk::getSystemColor(SystemColor systemColor, Color& outColor)
{
	return false;
}

	}
}

