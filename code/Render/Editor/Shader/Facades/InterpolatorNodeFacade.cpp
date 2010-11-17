#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Editor/Shader/Facades/InterpolatorNodeFacade.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/IpolNodeShape.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.InterpolatorNodeFacade", InterpolatorNodeFacade, NodeFacade)

InterpolatorNodeFacade::InterpolatorNodeFacade()
{
	m_nodeShape = new ui::custom::IpolNodeShape();
}

Ref< Node > InterpolatorNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->createInstance());
}

Ref< ui::custom::Node > InterpolatorNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
		L"",
		L"",
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
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

void InterpolatorNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	Node* shaderNode
)
{
}

void InterpolatorNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	Node* shaderNode
)
{
	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(shaderNode->getInformation());
}

void InterpolatorNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setColor(validationSucceeded ? Color(255, 255, 255) : Color(255, 120, 120));
}

	}
}
