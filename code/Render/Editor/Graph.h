#pragma once

#include "Core/RefArray.h"
#include "Core/RefSet.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

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

class Node;
class Edge;
class OutputPin;
class InputPin;

/*! Generic node/edge-style graph.
 * \ingroup Render
 */
class T_DLLCLASS Graph : public ISerializable
{
	T_RTTI_CLASS;

public:
	Graph();

	Graph(const RefArray< Node >& nodes, const RefArray< Edge >& edges);

	virtual ~Graph();

	/*! Add node to graph.
	 *
	 * \param node Node to add.
	 */
	void addNode(Node* node);

	/*! Remove node from graph.
	 *
	 * \param node Node to remove.
	 */
	void removeNode(Node* node);

	/*! Add edge to graph.
	 *
	 * \param edge Edge to add.
	 */
	void addEdge(Edge* edge);

	/*! Remove edge from graph.
	 *
	 * \param edge Edge to remove.
	 */
	void removeEdge(Edge* edge);

	/*! Remove all nodes and edges from graph. */
	void removeAll();

	/*! Get all nodes of a specific type.
	 *
	 * \param nodeType Type of node to find.
	 * \param outNodes Nodes of given type.
	 * \return Number of nodes.
	 */
	size_t findNodesOf(const TypeInfo& nodeType, RefArray< Node >& outNodes) const;

	/*! Get all nodes of a specific type.
	 *
	 * \param nodeType Type of node to find.
	 * \param outNodes Nodes of given type.
	 * \return Number of nodes.
	 */
	template < typename NodeType >
	size_t findNodesOf(RefArray< NodeType >& outNodes) const
	{
		RefArray< Node > untypedNodes;
		findNodesOf(type_of< NodeType >(), untypedNodes);

		outNodes.resize(untypedNodes.size());
		for (size_t i = 0; i < untypedNodes.size(); ++i)
			outNodes[i] = checked_type_cast< NodeType* >(untypedNodes[i]);

		return outNodes.size();
	}

	/*! Find edge connected to input pin.
	 *
	 * \param inputPin Input pin.
	 * \return Connected edge.
	 */
	Edge* findEdge(const InputPin* inputPin) const;

	/*! Find edges connected from output pin.
	 *
	 * \param outputPin Output pin.
	 * \param outEdges Connected edges.
	 * \return Number of connected edges.
	 */
	uint32_t findEdges(const OutputPin* outputPin, RefSet< Edge >& outEdges) const;

	/*! Find output pin connected to input pin.
	 *
	 * \param inputPin Input pin.
	 * \return Connected output pin.
	 */
	const OutputPin* findSourcePin(const InputPin* inputPin) const;

	/*! Find all input pins which are connected to output pin.
	 *
	 * \param outputPin Output pin.
	 * \param outDestinations Connected input pins.
	 * \return Number of connected input pins.
	 */
	uint32_t findDestinationPins(const OutputPin* outputPin, AlignedVector< const InputPin* >& outDestinations) const;

	/*! Get number of destination pins connected to an output pin.
	 *
	 * \param outputPin Output pin.
	 * \return Number of connected destinations.
	 */
	uint32_t getDestinationCount(const OutputPin* outputPin) const;

	/*! Detach node from all input and output edges.
	 *
	 * \param node Node to be detached.
	 */
	void detach(const Node* node);

	/*! Rewire all edges from one output to another output.
	 *
	 * \param outputPin Current output pin.
	 * \param newOutputPin New output pin.
	 */
	void rewire(const OutputPin* outputPin, const OutputPin* newOutputPin);

	/*! Serialize graph. */
	virtual void serialize(ISerializer& s) override final;

	/*! Get all nodes.
	 *
	 * \return Array of nodes.
	 */
	const RefArray< Node >& getNodes() const { return m_nodes; }

	/*! Get all edges.
	 *
	 * \return Array of edges.
	 */
	const RefArray< Edge >& getEdges() const { return m_edges; }

private:
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
	SmallMap< const InputPin*, Edge* > m_inputPinToEdge;
	SmallMap< const OutputPin*, uint32_t > m_outputPinDestinationCount;

	void updateInputPinToEdge();

	void updateOutputPinDestinationCount();
};

	}
}
