#include <deque>
#include <set>
#include "Core/Log/Log.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphHash.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphOrderEvaluator.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct CopyVisitor
{
	Ref< ShaderGraph > m_shaderGraph;

	bool operator () (Node* node)
	{
		m_shaderGraph->addNode(node);
		return true;
	}

	bool operator () (Edge* edge)
	{
		m_shaderGraph->addEdge(edge);
		return true;
	}
};

bool insideCycle(const ShaderGraph* shaderGraph, const OutputPin* outputPin)
{
	// No input pins on node means it cannot be part of a cycle.
	if (outputPin->getNode()->getInputPinCount() <= 0)
		return false;

	std::deque< const OutputPin* > scanOutputPins;
	std::set< const OutputPin* > scannedOutputPins;

	scanOutputPins.push_back(outputPin);
	scannedOutputPins.insert(outputPin);

	while (!scanOutputPins.empty())
	{
		const OutputPin* scanOutputPin = scanOutputPins.front();
		T_ASSERT(scanOutputPin);

		scanOutputPins.pop_front();

		const Node* node = scanOutputPin->getNode();
		T_ASSERT(node);

		int32_t inputPinCount = node->getInputPinCount();
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT(inputPin);

			const OutputPin* sourceOutputPin = shaderGraph->findSourcePin(inputPin);
			if (sourceOutputPin)
			{
				if (sourceOutputPin == outputPin)
					return true;

				if (scannedOutputPins.find(sourceOutputPin) == scannedOutputPins.end())
				{
					scanOutputPins.push_back(sourceOutputPin);
					scannedOutputPins.insert(sourceOutputPin);
				}
			}
		}
	}

	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphOptimizer", ShaderGraphOptimizer, Object)

ShaderGraphOptimizer::ShaderGraphOptimizer(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_insertedCount(0)
,	m_frequentUniformsAsLinear(false)
{
}

Ref< ShaderGraph > ShaderGraphOptimizer::removeUnusedBranches() const
{
	RefArray< Node > roots;
	for (auto node : m_shaderGraph->getNodes())
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		if (nodeTraits && nodeTraits->isRoot(m_shaderGraph, node))
			roots.push_back(node);
	}

	CopyVisitor visitor;
	visitor.m_shaderGraph = new ShaderGraph();
	GraphTraverse(m_shaderGraph, roots).preorder(visitor);
	return visitor.m_shaderGraph;
}

Ref< ShaderGraph > ShaderGraphOptimizer::mergeBranches() const
{
	uint32_t mergedNodes = 0;

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	// Keep reference to array as we assume it will shrink
	// when we remove nodes.
	const RefArray< Node >& nodes = shaderGraph->getNodes();
	if (nodes.empty())
		return shaderGraph;

	// Calculate node hashes.
	SmallMap< const Node*, uint32_t > hash;
	for (auto node : nodes)
		hash[node] = ShaderGraphHash::calculate(node);

	// Merge single output nodes.
	for (uint32_t i = 0; i < uint32_t(nodes.size() - 1); ++i)
	{
		if (nodes[i]->getInputPinCount() != 0 || nodes[i]->getOutputPinCount() != 1)
			continue;

		const TypeInfo* type0 = &type_of(nodes[i]);
		const uint32_t hash0 = hash[nodes[i]];

		for (uint32_t j = i + 1; j < uint32_t(nodes.size()); )
		{
			if (nodes[j]->getInputPinCount() != 0 || nodes[j]->getOutputPinCount() != 1)
			{
				++j;
				continue;
			}

			if (type0 != &type_of(nodes[j]) || hash0 != hash[nodes[j]])
			{
				++j;
				continue;
			}

			// Identical nodes found; rewire edges.
			RefSet< Edge > edges;
			if (shaderGraph->findEdges(nodes[j]->getOutputPin(0), edges) > 0)
			{
				for (RefSet< Edge >::const_iterator k = edges.begin(); k != edges.end(); ++k)
				{
					Ref< Edge > edge = new Edge(
						nodes[i]->getOutputPin(0),
						(*k)->getDestination()
					);

					shaderGraph->removeEdge(*k);
					shaderGraph->addEdge(edge);
				}
			}

			// Remove redundant node.
			T_ASSERT(nodes[j]->getInputPinCount() == 0);
			T_ASSERT(nodes[j]->getOutputPinCount() == 1);
			shaderGraph->removeNode(nodes[j]);
			mergedNodes++;
		}
	}

	// Merge nodes which have same input signature; ie. are completely
	// connected to same nodes.
	for (;;)
	{
		bool merged = false;

		// Merge nodes; build array of nodes to remove.
		for (uint32_t i = 0; i < (uint32_t)(nodes.size() - 1); ++i)
		{
			const TypeInfo* type0 = &type_of(nodes[i]);
			const uint32_t hash0 = hash[nodes[i]];
			const int32_t inputPinCount = nodes[i]->getInputPinCount();

			for (uint32_t j = i + 1; j < (uint32_t)(nodes.size()); )
			{
				if (type0 != &type_of(nodes[j]) || hash0 != hash[nodes[j]])
				{
					++j;
					continue;
				}

				T_ASSERT(inputPinCount == nodes[j]->getInputPinCount());

				int32_t outputPinCount = nodes[i]->getOutputPinCount();
				T_ASSERT(outputPinCount == nodes[j]->getOutputPinCount());

				// Are both nodes wired to same input nodes?
				bool wiredIdentical = true;
				for (int32_t k = 0; k < inputPinCount; ++k)
				{
					const OutputPin* sourcePin1 = shaderGraph->findSourcePin(nodes[i]->getInputPin(k));
					const OutputPin* sourcePin2 = shaderGraph->findSourcePin(nodes[j]->getInputPin(k));
					if (sourcePin1 != sourcePin2)
					{
						wiredIdentical = false;
						break;
					}
				}
				if (!wiredIdentical)
				{
					++j;
					continue;
				}

				// Identically wired nodes found; rewire output edges.
				for (int32_t k = 0; k < outputPinCount; ++k)
				{
					RefSet< Edge > edges;
					shaderGraph->findEdges(nodes[j]->getOutputPin(k), edges);
					for (RefSet< Edge >::const_iterator m = edges.begin(); m != edges.end(); ++m)
					{
						Ref< Edge > edge = new Edge(
							nodes[i]->getOutputPin(k),
							(*m)->getDestination()
						);
						shaderGraph->removeEdge(*m);
						shaderGraph->addEdge(edge);
					}
				}

				// Remove input edges.
				for (int32_t k = 0; k < inputPinCount; ++k)
				{
					Ref< Edge > edge = shaderGraph->findEdge(nodes[j]->getInputPin(k));
					if (edge)
						shaderGraph->removeEdge(edge);
				}

				// Remove node; should be completely disconnected now.
				shaderGraph->removeNode(nodes[j]);

				merged = true;
				mergedNodes++;
			}
		}

		if (!merged)
			break;
	}

	T_DEBUG(L"Merged " << mergedNodes << L" node(s)");
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphOptimizer::insertInterpolators(bool frequentUniformsAsLinear) const
{
	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	m_frequentUniformsAsLinear = frequentUniformsAsLinear;
	m_insertedCount = 0;

	RefArray< PixelOutput > pixelOutputNodes;
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputNodes);

	m_visited.clear();
	for (auto pixelOutputNode : pixelOutputNodes)
		insertInterpolators(shaderGraph, pixelOutputNode);

	return shaderGraph;
}

