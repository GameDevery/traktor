#pragma once

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace ui
	{

class CloseEvent;

	}

	namespace spark
	{

class FlashDebuggerDialog;

class DebuggerTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const override final;

	virtual Ref< ui::IBitmap > getIcon() const override final;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const override final;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param) override final;

private:
	Ref< FlashDebuggerDialog > m_debuggerDialog;

	void eventCloseDialog(ui::CloseEvent* event);
};

	}
}

