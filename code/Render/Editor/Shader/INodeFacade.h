#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class GraphControl;
class Node;

	}

	namespace render
	{

class Node;
class ShaderGraph;

/*! Node facade bridging editor and shader graph nodes.
 * \ingroup Render
 */
class INodeFacade : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create a shader graph node. */
	virtual Ref< Node > createShaderNode(
		const TypeInfo* nodeType,
		editor::IEditor* editor
	) = 0;

	/*! Create a UI editor node. */
	virtual Ref< ui::Node > createEditorNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	/*! Edit properties of shader node; usually from double click on editor node. */
	virtual void editShaderNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ui::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	/*! Refresh editor represenation of shader node. */
	virtual void refreshEditorNode(
		editor::IEditor* editor,
		ui::GraphControl* graphControl,
		ui::Node* editorNode,
		ShaderGraph* shaderGraph,
		Node* shaderNode
	) = 0;

	/*! Set validation indicator on editor node.
	 *  Usually change color of node to indicate error/success.
	 */
	virtual void setValidationIndicator(
		ui::Node* editorNode,
		bool validationSucceeded
	) = 0;
};

	}
}

