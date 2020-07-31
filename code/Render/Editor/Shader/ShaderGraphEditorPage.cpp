#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/IBrowseFilter.h"
#include "I18N/Text.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/INodeFacade.h"
#include "Render/Editor/Shader/NodeCategories.h"
#include "Render/Editor/Shader/ShaderDependencyPane.h"
#include "Render/Editor/Shader/ShaderDependencyTracker.h"
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/ShaderGraphEditorClipboardData.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/ShaderGraphEvaluator.h"
#include "Render/Editor/Shader/ShaderGraphHash.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Editor/Shader/ShaderViewer.h"
#include "Render/Editor/Shader/QuickMenuTool.h"
#include "Render/Editor/Shader/Facades/DefaultNodeFacade.h"
#include "Render/Editor/Shader/Facades/ColorNodeFacade.h"
#include "Render/Editor/Shader/Facades/CommentNodeFacade.h"
#include "Render/Editor/Shader/Facades/ExternalNodeFacade.h"
#include "Render/Editor/Shader/Facades/InterpolatorNodeFacade.h"
#include "Render/Editor/Shader/Facades/ScriptNodeFacade.h"
#include "Render/Editor/Shader/Facades/SwitchNodeFacade.h"
#include "Render/Editor/Shader/Facades/SwizzleNodeFacade.h"
#include "Render/Editor/Shader/Facades/TextureNodeFacade.h"
#include "Render/Editor/Shader/Facades/UniformNodeFacade.h"
#include "Render/Editor/Shader/Facades/VariableNodeFacade.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/PaintSettings.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeActivateEvent.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowDoubleClickEvent.h"
#include "Ui/GridView/GridView.h"

// Resources
#include "Resources/Tools.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const wchar_t* c_typeNames[] =
{
	L"",
	L"Scalar",
	L"Vector 2",
	L"Vector 3",
	L"Vector 4",
	L"Matrix",
	L"Texture 2D",
	L"Texture 3D",
	L"Texture Cube",
	L"Struct Buffer",
	L"State"
};

class FragmentReaderAdapter : public FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(db::Database* db)
	:	m_db(db)
	{
	}

	virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid) const
	{
		return m_db->getObjectReadOnly< ShaderGraph >(fragmentGuid);
	}

private:
	Ref< db::Database > m_db;
};

struct RemoveInputPortPred
{
	bool m_connectable;
	bool m_optional;

	RemoveInputPortPred(bool connectable, bool optional)
	:	m_connectable(connectable)
	,	m_optional(optional)
	{
	}

