#include "Core/Io/StringOutputStream.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Facades/ExternalNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/PaintSettings.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ExternalNodeFacade", ExternalNodeFacade, INodeFacade)

ExternalNodeFacade::ExternalNodeFacade(ui::GraphControl* graphControl)
{
	m_nodeShape = new ui::DefaultNodeShape(graphControl, ui::DefaultNodeShape::StExternal);
}

Ref< Node > ExternalNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	editor::TypeBrowseFilter filter(type_of< ShaderGraph >());
	Ref< db::Instance > fragmentInstance = editor->browseInstance(&filter);
	if (!fragmentInstance)
		return 0;

	Ref< ShaderGraph > fragmentGraph = fragmentInstance->getObject< ShaderGraph >();
	if (!fragmentGraph)
		return 0;

	return new External(
		fragmentInstance->getGuid(),
		fragmentGraph
	);
}

Ref< ui::Node > ExternalNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	External* externalNode = mandatory_non_null_type_cast< External* >(shaderNode);
	std::wstring title;

	Guid fragmentGuid = externalNode->getFragmentGuid();

	Ref< ShaderGraph > fragmentGraph;
	RefArray< InputPort > inputPorts;

	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(fragmentGuid);
	if (instance)
	{
		title = instance->getName();
		fragmentGraph = instance->getObject< ShaderGraph >();
		fragmentGraph->findNodesOf< InputPort >(inputPorts);
	}
	else
		title = fragmentGuid.format();

	Ref< ui::Node > editorNode = new ui::Node(
		title,
		L"",
		ui::Point(
			ui::dpi96(shaderNode->getPosition().first),
			ui::dpi96(shaderNode->getPosition().second)
		),
		m_nodeShape
	);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);

		StringOutputStream ss;
		ss << inputPin->getName();

		if (inputPin->isOptional())
		{
			const auto& values = externalNode->getValues();
			const auto it = values.find(inputPin->getName());
			if (it != values.end())
				ss << L" (" << it->second << L")";
			else if (fragmentGraph)
			{
				auto it = std::find_if(inputPorts.begin(), inputPorts.end(), [=](InputPort* inputPort) {
					return inputPort->getName() == inputPin->getName();
				});
				if (it != inputPorts.end())
				{
					if (it->haveDefaultValue())
						ss << L" (" << it->getDefaultValue() << L")";
				}
				else
					ss << L" (N/A)";
			}
			else
				ss << L" (N/A)";
		}

		editorNode->createInputPin(
			inputPin->getName(),
			ss.str(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName(),
			outputPin->getName()
		);
	}

	editorNode->setComment(shaderNode->getComment());

	return editorNode;
}

void ExternalNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(
		checked_type_cast< External* >(shaderNode)->getFragmentGuid()
	);
	if (instance)
		editor->openEditor(instance);
}

void ExternalNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	External* external = checked_type_cast< External*, false >(shaderNode);
	Ref< db::Instance > instance = editor->getSourceDatabase()->getInstance(static_cast< External* >(shaderNode)->getFragmentGuid());
	editorNode->setTitle(instance ? instance->getName() : L"{ Null reference }");
	editorNode->setComment(shaderNode->getComment());
}

void ExternalNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
