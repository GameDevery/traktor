#ifndef traktor_editor_DatabaseView_H
#define traktor_editor_DatabaseView_H

#include "Core/RefArray.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class Edit;
class TreeView;
class TreeViewItem;
class HierarchicalState;
class PopupMenu;

		namespace custom
		{

class ToolBar;
class ToolBarButton;

		}
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

	void destroy();

	void setDatabase(db::Database* db);

	void updateView();

	bool highlight(const db::Instance* instance);

	bool handleCommand(const ui::Command& command);

	virtual void setEnable(bool enable);

private:
	IEditor* m_editor;
	Ref< ui::custom::ToolBar > m_toolSelection;
	Ref< ui::custom::ToolBarButton > m_toolFilterType;
	Ref< ui::custom::ToolBarButton > m_toolFilterAssets;
	Ref< ui::custom::ToolBarButton > m_toolFilterShow;
	Ref< ui::Edit > m_editFilter;
	Ref< ui::TreeView > m_treeDatabase;
	Ref< ui::HierarchicalState > m_treeState;
	Ref< ui::PopupMenu > m_menuGroup;
	Ref< ui::PopupMenu > m_menuInstance;
	Ref< ui::PopupMenu > m_menuInstanceAsset;
	Ref< db::Database > m_db;
	Ref< Filter > m_filter;
	RefArray< IWizardTool > m_wizardTools;
	std::set< Guid > m_rootInstances;
	std::wstring m_filterText;

	int32_t getIconIndex(const TypeInfo* instanceType) const;

	Ref< ui::TreeViewItem > buildTreeItem(ui::TreeView* treeView, ui::TreeViewItem* parentItem, db::Group* group);

	void filterType(db::Instance* instance);

	void filterDependencies(db::Instance* instance);

	void eventToolSelectionClicked(ui::Event* event);

	void eventFilterKey(ui::Event* event);

	void eventTimer(ui::Event* event);

	void eventInstanceActivate(ui::Event* event);

	void eventInstanceButtonDown(ui::Event* event);

	void eventInstanceRenamed(ui::Event* event);

	void eventInstanceDrag(ui::Event* event);
};

	}
}

#endif	// traktor_editor_DatabaseView_H
