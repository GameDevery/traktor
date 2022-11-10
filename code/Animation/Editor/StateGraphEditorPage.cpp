/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StateNodeAnimation.h"
#include "Animation/Animation/StateNodeController.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Animation/Transition.h"
#include "Animation/Editor/AnimationPreviewControl.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/Editor/StateGraphEditorPage.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/AspectLayout.h"
#include "Ui/Splitter.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/DefaultNodeShape.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateGraphEditorPage", StateGraphEditorPage, editor::IEditorPage)

StateGraphEditorPage::StateGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool StateGraphEditorPage::create(ui::Container* parent)
{
	m_stateGraph = m_document->getObject< StateGraph >(0);
	if (!m_stateGraph)
		return false;

	m_statePreviewController = new StatePoseController(resource::Proxy< StateGraph >(m_stateGraph));

	// Create state graph container.
	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	// Create our custom toolbar.
	m_toolBarGraph = new ui::ToolBar();
	m_toolBarGraph->create(container);
	m_toolBarGraph->addImage(new ui::StyleBitmap(L"Animation.Alignment"), 6);
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_LEFT"), 0, ui::Command(L"StateGraph.Editor.AlignLeft")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_RIGHT"), 1, ui::Command(L"StateGraph.Editor.AlignRight")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_TOP"), 2, ui::Command(L"StateGraph.Editor.AlignTop")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_ALIGN_BOTTOM"), 3, ui::Command(L"StateGraph.Editor.AlignBottom")));
	m_toolBarGraph->addItem(new ui::ToolBarSeparator());
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_EVEN_VERTICALLY"), 4, ui::Command(L"StateGraph.Editor.EvenSpaceVertically")));
	m_toolBarGraph->addItem(new ui::ToolBarButton(i18n::Text(L"STATEGRAPH_EVEN_HORIZONTALLY"), 5, ui::Command(L"StateGraph.Editor.EventSpaceHorizontally")));
	m_toolBarGraph->addEventHandler< ui::ToolBarButtonClickEvent >(this, &StateGraphEditorPage::eventToolBarGraphClick);

	// Create state graph editor control.
	m_editorGraph = new ui::GraphControl();
	m_editorGraph->setText(L"ANIMATION STATE");
	m_editorGraph->create(container, ui::GraphControl::WsEdgeSelectable | ui::WsDoubleBuffer | ui::WsAccelerated);
	m_editorGraph->addEventHandler< ui::MouseButtonDownEvent >(this, &StateGraphEditorPage::eventButtonDown);
	m_editorGraph->addEventHandler< ui::SelectionChangeEvent >(this, &StateGraphEditorPage::eventSelect);
	m_editorGraph->addEventHandler< ui::NodeMovedEvent >(this, &StateGraphEditorPage::eventNodeMoved);
	m_editorGraph->addEventHandler< ui::EdgeConnectEvent >(this, &StateGraphEditorPage::eventEdgeConnect);
	m_editorGraph->addEventHandler< ui::EdgeDisconnectEvent >(this, &StateGraphEditorPage::eventEdgeDisconnect);

	// Build popup menu.
	m_menuPopup = new ui::Menu();
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"StateGraph.Editor.Create"), i18n::Text(L"STATEGRAPH_CREATE_STATE")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"STATEGRAPH_DELETE_STATE")));
	m_menuPopup->add(new ui::MenuItem(L"-"));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"StateGraph.Editor.SetRoot"), i18n::Text(L"STATEGRAPH_SET_ROOT")));

	// Create properties view.
	m_propertiesView = m_site->createPropertiesView(parent);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &StateGraphEditorPage::eventPropertiesChanged);
	m_site->createAdditionalPanel(m_propertiesView, ui::dpi96(400), false);

	// Create preview panel.
	m_containerPreview = new ui::Container();
	m_containerPreview->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0, 0));
	m_containerPreview->setText(L"Animation Preview");

	m_toolBarPreview = new ui::ToolBar();
	m_toolBarPreview->create(m_containerPreview);
	m_toolBarPreview->addItem(new ui::ToolBarButton(L"Mesh...", ui::Command(L"StateGraph.Editor.BrowseMesh")));
	m_toolBarPreview->addItem(new ui::ToolBarButton(L"Skeleton...", ui::Command(L"StateGraph.Editor.BrowseSkeleton")));
	m_toolBarPreview->addEventHandler< ui::ToolBarButtonClickEvent >(this, &StateGraphEditorPage::eventToolBarPreviewClick);

	m_previewControl = new AnimationPreviewControl(m_editor);
	m_previewControl->create(m_containerPreview);
	m_previewControl->setPoseController(m_statePreviewController);

	m_previewConditions = new ui::Container();
	m_previewConditions->create(m_containerPreview, ui::WsNone, new ui::TableLayout(L"50%,50%", L"*", 0, 0));

	m_site->createAdditionalPanel(m_containerPreview, ui::dpi96(450), false);

	createEditorNodes(
		m_stateGraph->getStates(),
		m_stateGraph->getTransitions()
	);

	parent->update();
	m_editorGraph->center();

	updateGraph();
	updatePreviewConditions();
	bindStateNodes();

	return true;
}

