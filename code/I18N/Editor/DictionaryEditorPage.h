/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_i18n_DictionaryEditorPage_H
#define traktor_i18n_DictionaryEditorPage_H

#include "Core/Ref.h"
#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace ui
	{

class GridItemContentChangeEvent;
class GridRowDoubleClickEvent;
class GridView;
class ToolBarButtonClickEvent;

	}

	namespace i18n
	{

class Dictionary;

class T_DLLCLASS DictionaryEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	DictionaryEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::GridView > m_gridDictionary;
	Ref< Dictionary > m_dictionary;
	Ref< Dictionary > m_referenceDictionary;

	void updateGrid();

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventGridRowDoubleClick(ui::GridRowDoubleClickEvent* event);

	void eventGridItemChange(ui::GridItemContentChangeEvent* event);
};

	}
}

#endif	// traktor_i18n_DictionaryEditorPage_H