	bool operator () (InputPort* ip) const
	{
		return ip->isConnectable() == m_connectable && ip->isOptional() == m_optional;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphEditorPage", ShaderGraphEditorPage, editor::IEditorPage)

ShaderGraphEditorPage::ShaderGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool ShaderGraphEditorPage::create(ui::Container* parent)
{
	m_shaderGraph = m_document->getObject< ShaderGraph >(0);
	if (!m_shaderGraph)
		return false;

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	// Create our custom toolbar.
	m_toolBar = new ui::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addImage(new ui::StyleBitmap(L"Shader.Tools"), 18);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_CENTER"), 7, ui::Command(L"ShaderGraph.Editor.Center")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_LEFT"), 0, ui::Command(L"ShaderGraph.Editor.AlignLeft")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_RIGHT"), 1, ui::Command(L"ShaderGraph.Editor.AlignRight")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_TOP"), 2, ui::Command(L"ShaderGraph.Editor.AlignTop")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_ALIGN_BOTTOM"), 3, ui::Command(L"ShaderGraph.Editor.AlignBottom")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVEN_VERTICALLY"), 4, ui::Command(L"ShaderGraph.Editor.EvenSpaceVertically")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVEN_HORIZONTALLY"), 5, ui::Command(L"ShaderGraph.Editor.EventSpaceHorizontally")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVALUATE_CONNECTED"), 14, ui::Command(L"ShaderGraph.Editor.EvaluateConnected")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_EVALUATE_TYPE"), 15, ui::Command(L"ShaderGraph.Editor.EvaluateType")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_REMOVE_UNUSED_NODES"), 8, ui::Command(L"ShaderGraph.Editor.RemoveUnusedNodes")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_AUTO_MERGE_BRANCHES"), 9, ui::Command(L"ShaderGraph.Editor.AutoMergeBranches")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_UPDATE_FRAGMENTS"), 10, ui::Command(L"ShaderGraph.Editor.UpdateFragments")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_CONSTANT_FOLD"), 11, ui::Command(L"ShaderGraph.Editor.ConstantFold")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_CLEANUP_SWIZZLES"), 12, ui::Command(L"ShaderGraph.Editor.CleanupSwizzles")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_INSERT_INTERPOLATORS"), 13, ui::Command(L"ShaderGraph.Editor.InsertInterpolators")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RESOLVE_VARIABLES"), 16, ui::Command(L"ShaderGraph.Editor.ResolveVariables")));
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RESOLVE_EXTERNALS"), 17, ui::Command(L"ShaderGraph.Editor.ResolveExternals")));

	m_toolBar->addItem(new ui::ToolBarSeparator());

	m_toolPlatform = new ui::ToolBarDropDown(ui::Command(), ui::dpi96(100), i18n::Text(L"SHADERGRAPH_PLATFORM_PERMUTATION"));
	m_toolPlatform->add(L"Android");
	m_toolPlatform->add(L"Emscripten");
	m_toolPlatform->add(L"iOS");
	m_toolPlatform->add(L"Linux");
	m_toolPlatform->add(L"macOS");
	m_toolPlatform->add(L"PS3");
	m_toolPlatform->add(L"PS4");
	m_toolPlatform->add(L"RaspberryPI");
	m_toolPlatform->add(L"Windows");
	m_toolBar->addItem(m_toolPlatform);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_PLATFORM_PERMUTATION"), 10, ui::Command(L"ShaderGraph.Editor.PlatformPermutation")));

	m_toolBar->addItem(new ui::ToolBarSeparator());

	m_toolRenderer = new ui::ToolBarDropDown(ui::Command(), ui::dpi96(100), i18n::Text(L"SHADERGRAPH_RENDERER_PERMUTATION"));
	m_toolRenderer->add(L"DX11");
	m_toolRenderer->add(L"OpenGL");
	m_toolRenderer->add(L"OpenGL ES2");
	m_toolRenderer->add(L"Vulkan");
	m_toolRenderer->add(L"GCM");
	m_toolRenderer->add(L"GNM");
	m_toolBar->addItem(m_toolRenderer);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RENDERER_PERMUTATION"), 10, ui::Command(L"ShaderGraph.Editor.RendererPermutation")));

	m_toolBar->addItem(new ui::ToolBarSeparator());

	m_toolTechniques = new ui::ToolBarDropDown(ui::Command(), ui::dpi96(200), i18n::Text(L"SHADERGRAPH_RENDERER_TECHNIQUE"));
	m_toolBar->addItem(m_toolTechniques);
	m_toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SHADERGRAPH_RENDERER_TECHNIQUE"), 10, ui::Command(L"ShaderGraph.Editor.Technique")));

	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ShaderGraphEditorPage::eventToolClick);

	// Create shader graph editor control.
	m_editorGraph = new ui::GraphControl();
	m_editorGraph->create(container);
	m_editorGraph->addEventHandler< ui::MouseButtonDownEvent >(this, &ShaderGraphEditorPage::eventButtonDown);
	m_editorGraph->addEventHandler< ui::SelectEvent >(this, &ShaderGraphEditorPage::eventSelect);
	m_editorGraph->addEventHandler< ui::NodeMovedEvent >(this, &ShaderGraphEditorPage::eventNodeMoved);
	m_editorGraph->addEventHandler< ui::NodeActivateEvent >(this, &ShaderGraphEditorPage::eventNodeDoubleClick);
	m_editorGraph->addEventHandler< ui::EdgeConnectEvent >(this, &ShaderGraphEditorPage::eventEdgeConnect);
	m_editorGraph->addEventHandler< ui::EdgeDisconnectEvent >(this, &ShaderGraphEditorPage::eventEdgeDisconnect);

	// Create shader graph referee view.
	m_dependencyPane = new ShaderDependencyPane(m_editor, m_document->getInstance(0)->getGuid());
	m_dependencyPane->create(parent);
	m_dependencyPane->setVisible(m_editor->getSettings()->getProperty< bool >(L"ShaderEditor.ShaderDependencyPaneVisible", true));
	m_site->createAdditionalPanel(m_dependencyPane, ui::dpi96(400), false);

	// Create shader graph output view.
	m_shaderViewer = new ShaderViewer(m_editor);
	m_shaderViewer->create(parent);
	m_shaderViewer->setVisible(m_editor->getSettings()->getProperty< bool >(L"ShaderEditor.ShaderViewVisible", true));
	m_site->createAdditionalPanel(m_shaderViewer, ui::dpi96(400), false);

	// Create variable grid.
	m_variablesContainer = new ui::Container();
	m_variablesContainer->create(parent, ui::WsNone, new ui::FloodLayout());
	m_variablesContainer->setText(i18n::Text(L"SHADERGRAPH_VARIABLES"));
	m_site->createAdditionalPanel(m_variablesContainer, ui::dpi96(400), false);

	m_variablesGrid = new ui::GridView();
	m_variablesGrid->create(m_variablesContainer, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit);
	m_variablesGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_VARIABLES_NAME"), ui::dpi96(140), true));
	m_variablesGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_VARIABLES_SCOPE"), ui::dpi96(80), false));
	m_variablesGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_VARIABLES_N_READ"), ui::dpi96(80), false));
	m_variablesGrid->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_VARIABLES_TYPE"), ui::dpi96(80), false));
	m_variablesGrid->addEventHandler< ui::GridItemContentChangeEvent >(this, &ShaderGraphEditorPage::eventVariableEdit);
	m_variablesGrid->addEventHandler< ui::GridRowDoubleClickEvent >(this, &ShaderGraphEditorPage::eventVariableDoubleClick);

	// Build popup menu.
	m_menuPopup = new ui::Menu();
	Ref< ui::MenuItem > menuItemCreate = new ui::MenuItem(i18n::Text(L"SHADERGRAPH_CREATE_NODE"));

	std::map< std::wstring, Ref< ui::MenuItem > > categories;
	for (size_t i = 0; i < sizeof_array(c_nodeCategories); ++i)
	{
		if (categories.find(c_nodeCategories[i].category) == categories.end())
		{
			categories[c_nodeCategories[i].category] = new ui::MenuItem(i18n::Text(c_nodeCategories[i].category));
			menuItemCreate->add(
				categories[c_nodeCategories[i].category]
			);
		}

		std::wstring title = c_nodeCategories[i].type.getName();
		size_t p = title.find_last_of(L'.');
		if (p > 0)
			title = i18n::Text(L"SHADERGRAPH_NODE_" + toUpper(title.substr(p + 1)));

		categories[c_nodeCategories[i].category]->add(
			new ui::MenuItem(ui::Command(i, L"ShaderGraph.Editor.Create"), title)
		);
	}

	m_menuPopup->add(menuItemCreate);
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SHADERGRAPH_DELETE_NODE")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"ShaderGraph.Editor.FindInDatabase"), i18n::Text(L"SHADERGRAPH_FIND_IN_DATABASE")));

	// Build quick menu.
	m_menuQuick = new QuickMenuTool();
	m_menuQuick->create(m_editorGraph);

	// Setup node facades.
	TypeInfoSet nodeTypes;
	type_of< Node >().findAllOf(nodeTypes);

	for (TypeInfoSet::const_iterator i = nodeTypes.begin(); i != nodeTypes.end(); ++i)
		m_nodeFacades[*i] = new DefaultNodeFacade(m_editorGraph);

	m_nodeFacades[&type_of< Color >()] = new ColorNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Comment >()] = new CommentNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< External >()] = new ExternalNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Interpolator >()] = new InterpolatorNodeFacade();
	m_nodeFacades[&type_of< Script >()] = new ScriptNodeFacade(this, m_editorGraph);
	m_nodeFacades[&type_of< Switch >()] = new SwitchNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Swizzle >()] = new SwizzleNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Texture >()] = new TextureNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Uniform >()] = new UniformNodeFacade(m_editorGraph);
	m_nodeFacades[&type_of< Variable >()] = new VariableNodeFacade(m_editorGraph);

	createEditorNodes(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	parent->update();
	m_editorGraph->center();

	updateGraph();

	m_site->setPropertyObject(0);
	return true;
}