void ShaderGraphOptimizer::insertInterpolators(ShaderGraph* shaderGraph, Node* node) const
{
	// Should never reach vertex inputs.
	T_FATAL_ASSERT(!is_a< VertexInput >(node));

	// If we've reached an (manually placed) interpolator
	// then stop even if order to high.
	if (is_a< Interpolator >(node))
		return;

	// If we've already visited source node there is no
	// place to put an interpolator as we're in a loop.
	if (m_visited.find(node) != m_visited.end())
		return;

	m_visited.insert(node);

	for (int32_t i = 0; i < node->getInputPinCount(); ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		T_ASSERT(inputPin);

		const OutputPin* sourceOutputPin = shaderGraph->findSourcePin(inputPin);
		if (!sourceOutputPin)
			continue;

		Ref< Node > sourceNode = sourceOutputPin->getNode();
		T_ASSERT(sourceNode);

		bool isSwizzle = is_a< Swizzle >(sourceNode);
		bool vertexMandatory = is_a< VertexInput >(sourceNode);
		bool inCycle = insideCycle(shaderGraph, sourceOutputPin);

		PinOrderType inputOrder = PotConstant;
		if (!vertexMandatory)
			inputOrder = ShaderGraphOrderEvaluator(shaderGraph, m_frequentUniformsAsLinear).evaluate(sourceOutputPin);

		if (vertexMandatory || (!isSwizzle && !inCycle && inputOrder <= PotLinear))
		{
			// We've reached low enough order; insert interpolator if linear and stop.
			if (vertexMandatory || inputOrder == PotLinear)
			{
				// Remove edge; replace with interpolator.
				Ref< Edge > edge = shaderGraph->findEdge(inputPin);
				T_ASSERT(edge);

				shaderGraph->removeEdge(edge);
				edge = nullptr;

				// If this output pin already connected to an interpolator node then we reuse it.
				RefSet< Edge > outputEdges;
				shaderGraph->findEdges(sourceOutputPin, outputEdges);
				for (auto outputEdge : outputEdges)
				{
					Ref< Node > targetNode = outputEdge->getDestination()->getNode();
					if (is_a< Interpolator >(targetNode))
					{
						edge = new Edge(targetNode->getOutputPin(0), inputPin);
						break;
					}
				}

				if (edge)
					shaderGraph->addEdge(edge);
				else
				{
					// Create new interpolator node.
					Ref< Interpolator > interpolator = new Interpolator();
					std::pair< int, int > position = sourceNode->getPosition(); position.first += 64;
					interpolator->setPosition(position);
					shaderGraph->addNode(interpolator);

					// Create new edges.
					edge = new Edge(sourceOutputPin, interpolator->getInputPin(0));
					shaderGraph->addEdge(edge);

					edge = new Edge(interpolator->getOutputPin(0), inputPin);
					shaderGraph->addEdge(edge);

					m_insertedCount++;
				}
			}
		}
		else
		{
			// Input still have too high order; need to keep in pixel shader.
			insertInterpolators(shaderGraph, sourceNode);
		}
	}
}

	}
}