void StateGraphEditorPage::destroy()
{
	m_site->destroyAdditionalPanel(m_propertiesView);
	m_site->destroyAdditionalPanel(m_containerPreview);

	safeDestroy(m_propertiesView);
	safeDestroy(m_containerPreview);
	safeDestroy(m_editorGraph);
}

bool StateGraphEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	T_ASSERT(primaryType);

	if (is_type_of< Animation >(*primaryType))
	{
		Ref< StateNode > state = new StateNodeAnimation(instance->getName(), resource::IdProxy< Animation >(instance->getGuid()), false);

		ui::Point absolutePosition = m_editorGraph->screenToClient(position) - m_editorGraph->getOffset();
		state->setPosition(std::pair< int, int >(absolutePosition.x, absolutePosition.y));

		m_stateGraph->addState(state);

		createEditorNode(state);
		bindStateNodes();
		updateGraph();
	}
	else if (is_type_of< mesh::MeshAsset >(*primaryType))
	{
		m_previewControl->setMesh(resource::Id< mesh::SkinnedMesh >(instance->getGuid()));
	}
	else if (is_type_of< SkeletonAsset >(*primaryType))
	{
		m_previewControl->setSkeleton(resource::Id< animation::Skeleton >(instance->getGuid()));
	}
	else
		return false;

	return true;
}