void ShaderGraphEditorPage::destroy()
{
	if (m_shaderViewer)
	{
		m_editor->checkoutGlobalSettings()->setProperty< PropertyBoolean >(L"ShaderEditor.ShaderViewVisible", m_shaderViewer->isVisible(false));
		m_editor->commitGlobalSettings();
		m_site->destroyAdditionalPanel(m_shaderViewer);
	}

	if (m_dependencyPane)
	{
		m_editor->checkoutGlobalSettings()->setProperty< PropertyBoolean >(L"ShaderEditor.ShaderDependencyPaneVisible", m_dependencyPane->isVisible(false));
		m_editor->commitGlobalSettings();
		m_site->destroyAdditionalPanel(m_dependencyPane);
	}

	if (m_variablesContainer)
		m_site->destroyAdditionalPanel(m_variablesContainer);

	m_nodeFacades.clear();
	safeDestroy(m_editorGraph);
	safeDestroy(m_shaderViewer);
	safeDestroy(m_dependencyPane);
	safeDestroy(m_variablesContainer);
	safeDestroy(m_menuQuick);
}

bool ShaderGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	T_ASSERT(primaryType);

	// Create texture node in case of a TextureAsset.
	if (is_type_of< TextureAsset >(*primaryType))
	{
		Ref< Texture > shaderNode = new Texture(instance->getGuid());
		m_shaderGraph->addNode(shaderNode);

		ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		shaderNode->setPosition(std::make_pair(absolutePosition.x, absolutePosition.y));

		Ref< ui::Node > editorNode = createEditorNode(shaderNode);
		m_editorGraph->addNode(editorNode);

		updateGraph();
	}
	// Create an external node in case of ShaderGraph.
	else if (is_type_of< ShaderGraph >(*primaryType))
	{
		// Prevent dropping itself thus creating cyclic dependencies.
		if (m_document->containInstance(instance))
			return false;

		Ref< ShaderGraph > fragmentGraph = instance->getObject< ShaderGraph >();
		T_ASSERT(fragmentGraph);

		Ref< External > shaderNode = new External(
			instance->getGuid(),
			fragmentGraph
		);
		m_shaderGraph->addNode(shaderNode);

		ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		shaderNode->setPosition(std::make_pair(absolutePosition.x, absolutePosition.y));

		Ref< ui::Node > editorNode = createEditorNode(shaderNode);
		m_editorGraph->addNode(editorNode);

		updateGraph();
	}
	else
		return false;

	return true;
}

