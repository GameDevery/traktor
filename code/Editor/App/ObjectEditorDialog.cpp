#include "Core/Io/StringOutputStream.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Instance.h"
#include "Editor/IObjectEditor.h"
#include "Editor/App/ObjectEditorDialog.h"
#include "I18N/Text.h"
#include "Ui/MessageBox.h"
#include "Ui/FloodLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.ObjectEditorDialog", ObjectEditorDialog, ui::ConfigDialog)

ObjectEditorDialog::ObjectEditorDialog(PropertyGroup* settings, IObjectEditor* objectEditor)
:	m_settings(settings)
,	m_objectEditor(objectEditor)
{
	T_ASSERT (m_objectEditor);
}

bool ObjectEditorDialog::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	ui::Size preferredSize = m_objectEditor->getPreferredSize();

	// Get instance's editor dimensions from settings.
	int32_t width = m_settings->getProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + instance->getGuid().format() + L"/Width", preferredSize.cx);
	int32_t height = m_settings->getProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + instance->getGuid().format() + L"/Height", preferredSize.cy);

	StringOutputStream ss;
	ss << L"Edit \"" << instance->getName() << L"\" (" << type_name(object) << L")";

	if (!ui::ConfigDialog::create(
		parent,
		ss.str(),
		width,
		height,
		ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton,
		new ui::FloodLayout()
	))
		return false;

	addClickEventHandler(ui::createMethodHandler(this, &ObjectEditorDialog::eventClick));
	addCloseEventHandler(ui::createMethodHandler(this, &ObjectEditorDialog::eventClose));

	m_instance = instance;
	m_object = object;

	if (!m_objectEditor->create(this, instance, object))
		return false;

	update();

	return true;
}

void ObjectEditorDialog::destroy()
{
	// Remember instance's editor dimensions in settings.
	if (m_settings && m_instance)
	{
		ui::Rect rc = getRect();
		m_settings->setProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + m_instance->getGuid().format() + L"/Width", rc.getWidth());
		m_settings->setProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + m_instance->getGuid().format() + L"/Height", rc.getHeight());
	}

	if (m_objectEditor)
	{
		m_objectEditor->destroy();
		m_objectEditor = 0;
	}

	ui::ConfigDialog::destroy();
}

bool ObjectEditorDialog::apply(bool keep)
{
	m_objectEditor->apply();
	if (m_instance->commit(keep ? db::CfKeepCheckedOut : db::CfDefault))
		return true;
	else
	{
		ui::MessageBox::show(this, i18n::Text(L"OBJECTEDITOR_ERROR_UNABLE_TO_COMMIT_MESSAGE"), i18n::Text(L"OBJECTEDITOR_ERROR_UNABLE_TO_COMMIT_CAPTION"), ui::MbIconError | ui::MbOk);
		return false;
	}
}

void ObjectEditorDialog::cancel()
{
	destroy();
	if (m_instance)
	{
		m_instance->revert();
		m_instance = 0;
	}
}

bool ObjectEditorDialog::handleCommand(const ui::Command& command)
{
	return m_objectEditor->handleCommand(command);
}

void ObjectEditorDialog::eventClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	switch (command.getId())
	{
	case ui::DrApply:
		apply(true);
		break;

	case ui::DrOk:
		if (apply(false))
			destroy();
		break;

	case ui::DrCancel:
		cancel();
		break;
	}
}

void ObjectEditorDialog::eventClose(ui::Event* event)
{
	cancel();
	event->consume();
}

	}
}
