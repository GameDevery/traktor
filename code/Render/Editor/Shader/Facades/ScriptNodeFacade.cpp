/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Facades/ScriptNodeFacade.h"

#include "Core/Misc/SafeDestroy.h"
#include "I18N/Text.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Ui/Application.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/Pin.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScriptNodeFacade", ScriptNodeFacade, INodeFacade)

ScriptNodeFacade::ScriptNodeFacade(ShaderGraphEditorPage* page)
	: m_page(page)
{
	m_nodeShape = new ui::DefaultNodeShape(ui::DefaultNodeShape::StScript);
}

Ref< Node > ScriptNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor)
{
	Ref< Script > sc = new Script();
	sc->setName(L"Unnamed");
	sc->addInputPin(Guid::create(), L"Input");
	sc->addOutputPin(Guid::create(), L"Output", ParameterType::Scalar);
	sc->setScript(L"$Output = $Input;\n");
	return sc;
}

Ref< ui::Node > ScriptNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
	Script* scriptNode = checked_type_cast< Script*, false >(shaderNode);

	Ref< ui::Node > editorNode = graphControl->createNode(
		scriptNode->getName(),
		L"",
		ui::UnitPoint(
			ui::Unit(scriptNode->getPosition().first),
			ui::Unit(scriptNode->getPosition().second)),
		m_nodeShape);

	for (int j = 0; j < scriptNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = scriptNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			inputPin->getId(),
			!inputPin->isOptional(),
			false);
	}

	for (int j = 0; j < scriptNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = scriptNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName(),
			outputPin->getId());
	}

	editorNode->setComment(scriptNode->getComment());
	editorNode->setInfo(scriptNode->getInformation());
	return editorNode;
}

void ScriptNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
	m_page->editScript(
		mandatory_non_null_type_cast< Script* >(shaderNode));
}

void ScriptNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode)
{
	Script* scriptNode = checked_type_cast< Script*, false >(shaderNode);

	editorNode->setTitle(scriptNode->getName());
	editorNode->setComment(scriptNode->getComment());
	editorNode->setInfo(scriptNode->getInformation());

	// Add or rename editor pins.
	for (int j = 0; j < scriptNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = scriptNode->getInputPin(j);
		ui::Pin* editorInputPin = editorNode->findInputPin(inputPin->getId());
		if (editorInputPin)
			editorInputPin->setName(inputPin->getName());
		else
			editorNode->createInputPin(inputPin->getName(), inputPin->getId(), !inputPin->isOptional(), false);
	}

	for (int j = 0; j < scriptNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = scriptNode->getOutputPin(j);
		ui::Pin* editorOutputPin = editorNode->findOutputPin(outputPin->getId());
		if (editorOutputPin)
			editorOutputPin->setName(outputPin->getName());
		else
			editorNode->createOutputPin(outputPin->getName(), outputPin->getId());
	}

	// Remove editors pins.
	RefArray< ui::Pin > removeEditorInputPins;
	for (auto editorInputPin : editorNode->getInputPins())
	{
		bool valid = false;
		for (int j = 0; j < scriptNode->getInputPinCount(); ++j)
		{
			if (scriptNode->getInputPin(j)->getId() == editorInputPin->getId())
			{
				valid = true;
				break;
			}
		}
		if (!valid)
			removeEditorInputPins.push_back(editorInputPin);
	}
	for (auto editorInputPin : removeEditorInputPins)
		editorNode->removeInputPin(editorInputPin);

	RefArray< ui::Pin > removeEditorOutputPins;
	for (auto editorOutputPin : editorNode->getOutputPins())
	{
		bool valid = false;
		for (int j = 0; j < scriptNode->getOutputPinCount(); ++j)
		{
			if (scriptNode->getOutputPin(j)->getId() == editorOutputPin->getId())
			{
				valid = true;
				break;
			}
		}
		if (!valid)
			removeEditorOutputPins.push_back(editorOutputPin);
	}
	for (auto editorOutputPin : removeEditorOutputPins)
		editorNode->removeOutputPin(editorOutputPin);
}

void ScriptNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

}
