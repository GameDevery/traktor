#pragma once

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class SearchToolDialog;

class SearchTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const override final;

	virtual Ref< ui::IBitmap > getIcon() const override final;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const override final;

	virtual bool launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param) override final;

private:
	Ref< SearchToolDialog > m_searchDialog;
};

	}
}

