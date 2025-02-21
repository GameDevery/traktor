/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <set>
#include "Core/Log/Log.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"

namespace traktor::render
{
	namespace
	{

Ref< ShaderGraph > replaceBranch(const ShaderGraph* shaderGraph, Branch* branch, bool path)
{
	Ref< ShaderGraph > shaderGraphResult = new ShaderGraph(
		shaderGraph->getNodes(),
		shaderGraph->getEdges()
	);

	const InputPin* inputPin = branch->getInputPin(path ? /* True */ 0 : /* False */ 1);
	T_ASSERT(inputPin);

	const OutputPin* outputPin = branch->getOutputPin(/* Output */ 0);
	T_ASSERT(outputPin);

	Ref< Edge > sourceEdge = shaderGraphResult->findEdge(inputPin);
	if (!sourceEdge)
		return nullptr;

	RefArray< Edge > destinationEdges = shaderGraphResult->findEdges(outputPin);

	shaderGraphResult->removeEdge(sourceEdge);
	for (auto destinationEdge : destinationEdges)
	{
		shaderGraphResult->removeEdge(destinationEdge);
		shaderGraphResult->addEdge(new Edge(
			sourceEdge->getSource(),
			destinationEdge->getDestination()
		));
	}

	shaderGraphResult = ShaderGraphOptimizer(shaderGraphResult).removeUnusedBranches(true);
	if (!shaderGraphResult)
		return nullptr;

	return shaderGraphResult;
}

void buildPermutations(
	const ShaderGraph* shaderGraph,
	const Guid& shaderGraphId,
	const AlignedVector< std::wstring >& parameterNames,
	uint32_t parameterMask,
	uint32_t parameterValue,
	AlignedVector< ShaderGraphCombinations::Combination >& outCombinations
)
{
	RefArray< Branch > branchNodes = shaderGraph->findNodesOf< Branch >();
	if (!branchNodes.empty())
	{
		Branch* branch = branchNodes.front();
		T_ASSERT(branch);

		const auto parameterIter = std::find(parameterNames.begin(), parameterNames.end(), branch->getParameterName());
		const uint32_t parameterBit = 1 << (uint32_t)std::distance(parameterNames.begin(), parameterIter);

		Ref< ShaderGraph > shaderGraphBranchTrue = replaceBranch(shaderGraph, branch, true);
		if (shaderGraphBranchTrue)
		{
			buildPermutations(
				shaderGraphBranchTrue,
				shaderGraphId,
				parameterNames,
				parameterMask | parameterBit,
				parameterValue | parameterBit,
				outCombinations
			);
		}

		Ref< ShaderGraph > shaderGraphBranchFalse = replaceBranch(shaderGraph, branch, false);
		if (shaderGraphBranchFalse)
		{
			buildPermutations(
				shaderGraphBranchFalse,
				shaderGraphId,
				parameterNames,
				parameterMask | parameterBit,
				parameterValue,
				outCombinations
			);
		}
	}
	else
	{
		const auto it = std::find_if(outCombinations.begin(), outCombinations.end(), [=](const ShaderGraphCombinations::Combination& c) {
			return c.mask == parameterMask && c.value == parameterValue;
		});
		if (it == outCombinations.end())
		{
			// Check with constant folding to see if this entire graph should be discarded, ie PixelOutput disabled etc.
			Ref< ShaderGraph > shaderGraphNoBranch = ShaderGraphStatic(shaderGraph, shaderGraphId).getConstantFolded();
			if (shaderGraphNoBranch)
			{
				ShaderGraphCombinations::Combination c;
				c.mask = parameterMask;
				c.value = parameterValue;
				c.shaderGraph = shaderGraph;
				outCombinations.push_back(c);
			}
		}
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphCombinations", ShaderGraphCombinations, Object)

ShaderGraphCombinations::ShaderGraphCombinations(const ShaderGraph* shaderGraph, const Guid& shaderGraphId)
{
	m_shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches(true);

	RefArray< Branch > branchNodes = m_shaderGraph->findNodesOf< Branch >();

	std::set< std::wstring > parameterNames;
	for (auto branchNode : branchNodes)
	{
		const std::wstring name = branchNode->getParameterName();
		if (parameterNames.find(name) != parameterNames.end())
			continue;

		m_parameterNames.push_back(name);
		parameterNames.insert(name);
	}

	buildPermutations(m_shaderGraph, shaderGraphId, m_parameterNames, 0, 0, m_combinations);
}

const AlignedVector< std::wstring >& ShaderGraphCombinations::getParameterNames() const
{
	return m_parameterNames;
}

AlignedVector< std::wstring > ShaderGraphCombinations::getParameterNames(uint32_t mask) const
{
	AlignedVector< std::wstring > parameterNames;
	for (uint32_t i = 0; i < uint32_t(m_parameterNames.size()); ++i)
	{
		if ((mask & (1 << i)) != 0)
			parameterNames.push_back(m_parameterNames[i]);
	}
	return parameterNames;
}

uint32_t ShaderGraphCombinations::getCombinationCount() const
{
	return (uint32_t)m_combinations.size();
}

uint32_t ShaderGraphCombinations::getCombinationMask(uint32_t index) const
{
	T_ASSERT(index < m_combinations.size());
	return m_combinations[index].mask;
}

uint32_t ShaderGraphCombinations::getCombinationValue(uint32_t index) const
{
	T_ASSERT(index < m_combinations.size());
	return m_combinations[index].value;
}

Ref< const ShaderGraph > ShaderGraphCombinations::getCombinationShaderGraph(uint32_t index) const
{
	T_ASSERT(index < m_combinations.size());
	return m_combinations[index].shaderGraph;
}

}
