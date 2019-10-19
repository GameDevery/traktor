#pragma once

#include <map>
#include "Core/RefArray.h"
#include "Editor/IEditorPage.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
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

class Container;
class EdgeConnectEvent;
class EdgeDisconnectEvent;
class GraphControl;
class GridItemContentChangeEvent;
class GridView;
class Menu;
class Node;
class NodeActivateEvent;
class NodeMovedEvent;
class Point;
class SelectEvent;
class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarDropDown;

	}

	namespace render
	{

class Edge;
class External;
class INodeFacade;
class Node;
class QuickMenuTool;
class ShaderDependencyPane;
class ShaderGraph;
class ShaderViewer;

class T_DLLEXPORT ShaderGraphEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	ShaderGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	void createEditorGraph();

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ShaderGraph > m_shaderGraph;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::ToolBarDropDown > m_toolPlatform;
	Ref< ui::ToolBarDropDown > m_toolRenderer;
	Ref< ui::ToolBarDropDown > m_toolTechniques;
	Ref< ui::GraphControl > m_editorGraph;
	Ref< ShaderDependencyPane > m_dependencyPane;
	Ref< ShaderViewer > m_shaderViewer;
	Ref< ui::Container > m_variablesContainer;
	Ref< ui::GridView > m_variablesGrid;
	Ref< ui::Menu > m_menuPopup;
	Ref< QuickMenuTool > m_menuQuick;
	std::map< const TypeInfo*, Ref< INodeFacade > > m_nodeFacades;

	void createEditorNodes(const RefArray< Node >& shaderNodes, const RefArray< Edge >& shaderEdges);

	Ref< ui::Node > createEditorNode(Node* shaderNode);

	void createNode(const TypeInfo* nodeType, const ui::Point& at);

	void refreshGraph();

	void updateGraph();

	void updateExternalNode(External* external);

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventSelect(ui::SelectEvent* event);

	void eventNodeMoved(ui::NodeMovedEvent* event);

	void eventNodeDoubleClick(ui::NodeActivateEvent* event);

	void eventEdgeConnect(ui::EdgeConnectEvent* event);

	void eventEdgeDisconnect(ui::EdgeDisconnectEvent* event);

	void eventVariableEdit(ui::GridItemContentChangeEvent* event);
};

	}
}

