#pragma once

#include "Core/RefArray.h"
#include "Ui/Container.h"

namespace traktor
{

class PropertyGroup;

	namespace ui
	{

class Edit;
class GridRowDoubleClickEvent;
class GridView;
class HierarchicalState;
class Menu;
class Splitter;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class TreeView;
class TreeViewContentChangeEvent;
class TreeViewDragEvent;
class TreeViewItem;
class TreeViewItemActivateEvent;
class TreeViewItemStateChangeEvent;

	}

	namespace db
	{

class Database;
class Group;
class Instance;

	}

	namespace editor
	{

class IEditor;
class IWizardTool;

class DatabaseView : public ui::Container
{
	T_RTTI_CLASS;

public:
	class Filter : public Object
	{
		T_RTTI_CLASS;

	public:
		virtual bool acceptInstance(const db::Instance* instance) const = 0;

		virtual bool acceptEmptyGroups() const = 0;
	};

	DatabaseView(IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	void setDatabase(db::Database* db);

	void updateView();

	bool highlight(const db::Instance* instance);

	bool handleCommand(const ui::Command& command);

	virtual void setEnable(bool enable) override final;

private:
	IEditor* m_editor;
	Ref< ui::ToolBar > m_toolSelection;
	Ref< ui::ToolBarButton > m_toolFilterType;
	Ref< ui::ToolBarButton > m_toolFilterAssets;
	Ref< ui::ToolBarButton > m_toolFilterShow;
	Ref< ui::ToolBarButton > m_toolFavoritesShow;
	Ref< ui::ToolBarDropDown > m_toolViewMode;
	Ref< ui::Edit > m_editFilter;
	Ref< ui::Splitter > m_splitter;
	Ref< ui::TreeView > m_treeDatabase;
	Ref< ui::GridView > m_gridInstances;
	Ref< ui::HierarchicalState > m_treeState;
	Ref< ui::Menu > m_menuGroup[2];
	Ref< ui::Menu > m_menuInstance;
	Ref< ui::Menu > m_menuInstanceAsset;
	Ref< PropertyGroup > m_iconsGroup;
	Ref< db::Database > m_db;
	Ref< Filter > m_filter;
	RefArray< IWizardTool > m_wizardTools;
	std::set< Guid > m_rootInstances;
	std::set< Guid > m_favoriteInstances;
	std::wstring m_filterText;
	int32_t m_filterCountDown;
	int32_t m_colorCountDown;

	int32_t getIconIndex(const TypeInfo* instanceType) const;

	Ref< ui::TreeViewItem > buildTreeItem(ui::TreeView* treeView, ui::TreeViewItem* parentItem, db::Group* group);

	Ref< ui::TreeViewItem > buildTreeItemSplit(ui::TreeView* treeView, ui::TreeViewItem* parentItem, db::Group* group);

	void updateTreeColors();

	void updateGridInstances();

	void filterType(db::Instance* instance);

	void filterDependencies(db::Instance* instance);

	void listInstanceDependents(db::Instance* instance);

	void eventToolSelectionClicked(ui::ToolBarButtonClickEvent* event);

	void eventFilterKey(ui::KeyUpEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void eventInstanceActivate(ui::TreeViewItemActivateEvent* event);

	void eventInstanceStateChange(ui::TreeViewItemStateChangeEvent* event);

	void eventInstanceSelect(ui::SelectionChangeEvent* event);

	void eventInstanceButtonDown(ui::MouseButtonDownEvent* event);

	void eventInstanceRenamed(ui::TreeViewContentChangeEvent* event);

	void eventInstanceDrag(ui::TreeViewDragEvent* event);

	void eventInstanceGridActivate(ui::GridRowDoubleClickEvent* event);
};

	}
}

