#pragma once

#include "Editor/IBrowsePreview.h"

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

class T_DLLCLASS TextureBrowsePreview : public editor::IBrowsePreview
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getPreviewTypes() const override final;

	virtual Ref< ui::Bitmap > generate(const editor::IEditor* editor, db::Instance* instance) const override final;
};

	}
}