bool ShaderGraphEditorPage::handleCommand(const ui::Command& command)
{
	if (m_shaderViewer->handleCommand(command))
		return true;

	if (command == L"Editor.PropertiesChanging")
	{
		m_document->push();
	}
	else if (command == L"Editor.PropertiesChanged")
	{
		refreshGraph();
		updateGraph();
	}
	else if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		RefArray< ui::Node > selectedNodes;
		if (m_editorGraph->getSelectedNodes(selectedNodes) > 0)
		{
			// Also copy edges which are affected by selected nodes.
			RefArray< ui::Edge > selectedEdges;
			m_editorGraph->getConnectedEdges(selectedNodes, true, selectedEdges);

			Ref< ShaderGraphEditorClipboardData > data = new ShaderGraphEditorClipboardData();

			ui::Rect bounds(0, 0, 0, 0);
			for (auto i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
			{
				Ref< Node > shaderNode = (*i)->getData< Node >(L"SHADERNODE");
				T_ASSERT(shaderNode);
				data->addNode(shaderNode);

				if (i != selectedNodes.begin())
				{
					ui::Rect rc = (*i)->calculateRect();
					bounds.left = std::min(bounds.left, rc.left);
					bounds.top = std::min(bounds.top, rc.top);
					bounds.right = std::max(bounds.right, rc.right);
					bounds.bottom = std::max(bounds.bottom, rc.bottom);
				}
				else
					bounds = (*i)->calculateRect();
			}

			data->setBounds(bounds);

			for (auto selectedEdge : selectedEdges)
			{
				Ref< Edge > shaderEdge = selectedEdge->getData< Edge >(L"SHADEREDGE");
				T_ASSERT(shaderEdge);
				data->addEdge(shaderEdge);
			}

			ui::Application::getInstance()->getClipboard()->setObject(data);

			// Remove edges and nodes from graphs if user cuts.
			if (command == L"Editor.Cut")
			{
				// Save undo state.
				m_document->push();

				// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
				selectedEdges.resize(0);
				m_editorGraph->getConnectedEdges(selectedNodes, false, selectedEdges);

				for (auto selectedEdge : selectedEdges)
				{
					m_shaderGraph->removeEdge(selectedEdge->getData< Edge >(L"SHADEREDGE"));
					m_editorGraph->removeEdge(selectedEdge);
				}

				for (auto selectedNode : selectedNodes)
				{
					m_shaderGraph->removeNode(selectedNode->getData< Node >(L"SHADERNODE"));
					m_editorGraph->removeNode(selectedNode);
				}
			}
		}
	}
	else if (command == L"Editor.Paste")
	{
		Ref< ShaderGraphEditorClipboardData > data = dynamic_type_cast< ShaderGraphEditorClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (data)
		{
			// Save undo state.
			m_document->push();

			const ui::Rect& bounds = data->getBounds();

			ui::Rect rcClient = m_editorGraph->getInnerRect();
			ui::Point center = m_editorGraph->clientToVirtual(rcClient.getCenter());

			for (RefArray< Node >::const_iterator i = data->getNodes().begin(); i != data->getNodes().end(); ++i)
			{
				// Create new unique instance ID.
				(*i)->setId(Guid::create());

				// Place node in view.
				std::pair< int, int > position = (*i)->getPosition();
				position.first = ui::invdpi96(center.x + ui::dpi96(position.first) - bounds.left - bounds.getWidth() / 2);
				position.second = ui::invdpi96(center.y + ui::dpi96(position.second) - bounds.top - bounds.getHeight() / 2);
				(*i)->setPosition(position);

				// Add node to graph.
				m_shaderGraph->addNode(*i);
			}

			for (RefArray< Edge >::const_iterator i = data->getEdges().begin(); i != data->getEdges().end(); ++i)
				m_shaderGraph->addEdge(*i);

			createEditorNodes(
				data->getNodes(),
				data->getEdges()
			);
			updateGraph();
		}
	}
	else if (command == L"Editor.SelectAll")
	{
		m_editorGraph->selectAllNodes();
		updateGraph();
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< ui::Node > nodes;
		if (m_editorGraph->getSelectedNodes(nodes) <= 0)
			return false;

		// Save undo state.
		m_document->push();

		// Remove edges first which are connected to selected nodes.
		RefArray< ui::Edge > edges;
		m_editorGraph->getConnectedEdges(nodes, false, edges);

		for (RefArray< ui::Edge >::iterator i = edges.begin(); i != edges.end(); ++i)
		{
			ui::Edge* editorEdge = *i;
			Ref< Edge > shaderEdge = editorEdge->getData< Edge >(L"SHADEREDGE");

			m_editorGraph->removeEdge(editorEdge);
			m_shaderGraph->removeEdge(shaderEdge);
		}

		for (RefArray< ui::Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			ui::Node* editorNode = *i;
			Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");

			m_editorGraph->removeNode(editorNode);
			m_shaderGraph->removeNode(shaderNode);
		}

		updateGraph();
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			m_shaderGraph = m_document->getObject< ShaderGraph >(0);
			T_ASSERT(m_shaderGraph);

			createEditorGraph();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			m_shaderGraph = m_document->getObject< ShaderGraph >(0);
			T_ASSERT(m_shaderGraph);

			createEditorGraph();
		}
	}
	else if (command == L"ShaderGraph.Editor.Center")
	{
		m_editorGraph->center();
	}
	else if (command == L"ShaderGraph.Editor.AlignLeft")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnLeft);
	}
	else if (command == L"ShaderGraph.Editor.AlignRight")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnRight);
	}
	else if (command == L"ShaderGraph.Editor.AlignTop")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnTop);
	}
	else if (command == L"ShaderGraph.Editor.AlignBottom")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnBottom);
	}
	else if (command == L"ShaderGraph.Editor.EvenSpaceVertically")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsVertically);
	}
	else if (command == L"ShaderGraph.Editor.EvenSpaceHorizontally")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsHorizontally);
	}
	else if (command == L"ShaderGraph.Editor.EvaluateConnected")
	{
		m_document->push();

		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).getConnectedPermutation();
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.EvaluateType")
	{
		m_document->push();

		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).getTypePermutation();
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.RemoveUnusedNodes")
	{
		m_document->push();

		m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).removeUnusedBranches();
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.AutoMergeBranches")
	{
		m_document->push();

		m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).mergeBranches();
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.UpdateFragments")
	{
		RefArray< ui::Node > selectedNodes;
		m_editorGraph->getSelectedNodes(selectedNodes);

		// Get selected external nodes; ie fragments.
		RefArray< External > selectedExternals;
		for (RefArray< ui::Node >::const_iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
		{
			Ref< External > selectedExternal = (*i)->getData< External >(L"SHADERNODE");
			if (selectedExternal)
				selectedExternals.push_back(selectedExternal);
		}

		if (!selectedExternals.empty())
		{
			m_document->push();

			for (RefArray< External >::const_iterator i = selectedExternals.begin(); i != selectedExternals.end(); ++i)
				updateExternalNode(*i);

			createEditorGraph();
		}
	}
	else if (command == L"ShaderGraph.Editor.ConstantFold")
	{
		m_document->push();

		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).getConnectedPermutation();
		T_ASSERT(m_shaderGraph);

		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).getTypePermutation();
		T_ASSERT(m_shaderGraph);

		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).getConstantFolded();
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.CleanupSwizzles")
	{
		m_document->push();

		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).cleanupRedundantSwizzles();
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.InsertInterpolators")
	{
		m_document->push();

		m_shaderGraph = ShaderGraphOptimizer(m_shaderGraph).insertInterpolators(false);
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.ResolveVariables")
	{
		m_document->push();

		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).getVariableResolved(ShaderGraphStatic::VrtLocal);
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.ResolveExternals")
	{
		FragmentReaderAdapter reader(m_editor->getSourceDatabase());
		Ref< ShaderGraph > shaderGraph = FragmentLinker(reader).resolve(m_shaderGraph, false);
		if (shaderGraph)
		{
			m_document->push();
			m_shaderGraph = shaderGraph;
			m_document->setObject(0, m_shaderGraph);
			createEditorGraph();
		}
		else
			log::error << L"Fragment linker failed." << Endl;
	}
	else if (command == L"ShaderGraph.Editor.PlatformPermutation")
	{
		m_document->push();

		std::wstring platformSignature = m_toolPlatform->getSelectedItem();
		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).getPlatformPermutation(platformSignature);
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.RendererPermutation")
	{
		m_document->push();

		std::wstring rendererSignature = m_toolRenderer->getSelectedItem();
		m_shaderGraph = ShaderGraphStatic(m_shaderGraph).getRendererPermutation(rendererSignature);
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.Technique")
	{
		m_document->push();

		std::wstring technique = m_toolTechniques->getSelectedItem();
		m_shaderGraph = ShaderGraphTechniques(m_shaderGraph).generate(technique);
		T_ASSERT(m_shaderGraph);

		m_document->setObject(0, m_shaderGraph);

		createEditorGraph();
	}
	else if (command == L"ShaderGraph.Editor.QuickMenu")
	{
		const TypeInfo* typeInfo = m_menuQuick->showMenu();
		if (typeInfo)
		{
			m_document->push();

			createNode(
				typeInfo,
				m_editorGraph->clientToVirtual(m_editorGraph->getInnerRect().getCenter())
			);
		}
		m_editorGraph->setFocus();
	}
	else if (command == L"ShaderGraph.Editor.FindInDatabase")
	{
		RefArray< ui::Node > nodes;
		if (m_editorGraph->getSelectedNodes(nodes) <= 0)
			return false;

		for (RefArray< ui::Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			Ref< External > selectedExternal = (*i)->getData< External >(L"SHADERNODE");
			if (selectedExternal)
			{
				Ref< db::Instance > fragmentInstance = m_editor->getSourceDatabase()->getInstance(selectedExternal->getFragmentGuid());
				if (fragmentInstance)
					m_editor->highlightInstance(fragmentInstance);
			}
		}
	}
	else
		return false;

	m_editorGraph->update();
	return true;
}

void ShaderGraphEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_shaderGraph)
		m_shaderViewer->reflect(m_shaderGraph);
}

