#pragma once

#include <functional>
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallSet.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Graph.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! Graph traversal helper.
 * \ingroup Render
 */
class GraphTraverse
{
public:
	GraphTraverse(const Graph* graph, Node* root)
	:	m_graph(graph)
	{
		m_roots.push_back(root);
	}

	GraphTraverse(const Graph* graph, const RefArray< Node >& roots)
	:	m_graph(graph)
	,	m_roots(roots)
	{
	}

	template < typename VisitorType >
	void preorder(VisitorType& visitor)
	{
		SmallSet< Node* > visited;
		for (RefArray< Node >::const_iterator i = m_roots.begin(); i != m_roots.end(); ++i)
			preorderImpl(*i, visitor, visited);
	}

	template < typename VisitorType >
	void postorder(VisitorType& visitor)
	{
		SmallSet< Node* > visited;
		for (RefArray< Node >::const_iterator i = m_roots.begin(); i != m_roots.end(); ++i)
			postorderImpl(*i, visitor, visited);
	}

	void preorder(const std::function< bool(const Node*) >& fn)
	{
		LambdaVisitor visitor(fn);
		preorder(visitor);
	}

	void postorder(const std::function< bool(const Node*) >& fn)
	{
		LambdaVisitor visitor(fn);
		postorder(visitor);
	}

	template < typename VisitorType >
	void breadthFirst(VisitorType& visitor)
	{
		SmallSet< Node* > visited;
		breadthFirstImpl(m_roots, visitor, visited);
	}

private:
	const Graph* m_graph;
	RefArray< Node > m_roots;

	class LambdaVisitor
	{
	public:
		LambdaVisitor(const std::function< bool(const Node*) >& fn)
		:	m_fn(fn)
		{
		}

		bool operator () (const Node* node)
		{
			return m_fn(node);
		}

		bool operator () (const Edge* edge)
		{
			return true;
		}

	private:
		std::function< bool(const Node*) > m_fn;
	};

	template < typename VisitorType >
	void preorderImpl(Node* node, VisitorType& visitor, SmallSet< Node* >& visited)
	{
		if (visited.find(node) != visited.end())
			return;
		visited.insert(node);

		if (!visitor(node))
			return;

		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT(inputPin);

			Edge* edge = m_graph->findEdge(inputPin);
			if (edge)
			{
				if (!visitor(edge))
					continue;

				preorderImpl(edge->getSource()->getNode(), visitor, visited);
			}
		}
	}

	template < typename VisitorType >
	void postorderImpl(Node* node, VisitorType& visitor, SmallSet< Node* >& visited)
	{
		if (visited.find(node) != visited.end())
			return;
		visited.insert(node);

		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT(inputPin);

			Edge* edge = m_graph->findEdge(inputPin);
			if (edge)
			{
				if (!visitor(edge))
					continue;

				postorderImpl(edge->getSource()->getNode(), visitor, visited);
			}
		}

		if (!visitor(node))
			return;
	}

	template < typename VisitorType >
	void breadthFirstImpl(const RefArray< Node >& nodes, VisitorType& visitor, SmallSet< Node* >& visited)
	{
		RefArray< Node > children;
		for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			if (visited.find(*i) != visited.end())
				continue;
			visited.insert(*i);

			if (!visitor(*i))
				continue;

			uint32_t inputPinCount = (*i)->getInputPinCount();
			for (uint32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = (*i)->getInputPin(j);
				T_ASSERT(inputPin);

				Edge* edge = m_graph->findEdge(inputPin);
				if (edge)
				{
					if (!visitor(edge))
						continue;

					children.push_back(edge->getSource()->getNode());
				}
			}
		}
		if (!children.empty())
			breadthFirstImpl(children, visitor, visited);
	}
};

struct FindInputPin
{
	const InputPin* inputPin;
	bool found;

	FindInputPin()
	:	inputPin(0)
	,	found(false)
	{
	}

	bool operator () (Node* node)
	{
		found |= bool(inputPin->getNode() == node);
		return !found;
	}

	bool operator () (Edge* edge)
	{
		return !found;
	}
};

struct PinsConnected
{
	const InputPin* inputPin;
	const OutputPin* outputPin;
	bool connected;

	PinsConnected()
	:	inputPin(0)
	,	outputPin(0)
	,	connected(false)
	{
	}

	bool operator () (Node* node)
	{
		return !connected;
	}

	bool operator () (Edge* edge)
	{
		// Don't traverse paths from source node from wrong input pin.
		if (
			edge->getDestination()->getNode() == inputPin->getNode() &&
			edge->getDestination() != inputPin
		)
			return false;

		connected |= bool(outputPin == edge->getSource());
		return true;
	}
};

struct CollectOutputs
{
	const InputPin* inputPin;
	AlignedVector< const OutputPin* > outputPins;

	CollectOutputs()
	:	inputPin(0)
	{
	}

	bool operator () (Node* node)
	{
		return true;
	}

	bool operator () (Edge* edge)
	{
		// Don't traverse paths from source node from wrong input pin.
		if (
			edge->getDestination()->getNode() == inputPin->getNode() &&
			edge->getDestination() != inputPin
		)
			return false;

		outputPins.push_back(edge->getSource());
		return true;
	}
};

/*! Traverse shader graph nodes through a root set.
 * \ingroup Render
 *
 * \param graph Shader graph.
 * \param roots Root nodes.
 * \param visitor Node and edge visitor.
 */
template < typename VisitorType >
void graphTraverse(const Graph* graph, const RefArray< Node >& roots, VisitorType& visitor)
{
	GraphTraverse(graph, roots).preorder(visitor);
}

/*! Check if an input propagate to a target node.
 * \ingroup Render
 */
bool T_DLLCLASS doesInputPropagateToNode(const Graph* graph, const InputPin* inputPin, Node* targetNode);

/*! Check if two pins are connected.
 * \ingroup Render
 */
bool T_DLLCLASS arePinsConnected(const Graph* graph, const OutputPin* outputPin, const InputPin* inputPin);

/*! Get merging, common, outputs from a set of input pins.
 * \ingroup Render
 */
void T_DLLCLASS getMergingOutputs(const Graph* graph, const AlignedVector< const InputPin* >& inputPins, AlignedVector< const OutputPin* >& outMergedOutputPins);

/*! Get non-dependent outputs from a an input and a set of dependent output pins.
 * \ingroup Render
 */
void T_DLLCLASS getNonDependentOutputs(const Graph* graph, const InputPin* inputPin, const AlignedVector< const OutputPin* >& dependentOutputPins, AlignedVector< const OutputPin* >& outOutputPins);

	}
}