bool StateGraphEditorPage::handleCommand(const ui::Command& command)
{
	if (m_propertiesView->handleCommand(command))
		return true;
	
	if (command == L"Editor.SettingsChanged")
	{
		m_previewControl->updateSettings();
		m_previewControl->update();
	}
	//if (command == L"Editor.Cut" || command == L"Editor.Copy")
	//{
	//	RefArray< ui::Node > selectedNodes;
	//	if (m_editorGraph->getSelectedNodes(selectedNodes) > 0)
	//	{
	//		// Also copy edges which are affected by selected nodes.
	//		RefArray< ui::Edge > selectedEdges;
	//		m_editorGraph->getSelectedEdges(selectedEdges, true);

	//		Ref< ShaderGraphEditorClipboardData > data = new ShaderGraphEditorClipboardData();
	//
	//		ui::Rect bounds(0, 0, 0, 0);
	//		for (RefArray< ui::Node >::iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
	//		{
	//			Ref< Node > shaderNode = (*i)->getData< Node >(L"SHADERNODE");
	//			T_ASSERT(shaderNode);
	//			data->addNode(shaderNode);

	//			if (i != selectedNodes.begin())
	//			{
	//				ui::Rect rc = (*i)->calculateRect();
	//				bounds.left = std::min(bounds.left, rc.left);
	//				bounds.top = std::min(bounds.top, rc.top);
	//				bounds.right = std::max(bounds.right, rc.right);
	//				bounds.bottom = std::max(bounds.bottom, rc.bottom);
	//			}
	//			else
	//				bounds = (*i)->calculateRect();
	//		}

	//		data->setBounds(bounds);

	//		for (RefArray< ui::Edge >::iterator i = selectedEdges.begin(); i != selectedEdges.end(); ++i)
	//		{
	//			Ref< Edge > shaderEdge = (*i)->getData< Edge >(L"SHADEREDGE");
	//			T_ASSERT(shaderEdge);
	//			data->addEdge(shaderEdge);
	//		}

	//		ui::Application::getInstance()->getClipboard()->setObject(data);

	//		// Remove edges and nodes from graphs if user cuts.
	//		if (command == L"Editor.Cut")
	//		{
	//			// Save undo state.
	//			m_undoStack->push(m_shaderGraph);

	//			// Remove edges which are connected to any selected node, not only those who connects to both selected end nodes.
	//			selectedEdges.resize(0);
	//			m_editorGraph->getSelectedEdges(selectedEdges, false);

	//			for (RefArray< ui::Edge >::iterator i = selectedEdges.begin(); i != selectedEdges.end(); ++i)
	//			{
	//				m_shaderGraph->removeEdge((*i)->getData< Edge >(L"SHADEREDGE"));
	//				m_editorGraph->removeEdge(*i);
	//			}

	//			for (RefArray< ui::Node >::iterator i = selectedNodes.begin(); i != selectedNodes.end(); ++i)
	//			{
	//				m_shaderGraph->removeNode((*i)->getData< Node >(L"SHADERNODE"));
	//				m_editorGraph->removeNode(*i);
	//			}
	//		}
	//	}
	//}
	//else if (command == L"Editor.Paste")
	//{
	//	Ref< ShaderGraphEditorClipboardData > data = dynamic_type_cast< ShaderGraphEditorClipboardData* >(
	//		ui::Application::getInstance()->getClipboard()->getObject()
	//	);
	//	if (data)
	//	{
	//		// Save undo state.
	//		m_undoStack->push(m_shaderGraph);

	//		const ui::Rect& bounds = data->getBounds();

	//		ui::Size graphSize = m_editorGraph->getInnerRect().getSize();
	//		int centerLeft = (graphSize.cx - bounds.getWidth()) / 2 - m_editorGraph->getOffset().cx;
	//		int centerTop = (graphSize.cy - bounds.getHeight()) / 2 - m_editorGraph->getOffset().cy;

	//		for (RefArray< Node >::const_iterator i = data->getNodes().begin(); i != data->getNodes().end(); ++i)
	//		{
	//			std::pair< int, int > position = (*i)->getPosition();
	//			position.first = (position.first - bounds.left) + centerLeft;
	//			position.second = (position.second - bounds.top) + centerTop;
	//			(*i)->setPosition(position);

	//			// Add node to graph.
	//			m_shaderGraph->addNode(*i);
	//		}

	//		for (RefArray< Edge >::const_iterator i = data->getEdges().begin(); i != data->getEdges().end(); ++i)
	//			m_shaderGraph->addEdge(*i);

	//		createEditorNodes(
	//			data->getNodes(),
	//			data->getEdges()
	//		);
	//		updateGraph();
	//	}
	//}
	/*else*/ if (command == L"Editor.SelectAll")
	{
		m_editorGraph->selectAllNodes();
		updateGraph();
	}
	else if (command == L"Editor.Unselect")
	{
		m_editorGraph->deselectAllNodes();
		updateGraph();
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< ui::Node > nodes;
		if (m_editorGraph->getSelectedNodes(nodes) <= 0)
			return false;

		m_document->push();

		// First remove transitions which are connected to selected states.
		RefArray< ui::Edge > edges;
		m_editorGraph->getConnectedEdges(nodes, false, edges);

		for (auto edge : edges)
		{
			Transition* transition = edge->getData< Transition >(L"TRANSITION");
			m_stateGraph->removeTransition(transition);
			m_editorGraph->removeEdge(edge);
		}

		// Then remove all states.
		for (auto node : nodes)
		{
			StateNode* state = node->getData< StateNode >(L"STATE");
			m_stateGraph->removeState(state);
			m_editorGraph->removeNode(node);
		}

		bindStateNodes();
		updateGraph();
		updatePreviewConditions();
	}
	else if (command == L"Editor.Undo")
	{
		if (m_document->undo())
		{
			Ref< StateGraph > stateGraph = m_document->getObject< StateGraph >(0);
			T_ASSERT(stateGraph);

			m_stateGraph = stateGraph;

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_stateGraph->getStates(),
				m_stateGraph->getTransitions()
			);

			bindStateNodes();
			updateGraph();
			updatePreviewConditions();
		}
	}
	else if (command == L"Editor.Redo")
	{
		if (m_document->redo())
		{
			Ref< StateGraph > stateGraph = m_document->getObject< StateGraph >(0);
			T_ASSERT(stateGraph);

			m_stateGraph = stateGraph;

			m_editorGraph->removeAllEdges();
			m_editorGraph->removeAllNodes();

			createEditorNodes(
				m_stateGraph->getStates(),
				m_stateGraph->getTransitions()
			);

			bindStateNodes();
			updateGraph();
			updatePreviewConditions();
		}
	}
	else if (command == L"StateGraph.Editor.SetRoot")
	{
		RefArray< ui::Node > selectedNodes;
		if (m_editorGraph->getSelectedNodes(selectedNodes) == 1)
		{
			Ref< StateNode > state = selectedNodes.front()->getData< StateNode >(L"STATE");
			T_ASSERT(state);

			m_stateGraph->setRootState(state);

			// Update color to show which node is root.
			for (auto node : m_editorGraph->getNodes())
				node->setShape(new ui::DefaultNodeShape(
					node == selectedNodes.front() ? ui::DefaultNodeShape::StDefault : ui::DefaultNodeShape::StExternal
				));
		}
	}
	else if (command == L"StateGraph.Editor.AlignLeft")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnLeft);
	}
	else if (command == L"StateGraph.Editor.AlignRight")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnRight);
	}
	else if (command == L"StateGraph.Editor.AlignTop")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnTop);
	}
	else if (command == L"StateGraph.Editor.AlignBottom")
	{
		m_document->push();
		m_editorGraph->alignNodes(ui::GraphControl::AnBottom);
	}
	else if (command == L"StateGraph.Editor.EvenSpaceVertically")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsVertically);
	}
	else if (command == L"StateGraph.Editor.EventSpaceHorizontally")
	{
		m_document->push();
		m_editorGraph->evenSpace(ui::GraphControl::EsHorizontally);
	}
	else if (command == L"StateGraph.Editor.BrowseMesh")
	{
		Ref< db::Instance > meshInstance = m_editor->browseInstance(type_of< mesh::MeshAsset >());
		if (meshInstance)
		{
			m_previewControl->setMesh(resource::Id< mesh::SkinnedMesh >(meshInstance->getGuid()));
		}
	}
	else if (command == L"StateGraph.Editor.BrowseSkeleton")
	{
		Ref< db::Instance > skeletonInstance = m_editor->browseInstance(type_of< animation::SkeletonAsset >());
		if (skeletonInstance)
		{
			m_previewControl->setSkeleton(resource::Id< Skeleton >(skeletonInstance->getGuid()));
		}
	}
	else
		return false;

	m_editorGraph->update();
	return true;
}

void StateGraphEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void StateGraphEditorPage::bindStateNodes()
{
	for (auto state : m_stateGraph->getStates())
		state->bind(m_previewControl->getResourceManager());
}

void StateGraphEditorPage::createEditorNodes(const RefArray< StateNode >& states, const RefArray< Transition >& transitions)
{
	std::map< const StateNode*, ui::Node* > nodeMap;

	// Create editor nodes for each state.
	for (auto state : states)
	{
		Ref< ui::Node > node = createEditorNode(state);
		nodeMap[state] = node;
	}

	// Create editor edges for each transition.
	for (auto transition : transitions)
	{
		StateNode* from = transition->from();
		StateNode* to = transition->to();

		ui::Node* fromNode = nodeMap[from];
		ui::Node* toNode = nodeMap[to];

		if (!fromNode || !toNode)
			continue;

		ui::Pin* fromPin = fromNode->findOutputPin(L"Leave");
		T_ASSERT(fromPin);

		ui::Pin* toPin = toNode->findInputPin(L"Enter");
		T_ASSERT(toPin);

		Ref< ui::Edge > transitionEdge = new ui::Edge(fromPin, toPin);
		transitionEdge->setData(L"TRANSITION", transition);
		m_editorGraph->addEdge(transitionEdge);
	}
}

Ref< ui::Node > StateGraphEditorPage::createEditorNode(StateNode* state)
{
	Ref< ui::INodeShape > shape = new ui::DefaultNodeShape(
		m_stateGraph->getRootState() == state ? ui::DefaultNodeShape::StDefault : ui::DefaultNodeShape::StExternal
	);

	Ref< ui::Node > node = m_editorGraph->createNode(
		state->getName(),
		L"",
		ui::Point(
			state->getPosition().first,
			state->getPosition().second
		),
		shape
	);
	node->setData(L"STATE", state);
	node->createInputPin(L"Enter", true);
	node->createOutputPin(L"Leave");

	return node;
}

void StateGraphEditorPage::createState(const ui::Point& at)
{
	Ref< StateNode > state = new StateNodeAnimation(i18n::Text(L"STATEGRAPH_UNNAMED"), resource::IdProxy< Animation >(), false);
	state->setPosition(std::pair< int, int >(at.x, at.y));
	m_stateGraph->addState(state);

	createEditorNode(state);
	bindStateNodes();
	updateGraph();
}

void StateGraphEditorPage::updateGraph()
{
	m_editorGraph->update();
}

