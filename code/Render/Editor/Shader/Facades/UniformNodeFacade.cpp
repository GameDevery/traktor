#include "Render/Editor/Shader/InputPin.h"
#include "Render/Editor/Shader/OutputPin.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/UniformNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/InOutNodeShape.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UniformNodeFacade", UniformNodeFacade, INodeFacade)

UniformNodeFacade::UniformNodeFacade(ui::GraphControl* graphControl)
{
	m_nodeShape = new ui::InOutNodeShape(graphControl, ui::InOutNodeShape::StUniform);
}

Ref< Node > UniformNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Uniform();
}

Ref< ui::Node > UniformNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< ui::Node > editorNode = new ui::Node(
		L"",
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

void UniformNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
}

void UniformNodeFacade::refreshEditorNode(
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

void UniformNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
