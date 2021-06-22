#include <string>
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Node.h"
#include "Sound/Processor/OutputPin.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GraphEvaluator", GraphEvaluator, Object)

bool GraphEvaluator::create(const Graph* graph)
{
	m_graph = graph;
	for (auto node : graph->getNodes())
	{
		Ref< ISoundBufferCursor > nodeCursor = node->createCursor();
		if (!nodeCursor)
		{
			log::error << L"Node \"" << type_name(node) << L"\" failed; no cursor." << Endl;
			return false;
		}

		m_nodeCursors[node] = nodeCursor;
	}
	m_timer.reset();
	return true;
}

void GraphEvaluator::setParameter(handle_t id, float parameter)
{
	for (auto it : m_nodeCursors)
		it.second->setParameter(id, parameter);
}

bool GraphEvaluator::evaluateScalar(const OutputPin* producerPin, float& outScalar) const
{
	const Node* producerNode = producerPin->getNode();
	T_ASSERT(producerNode != nullptr);

	ISoundBufferCursor* producerCursor = m_nodeCursors[producerNode];
	if (!producerCursor)
		return false;

	if (!producerNode->getScalar(producerCursor, this, outScalar))
		return false;

	return true;
}

bool GraphEvaluator::evaluateScalar(const InputPin* consumerPin, float& outScalar) const
{
	const OutputPin* producerPin = m_graph->findSourcePin(consumerPin);
	if (producerPin)
		return evaluateScalar(producerPin, outScalar);
	else
		return false;
}

bool GraphEvaluator::evaluateBlock(const OutputPin* producerPin, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	const Node* producerNode = producerPin->getNode();
	T_ASSERT(producerNode != nullptr);

	ISoundBufferCursor* producerCursor = m_nodeCursors[producerNode];
	if (!producerCursor)
		return false;

	const uint32_t consumerCount = m_graph->getDestinationCount(producerPin);
	if (consumerCount == 1)
	{
		if (!producerNode->getBlock(producerCursor, this, mixer, outBlock))
			return false;
	}
	else if (consumerCount >= 2)
	{
		// Need to buffer output of node since we have multiple consumers.
		auto it = m_cachedBlocks.find(producerPin);
		if (it != m_cachedBlocks.end())
		{
			if (consumerCount == 2)
				outBlock = it->second;
			else
			{
				auto copiedBlock = copyBlock(it->second);
				if (!copiedBlock)
					return false;

				outBlock = *copiedBlock;		
			}
		}
		else
		{
			if (!producerNode->getBlock(producerCursor, this, mixer, outBlock))
				return false;

			auto copiedBlock = copyBlock(outBlock);
			if (!copiedBlock)
				return false;

			m_cachedBlocks[producerPin] = *copiedBlock;
		}
	}

	return true;
}

bool GraphEvaluator::evaluateBlock(const InputPin* consumerPin, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	const OutputPin* producerPin = m_graph->findSourcePin(consumerPin);
	if (producerPin)
		return evaluateBlock(producerPin, mixer, outBlock);
	else
		return false;
}

float GraphEvaluator::getTime() const
{
	return float(m_timer.getElapsedTime());
}

void GraphEvaluator::flushCachedBlocks()
{
	for (auto& block : m_blocks)
	{
		for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
		{
			if (block.samples[i])
				Alloc::freeAlign(block.samples[i]);
		}
	}
	m_blocks.clear();
	m_cachedBlocks.reset();
}

SoundBlock* GraphEvaluator::copyBlock(const SoundBlock& sourceBlock) const
{
	if (m_blocks.full())
		return nullptr;

	SoundBlock& block = m_blocks.push_back();

	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
	{
		if (sourceBlock.samples[i])
		{
			block.samples[i] = (float*)Alloc::acquireAlign(sourceBlock.samplesCount * sizeof(float), 16, T_FILE_LINE);
			std::memcpy(block.samples[i], sourceBlock.samples[i], sourceBlock.samplesCount * sizeof(float));
		}
		else
			block.samples[i] = nullptr;
	}

	block.samplesCount = sourceBlock.samplesCount;
	block.sampleRate = sourceBlock.sampleRate;
	block.maxChannel = sourceBlock.maxChannel;
	block.category = sourceBlock.category;

	return &block;
}

	}
}