void StateGraphEditorPage::updatePreviewConditions()
{
	std::map< std::wstring, bool > conditions;

	// Collect all condition variables.
	for (auto transition : m_stateGraph->getTransitions())
	{
		std::wstring c = transition->getCondition();
		if (!c.empty())
		{
			if (c[0] == L'!')
				c = c.substr(1);

			conditions[c] = false;
		}
	}

	// Keep all existing condition states.
	for (ui::Widget* it = m_previewConditions->getFirstChild(); it; it = it->getNextSibling())
	{
		ui::CheckBox* condition = mandatory_non_null_type_cast< ui::CheckBox* >(it);
		if (conditions.find(condition->getText()) != conditions.end())
			conditions[condition->getText()] = condition->isChecked();
	}

	// Destroy all checkboxes.
	while (m_previewConditions->getFirstChild())
		m_previewConditions->getFirstChild()->destroy();

	// Recreate checkboxes.
	for (auto i = conditions.begin(); i != conditions.end(); ++i)
	{
		Ref< ui::CheckBox > cb = new ui::CheckBox();
		cb->create(m_previewConditions, i->first, i->second);
		cb->addEventHandler< ui::ButtonClickEvent >(this, &StateGraphEditorPage::eventPreviewConditionClick);
	}

	m_containerPreview->update();
}

void StateGraphEditorPage::eventToolBarGraphClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void StateGraphEditorPage::eventToolBarPreviewClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void StateGraphEditorPage::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_menuPopup->showModal(m_editorGraph, event->getPosition());
	if (!selected)
		return;

	const ui::Command& command = selected->getCommand();

	if (command == L"StateGraph.Editor.Create")
		createState(event->getPosition() - m_editorGraph->getOffset());
	else
		handleCommand(command);
}

void StateGraphEditorPage::eventSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::Node > nodes;
	RefArray< ui::Edge > edges;

	if (m_editorGraph->getSelectedNodes(nodes) == 1)
	{
		StateNode* state = nodes[0]->getData< StateNode >(L"STATE");
		T_ASSERT(state);

		m_propertiesView->setPropertyObject(state);
		m_previewControl->setPoseController(new StateNodeController(state));
	}
	else if (m_editorGraph->getSelectedEdges(edges) == 1)
	{
		Transition* transition = edges[0]->getData< Transition >(L"TRANSITION");
		T_ASSERT(transition);

		m_propertiesView->setPropertyObject(transition);
		m_previewControl->setPoseController(m_statePreviewController);
	}
	else
	{
		m_propertiesView->setPropertyObject(0);
		m_previewControl->setPoseController(m_statePreviewController);
	}
}

void StateGraphEditorPage::eventNodeMoved(ui::NodeMovedEvent* event)
{
	ui::Node* node = event->getNode();
	T_ASSERT(node);

	// Get state from editor node.
	StateNode* state = node->getData< StateNode >(L"STATE");
	T_ASSERT(state);

	ui::Point position = node->getPosition();
	if (position.x != state->getPosition().first || position.y != state->getPosition().second)
	{
		state->setPosition(std::pair< int, int >(
			node->getPosition().x,
			node->getPosition().y
		));
	}

	// Update properties.
	if (node->isSelected())
		m_propertiesView->setPropertyObject(state);
}

void StateGraphEditorPage::eventEdgeConnect(ui::EdgeConnectEvent* event)
{
	ui::Edge* edge = event->getEdge();

	ui::Pin* leavePin = edge->getSourcePin();
	T_ASSERT(leavePin);

	ui::Pin* enterPin = edge->getDestinationPin();
	T_ASSERT(enterPin);

	StateNode* leaveState = leavePin->getNode()->getData< StateNode >(L"STATE");
	T_ASSERT(leaveState);

	StateNode* enterState = enterPin->getNode()->getData< StateNode >(L"STATE");
	T_ASSERT(enterState);

	Ref< Transition > transition = new Transition(leaveState, enterState);
	m_stateGraph->addTransition(transition);

	edge->setData(L"TRANSITION", transition);
	m_editorGraph->addEdge(edge);

	updateGraph();
}

void StateGraphEditorPage::eventEdgeDisconnect(ui::EdgeDisconnectEvent* event)
{
	Ref< ui::Edge > edge = event->getEdge();

	Transition* transition = checked_type_cast< Transition* >(edge->getData(L"TRANSITION"));
	m_stateGraph->removeTransition(transition);

	updateGraph();
}

void StateGraphEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	// Refresh editor nodes.
	for (auto node : m_editorGraph->getNodes())
	{
		StateNode* state = node->getData< StateNode >(L"STATE");
		node->setTitle(state->getName());

		const auto& position = state->getPosition();
		node->setPosition(ui::Point(
			position.first,
			position.second
		));
	}

	updateGraph();
	updatePreviewConditions();
}

void StateGraphEditorPage::eventPreviewConditionClick(ui::ButtonClickEvent* event)
{
	ui::CheckBox* cb = mandatory_non_null_type_cast< ui::CheckBox* >(event->getSender());
	m_statePreviewController->setCondition(cb->getText(), cb->isChecked(), false);
}

	}
}
