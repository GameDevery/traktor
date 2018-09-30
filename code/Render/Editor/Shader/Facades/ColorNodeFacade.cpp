/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/ColorNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/InputNodeShape.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ColorNodeFacade", ColorNodeFacade, INodeFacade)

ColorNodeFacade::ColorNodeFacade(ui::GraphControl* graphControl)
{
	m_nodeShape = new ui::InputNodeShape(graphControl);
}

Ref< Node > ColorNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Color();
}

Ref< ui::Node > ColorNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< ui::Node > editorNode = new ui::Node(
		i18n::Text(L"SHADERGRAPH_NODE_COLOR"),
		shaderNode->getInformation(),
		ui::Point(
			ui::dpi96(shaderNode->getPosition().first),
			ui::dpi96(shaderNode->getPosition().second)
		),
		m_nodeShape
	);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName()
		);
	}

	editorNode->setComment(shaderNode->getComment());

	return editorNode;
}

void ColorNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< Color > colorNode = checked_type_cast< Color* >(shaderNode);

	ui::ColorDialog colorDialog;
	colorDialog.create(
		graphControl,
		i18n::Text(L"COLOR_DIALOG_TEXT"),
		ui::ColorDialog::WsDefaultFixed | ui::ColorDialog::WsAlpha,
		colorNode->getColor()
	);
	if (colorDialog.showModal() == ui::DrOk)
		colorNode->setColor(colorDialog.getColor());
	colorDialog.destroy();
}

void ColorNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(shaderNode->getInformation());
}

void ColorNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