void ShaderGraphEditorPage::createEditorGraph()
{
	m_editorGraph->removeAllEdges();
	m_editorGraph->removeAllNodes();

	createEditorNodes(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	updateGraph();

	m_site->setPropertyObject(0);
}

void ShaderGraphEditorPage::createEditorNodes(const RefArray< Node >& shaderNodes, const RefArray< Edge >& shaderEdges)
{
	// Keep a map from shader nodes to editor nodes.
	std::map< Ref< Node >, Ref< ui::Node > > nodeMap;

	// Create editor nodes for each shader node.
	for (auto shaderNode : shaderNodes)
	{
		Ref< ui::Node > editorNode = createEditorNode(shaderNode);
		m_editorGraph->addNode(editorNode);
		nodeMap[shaderNode] = editorNode;
	}

	// Create editor edges for each shader edge.
	for (auto shaderEdge : shaderEdges)
	{
		const OutputPin* shaderSourcePin = shaderEdge->getSource();
		if (!shaderSourcePin)
		{
			log::warning << L"Invalid shader edge, no source pin" << Endl;
			continue;
		}

		const InputPin* shaderDestinationPin = shaderEdge->getDestination();
		if (!shaderDestinationPin)
		{
			log::warning << L"Invalid shader edge, no destination pin" << Endl;
			continue;
		}

		Ref< ui::Node > editorSourceNode = nodeMap[shaderSourcePin->getNode()];
		if (!editorSourceNode)
		{
			log::warning << L"Invalid shader pin, no editor source node found of pin \"" << shaderSourcePin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::Node > editorDestinationNode = nodeMap[shaderDestinationPin->getNode()];
		if (!editorDestinationNode)
		{
			log::warning << L"Invalid shader pin, no editor destination node found of pin \"" << shaderDestinationPin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::Pin > editorSourcePin = editorSourceNode->findOutputPin(shaderSourcePin->getName());
		if (!editorSourcePin)
		{
			log::warning << L"Unable to find editor source pin \"" << shaderSourcePin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::Pin > editorDestinationPin = editorDestinationNode->findInputPin(shaderDestinationPin->getName());
		if (!editorDestinationPin)
		{
			log::warning << L"Unable to find editor destination pin \"" << shaderDestinationPin->getName() << L"\"" << Endl;
			continue;
		}

		Ref< ui::Edge > editorEdge = new ui::Edge(editorSourcePin, editorDestinationPin);
		editorEdge->setData(L"SHADEREDGE", shaderEdge);

		m_editorGraph->addEdge(editorEdge);
	}
}

Ref< ui::Node > ShaderGraphEditorPage::createEditorNode(Node* shaderNode)
{
	Ref< INodeFacade > nodeFacade = m_nodeFacades[&type_of(shaderNode)];
	T_ASSERT_M (nodeFacade, L"No node facade class found");

	Ref< ui::Node > editorNode = nodeFacade->createEditorNode(
		m_editor,
		m_editorGraph,
		m_shaderGraph,
		shaderNode
	);

	if (!editorNode)
		return 0;

	editorNode->setData(L"SHADERNODE", shaderNode);
	editorNode->setData(L"FACADE", nodeFacade);

	return editorNode;
}

void ShaderGraphEditorPage::createNode(const TypeInfo* nodeType, const ui::Point& at)
{
	Ref< Node > shaderNode = m_nodeFacades[nodeType]->createShaderNode(nodeType, m_editor);
	if (!shaderNode)
		return;

	// Add to shader graph.
	shaderNode->setPosition(std::pair< int, int >(
		ui::invdpi96(at.x),
		ui::invdpi96(at.y)
	));
	m_shaderGraph->addNode(shaderNode);

	// Create editor node from shader node.
	Ref< ui::Node > editorNode = createEditorNode(shaderNode);
	m_editorGraph->addNode(editorNode);
	updateGraph();
}

void ShaderGraphEditorPage::refreshGraph()
{
	// Refresh editor nodes.
	for (auto editorNode : m_editorGraph->getNodes())
	{
		Node* shaderNode = editorNode->getData< Node >(L"SHADERNODE");
		INodeFacade* nodeFacade = editorNode->getData< INodeFacade >(L"FACADE");

		if (!shaderNode || !nodeFacade)
			continue;

		nodeFacade->refreshEditorNode(m_editor, m_editorGraph, editorNode, m_shaderGraph, shaderNode);

		const std::pair< int, int >& position = shaderNode->getPosition();
		editorNode->setPosition(ui::Point(
			ui::dpi96(position.first),
			ui::dpi96(position.second)
		));
	}
}

void ShaderGraphEditorPage::updateGraph()
{
	struct VariableInfo
	{
		uint32_t globalCount;
		uint32_t localCount;
		uint32_t writeCount;
		uint32_t readCount;
		PinType type;

		VariableInfo()
		:	globalCount(0)
		,	localCount(0)
		,	writeCount(0)
		,	readCount(0)
		,	type(PntVoid)
		{
		}
	};
	std::map< std::wstring, VariableInfo > variables;

	// Extract techniques.
	m_toolTechniques->removeAll();
	for (const auto& technique : ShaderGraphTechniques(m_shaderGraph).getNames())
		m_toolTechniques->add(technique);

	// Update variables grid.
	RefArray< Variable > variableNodes;
	m_shaderGraph->findNodesOf< Variable >(variableNodes);
	for (auto variableNode : variableNodes)
	{
		auto& vi = variables[variableNode->getName()];

		if (variableNode->isGlobal())
			++vi.globalCount;
		else
			++vi.localCount;

		vi.readCount += m_shaderGraph->getDestinationCount(variableNode->getOutputPin(0));

		const Edge* sourceEdge = m_shaderGraph->findEdge(variableNode->getInputPin(0));
		if (sourceEdge != nullptr)
		{
			++vi.writeCount;

			Constant value = ShaderGraphEvaluator(m_shaderGraph).evaluate(sourceEdge->getSource());
			vi.type = value.getType();
		}
	}
	m_variablesGrid->removeAllRows();
	for (const auto& variable : variables)
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(new ui::GridItem(variable.first));

		if (variable.second.globalCount > 0 && variable.second.localCount == 0)
			row->add(new ui::GridItem(i18n::Text(L"SHADERGRAPH_VARIABLES_GLOBAL")));
		else if (variable.second.globalCount == 0 && variable.second.localCount > 0)
			row->add(new ui::GridItem(i18n::Text(L"SHADERGRAPH_VARIABLES_LOCAL")));
		else
		{
			row->add(new ui::GridItem(i18n::Text(L"SHADERGRAPH_VARIABLES_SCOPE_ERROR")));
			row->setBackground(Color4ub(255, 0, 0, 255));
		}

		if (variable.second.readCount > 0 && variable.second.writeCount == 0 && variable.second.localCount > 0)
			row->setBackground(Color4ub(255, 0, 0, 255));

		row->add(new ui::GridItem(toString(variable.second.readCount)));
		row->add(new ui::GridItem(c_typeNames[(int32_t)variable.second.type]));
		m_variablesGrid->addRow(row);
	}

	// Validate shader graph.
	std::vector< const Node* > errorNodes;
	bool validationResult = ShaderGraphValidator(m_shaderGraph).validate(ShaderGraphValidator::SgtFragment, &errorNodes);

	// Update validation indication of each node.
	for (auto editorNode : m_editorGraph->getNodes())
	{
		Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
		T_ASSERT(shaderNode);

		Ref< INodeFacade > nodeFacade = editorNode->getData< INodeFacade >(L"FACADE");
		T_ASSERT(nodeFacade);

		if (std::find(errorNodes.begin(), errorNodes.end(), shaderNode) != errorNodes.end())
			nodeFacade->setValidationIndicator(editorNode, false);
		else
			nodeFacade->setValidationIndicator(editorNode, true);
	}

	// If validation succeeded then update generated shader as well.
	if (validationResult)
		m_shaderViewer->reflect(m_shaderGraph);

	// Evaluate output types (and partial values) if validation succeeded.
	if (validationResult)
	{
		for (auto editorEdge : m_editorGraph->getEdges())
		{
			Ref< Edge > shaderEdge = editorEdge->getData< Edge >(L"SHADEREDGE");
			T_ASSERT(shaderEdge);

			// Set default thickness first; override below for "fat" types.
			editorEdge->setThickness(2);

			StringOutputStream ss;
			Constant value = ShaderGraphEvaluator(m_shaderGraph).evaluate(shaderEdge->getSource());
			switch (value.getType())
			{
			case PntVoid:
				break;

			case PntScalar1:
			case PntScalar2:
			case PntScalar3:
			case PntScalar4:
				for (int32_t i = 0; i < value.getWidth(); ++i)
				{
					if (i > 0)
						ss << L",";

					if (value.isConst(i))
						ss << value.getValue(i);
					else
						ss << L"X";
				}
				break;

			case PntMatrix:
				ss << L"Matrix";
				break;

			case PntTexture2D:
				ss << L"Texture2d";
				editorEdge->setThickness(4);
				break;

			case PntTexture3D:
				ss << L"Texture3d";
				editorEdge->setThickness(4);
				break;

			case PntTextureCube:
				ss << L"TextureCube";
				editorEdge->setThickness(4);
				break;

			case PntStructBuffer:
				ss << L"StructBuffer";
				editorEdge->setThickness(4);
				break;

			case PntState:
				ss << L"State";
				editorEdge->setThickness(4);
				break;
			}

			editorEdge->setText(ss.str());
		}
	}
	else
	{
		for (auto editorEdge : m_editorGraph->getEdges())
		{
			editorEdge->setText(L"");
			editorEdge->setThickness(2);
		}
	}

	// Redraw editor graph.
	m_editorGraph->update();
}

void ShaderGraphEditorPage::updateExternalNode(External* external)
{
	// Get fragment graph from source database.
	Ref< ShaderGraph > fragmentGraph = m_editor->getSourceDatabase()->getObjectReadOnly< ShaderGraph >(external->getFragmentGuid());
	if (!fragmentGraph)
	{
		ui::MessageBox::show(
			i18n::Text(L"SHADERGRAPH_ERROR_MISSING_FRAGMENT_MESSAGE"),
			i18n::Text(L"SHADERGRAPH_ERROR_MISSING_FRAGMENT_CAPTION"),
			ui::MbIconError | ui::MbOk
		);
		return;
	}

	// Get input ports; remove non-connectable ports.
	RefArray< InputPort > fragmentInputs;
	fragmentGraph->findNodesOf< InputPort >(fragmentInputs);

	RefArray< InputPort >::iterator
		i = std::remove_if(fragmentInputs.begin(), fragmentInputs.end(), RemoveInputPortPred(false, false)); fragmentInputs.erase(i, fragmentInputs.end());
		i = std::remove_if(fragmentInputs.begin(), fragmentInputs.end(), RemoveInputPortPred(false, true));  fragmentInputs.erase(i, fragmentInputs.end());

	// Get output ports.
	RefArray< OutputPort > fragmentOutputs;
	fragmentGraph->findNodesOf< OutputPort >(fragmentOutputs);

	// Get input-/output pins; these might differ if fragment has been updated.
	uint32_t externalInputPinCount = external->getInputPinCount();
	uint32_t externalOutputPinCount = external->getOutputPinCount();

	std::vector< const InputPin* > externalInputPins(externalInputPinCount);
	for (uint32_t i = 0; i < externalInputPinCount; ++i)
		externalInputPins[i] = external->getInputPin(i);

	std::vector< const OutputPin* > externalOutputPins(externalOutputPinCount);
	for (uint32_t i = 0; i < externalOutputPinCount; ++i)
		externalOutputPins[i] = external->getOutputPin(i);

	// Remove input ports and pins which match.
	for (RefArray< InputPort >::iterator i = fragmentInputs.begin(); i != fragmentInputs.end(); )
	{
		std::vector< const InputPin* >::iterator j = externalInputPins.begin();
		while (j != externalInputPins.end())
		{
			if ((*i)->getName() == (*j)->getName())
				break;
			++j;
		}
		if (j != externalInputPins.end())
		{
			i = fragmentInputs.erase(i);
			externalInputPins.erase(j);
		}
		else
			++i;
	}

	// Remove output ports and pins which match.
	for (RefArray< OutputPort >::iterator i = fragmentOutputs.begin(); i != fragmentOutputs.end(); )
	{
		std::vector< const OutputPin* >::iterator j = externalOutputPins.begin();
		while (j != externalOutputPins.end())
		{
			if ((*i)->getName() == (*j)->getName())
				break;
			++j;
		}
		if (j != externalOutputPins.end())
		{
			i = fragmentOutputs.erase(i);
			externalOutputPins.erase(j);
		}
		else
			++i;
	}

	// If we don't have any ports nor pins there is nothing to update.
	if (
		fragmentInputs.empty() &&
		fragmentOutputs.empty() &&
		externalInputPins.empty() &&
		externalOutputPins.empty()
	)
		return;

	// Remove pins which have their respective ports removed.
	while (!externalInputPins.empty())
	{
		Ref< Edge > edge = m_shaderGraph->findEdge(externalInputPins.back());
		if (edge)
			m_shaderGraph->removeEdge(edge);

		external->removeValue(externalInputPins.back()->getName());
		external->removeInputPin(externalInputPins.back());

		externalInputPins.pop_back();
	}
	while (!externalOutputPins.empty())
	{
		RefSet< Edge > edges;
		m_shaderGraph->findEdges(externalOutputPins.back(), edges);
		for (auto edge : edges)
			m_shaderGraph->removeEdge(edge);

		external->removeOutputPin(externalOutputPins.back());
		externalOutputPins.pop_back();
	}

	// Add new pins for new ports.
	for (const auto& inputPort : fragmentInputs)
	{
		external->createInputPin(inputPort->getId(), inputPort->getName(), inputPort->isOptional());
		if (inputPort->isOptional() && inputPort->haveDefaultValue())
			external->setValue(inputPort->getName(), inputPort->getDefaultValue());
	}
	for (const auto& outputPort : fragmentOutputs)
		external->createOutputPin(outputPort->getId(), outputPort->getName());
}

void ShaderGraphEditorPage::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void ShaderGraphEditorPage::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_menuPopup->showModal(m_editorGraph, event->getPosition());
	if (!selected)
		return;

	const ui::Command& command = selected->getCommand();

	if (command == L"ShaderGraph.Editor.Create")	// Create
	{
		m_document->push();

		createNode(
			&c_nodeCategories[command.getId()].type,
			m_editorGraph->clientToVirtual(event->getPosition())
		);
	}
	else
		handleCommand(command);
}

void ShaderGraphEditorPage::eventSelect(ui::SelectEvent* event)
{
	RefArray< ui::Node > nodes;
	if (m_editorGraph->getSelectedNodes(nodes) == 1)
	{
		Ref< Node > shaderNode = nodes[0]->getData< Node >(L"SHADERNODE");
		T_ASSERT(shaderNode);

		m_site->setPropertyObject(shaderNode);
	}
	else
		m_site->setPropertyObject(0);
}

void ShaderGraphEditorPage::eventNodeMoved(ui::NodeMovedEvent* event)
{
	Ref< ui::Node > editorNode = event->getNode();
	T_ASSERT(editorNode);

	// Get shader graph node from editor node.
	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
	T_ASSERT(shaderNode);

	ui::Point position = editorNode->getPosition();
	position.x = ui::invdpi96(position.x);
	position.y = ui::invdpi96(position.y);

	if (position.x != shaderNode->getPosition().first || position.y != shaderNode->getPosition().second)
	{
		m_document->push();

		// Reflect position into shader graph node.
		shaderNode->setPosition(std::pair< int, int >(
			position.x,
			position.y
		));
	}

	// Update properties.
	if (editorNode->isSelected())
		m_site->setPropertyObject(shaderNode);
}

void ShaderGraphEditorPage::eventNodeDoubleClick(ui::NodeActivateEvent* event)
{
	Ref< ui::Node > editorNode = event->getNode();
	Ref< Node > shaderNode = editorNode->getData< Node >(L"SHADERNODE");
	T_ASSERT(shaderNode);

	m_nodeFacades[&type_of(shaderNode)]->editShaderNode(
		m_editor,
		m_editorGraph,
		editorNode,
		m_shaderGraph,
		shaderNode
	);

	// Update properties.
	m_site->setPropertyObject(shaderNode);

	// Refresh graph; information might have changed.
	refreshGraph();
}

void ShaderGraphEditorPage::eventEdgeConnect(ui::EdgeConnectEvent* event)
{
	Ref< ui::Edge > editorEdge = event->getEdge();
	Ref< ui::Pin > editorSourcePin = editorEdge->getSourcePin();
	T_ASSERT(editorSourcePin);

	Ref< ui::Pin > editorDestinationPin = editorEdge->getDestinationPin();
	T_ASSERT(editorDestinationPin);

	Ref< Node > shaderSourceNode = editorSourcePin->getNode()->getData< Node >(L"SHADERNODE");
	T_ASSERT(shaderSourceNode);

	Ref< Node > shaderDestinationNode = editorDestinationPin->getNode()->getData< Node >(L"SHADERNODE");
	T_ASSERT(shaderDestinationNode);

	const OutputPin* shaderSourcePin = shaderSourceNode->findOutputPin(editorSourcePin->getName());
	T_ASSERT(shaderSourcePin);

	const InputPin* shaderDestinationPin = shaderDestinationNode->findInputPin(editorDestinationPin->getName());
	T_ASSERT(shaderDestinationPin);

	// Replace existing edge.
	Ref< Edge > shaderEdge = m_shaderGraph->findEdge(shaderDestinationPin);
	if (shaderEdge)
	{
		m_shaderGraph->removeEdge(shaderEdge);

		RefArray< ui::Edge > editorEdges;
		m_editorGraph->getConnectedEdges(editorDestinationPin, editorEdges);
		T_ASSERT(editorEdges.size() == 1);

		m_editorGraph->removeEdge(editorEdges.front());
	}

	m_document->push();

	shaderEdge = new Edge(shaderSourcePin, shaderDestinationPin);
	m_shaderGraph->addEdge(shaderEdge);

	editorEdge->setData(L"SHADEREDGE", shaderEdge);
	m_editorGraph->addEdge(editorEdge);

	updateGraph();
}

void ShaderGraphEditorPage::eventEdgeDisconnect(ui::EdgeDisconnectEvent* event)
{
	ui::Edge* editorEdge = event->getEdge();
	Edge* edge = mandatory_non_null_type_cast< Edge* >(editorEdge->getData(L"SHADEREDGE"));

	m_document->push();
	m_shaderGraph->removeEdge(edge);

	updateGraph();
}

void ShaderGraphEditorPage::eventVariableEdit(ui::GridItemContentChangeEvent* event)
{
	RefArray< Variable > variableNodes;
	m_shaderGraph->findNodesOf< Variable >(variableNodes);

	std::wstring renameFrom = event->getOriginalText();
	std::wstring renameTo = event->getItem()->getText();

	if (renameFrom == renameTo)
		return;

	// Check if "rename to" is a valid name, ie not empty nor collide.
	if (renameTo.empty())
		return;

	for (auto variableNode : variableNodes)
	{
		std::wstring name = variableNode->getName();
		if (name != renameFrom && name == renameTo)
			return;
	}

	// Name is valid, rename variables.
	m_document->push();
	for (auto variableNode : variableNodes)
	{
		std::wstring name = variableNode->getName();
		if (name == renameFrom)
			variableNode->setName(renameTo);
	}

	refreshGraph();

	m_editorGraph->update();
	event->consume();
}

void ShaderGraphEditorPage::eventVariableDoubleClick(ui::GridRowDoubleClickEvent* event)
{
	std::wstring variableName = event->getRow()->get(0)->getText();

	RefArray< Variable > variableNodes;
	m_shaderGraph->findNodesOf< Variable >(variableNodes);

	auto it = std::find_if(variableNodes.begin(), variableNodes.end(), [&](const Variable* v) {
		if (v->getName() != variableName)
			return false;

		if (m_shaderGraph->findEdge(v->getInputPin(0)) == nullptr)
			return false;

		return true;
	});
	if (it == variableNodes.end())
		return;

	Variable* variable = *it;
	T_ASSERT(variable);

	m_editorGraph->deselectAllNodes();
	for (auto editorNode : m_editorGraph->getNodes())
	{
		if (editorNode->getData< Node >(L"SHADERNODE") == variable)
			editorNode->setSelected(true);
	}

	m_editorGraph->center(true);
	m_editorGraph->update();

	event->consume();
}

	}
}
