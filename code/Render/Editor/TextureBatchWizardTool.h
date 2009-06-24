#ifndef traktor_render_TextureBatchWizardTool_H
#define traktor_render_TextureBatchWizardTool_H

#include "Editor/IWizardTool.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS TextureBatchWizardTool : public editor::IWizardTool
{
	T_RTTI_CLASS(TextureBatchWizardTool)

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group);
};

	}
}

#endif	// traktor_render_TextureBatchWizardTool_H
