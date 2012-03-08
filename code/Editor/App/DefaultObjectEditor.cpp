#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/ITypedAsset.h"
#include "Editor/TypeBrowseFilter.h"
#include "Editor/App/DefaultObjectEditor.h"
#include "I18N/Text.h"
#include "Ui/Event.h"
#include "Ui/FileDialog.h"
#include "Ui/MethodHandler.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DefaultObjectEditor", DefaultObjectEditor, IObjectEditor)

DefaultObjectEditor::DefaultObjectEditor(IEditor* editor)
:	m_editor(editor)
{
}

bool DefaultObjectEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	m_object = object;

	m_propertyList = new ui::custom::AutoPropertyList();
	m_propertyList->create(parent, ui::WsClientBorder | ui::WsDoubleBuffer | ui::custom::AutoPropertyList::WsColumnHeader, this);
	m_propertyList->addCommandEventHandler(ui::createMethodHandler(this, &DefaultObjectEditor::eventPropertyCommand));
	m_propertyList->setSeparator(200);
	m_propertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_propertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	m_propertyList->bind(m_object, 0);

	return true;
}

void DefaultObjectEditor::destroy()
{
	safeDestroy(m_propertyList);
	m_instance = 0;
	m_object = 0;
}

void DefaultObjectEditor::apply()
{
	m_propertyList->apply();
	m_instance->setObject(m_object);
}

bool DefaultObjectEditor::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getPath();
	return true;
}

void DefaultObjectEditor::eventPropertyCommand(ui::Event* event)
{
	const ui::CommandEvent* cmdEvent = checked_type_cast< const ui::CommandEvent* >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

	Ref< ui::custom::FilePropertyItem > fileItem = dynamic_type_cast< ui::custom::FilePropertyItem* >(event->getItem());
	if (fileItem)
	{
		ui::FileDialog fileDialog;
		if (!fileDialog.create(m_propertyList, i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
			return;

		Path path = fileItem->getPath();
		if (fileDialog.showModal(path) == ui::DrOk)
		{
			fileItem->setPath(path);
			m_propertyList->apply();
		}

		fileDialog.destroy();
	}

	Ref< ui::custom::BrowsePropertyItem > browseItem = dynamic_type_cast< ui::custom::BrowsePropertyItem* >(event->getItem());
	if (browseItem)
	{
		if (cmd == L"Property.Browse")
		{
			if (browseItem->getValue().isNull())
			{
				Ref< db::Instance > instance;
				if (browseItem->getFilterType())
				{
					const TypeInfo* filterType = browseItem->getFilterType();

					// Check if filter type is actually a result of a asset; in such case we should
					// browse for the asset and not the final result.
					TypeInfoSet filterTypes;

					std::vector< const TypeInfo* > assetTypes;
					type_of< ITypedAsset >().findAllOf(assetTypes);
					for (std::vector< const TypeInfo* >::iterator i = assetTypes.begin(); i != assetTypes.end(); ++i)
					{
						Ref< ITypedAsset > asset = dynamic_type_cast< ITypedAsset* >((*i)->createInstance());
						if (asset && asset->getOutputType())
						{
							if (is_type_of(*asset->getOutputType(), *filterType))
								filterTypes.insert(*i);
						}
					}

					if (filterTypes.empty())
						filterTypes.insert(filterType);

					editor::TypeBrowseFilter filter(filterTypes);
					instance = m_editor->browseInstance(&filter);
				}
				else
					instance = m_editor->browseInstance();

				if (instance)
				{
					browseItem->setValue(instance->getGuid());
					m_propertyList->apply();
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_propertyList->apply();
			}
		}
		else if (cmd == L"Property.Edit")
		{
			Guid instanceGuid = browseItem->getValue();
			if (instanceGuid.isNull() || !instanceGuid.isValid())
				return;

			Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(instanceGuid);
			if (!instance)
				return;

			m_editor->openEditor(instance);
		}
	}

	Ref< ui::custom::ObjectPropertyItem > objectItem = dynamic_type_cast< ui::custom::ObjectPropertyItem* >(event->getItem());
	if (objectItem)
	{
		const TypeInfo* objectType = objectItem->getObjectType();
		if (!objectType)
			objectType = &type_of< ISerializable >();

		if (!objectItem->getObject())
		{
			objectType = m_editor->browseType(objectType);
			if (objectType)
			{
				Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
				if (object)
				{
					objectItem->setObject(object);

					m_propertyList->refresh(objectItem, object);
					m_propertyList->apply();
				}
			}
		}
		else
		{
			objectItem->setObject(0);

			m_propertyList->refresh(objectItem, 0);
			m_propertyList->apply();
		}
	}
	
	Ref< ui::custom::ArrayPropertyItem > arrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(event->getItem());
	if (arrayItem)
	{
		if (cmd == L"Property.Edit")
		{
			if (arrayItem->getElementType())
			{
				const TypeInfo* objectType = m_editor->browseType(arrayItem->getElementType());
				if (objectType)
				{
					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
					if (object)
					{
						m_propertyList->addObject(arrayItem, object);
						m_propertyList->apply();
						m_propertyList->refresh();
					}
				}
			}
			else	// Non-complex array; just apply and refresh.
			{
				m_propertyList->apply();
				m_propertyList->refresh();
			}
		}
		else if (cmd == L"Property.Remove")
		{
			m_propertyList->apply();
			m_propertyList->refresh();
		}
	}

	Ref< ui::custom::ColorPropertyItem > colorItem = dynamic_type_cast< ui::custom::ColorPropertyItem* >(event->getItem());
	if (colorItem)
	{
		ui::custom::ColorDialog dialogColor;
		if (dialogColor.create(m_propertyList, i18n::Text(L"COLOR_DIALOG_TEXT"), ui::custom::ColorDialog::WsDefaultFixed | ui::custom::ColorDialog::WsAlpha, colorItem->getValue()))
		{
			if (dialogColor.showModal() == ui::DrOk)
			{
				colorItem->setValue(dialogColor.getColor());
				m_propertyList->apply();
			}
		}
		dialogColor.destroy();
	}

	m_propertyList->update();
}

	}
}
