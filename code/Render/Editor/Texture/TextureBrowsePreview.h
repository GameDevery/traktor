#ifndef traktor_render_TextureBrowsePreview_H
#define traktor_render_TextureBrowsePreview_H

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
	virtual TypeInfoSet getPreviewTypes() const;

	virtual Ref< ui::Bitmap > generate(const editor::IEditor* editor, db::Instance* instance) const;
};

	}
}

#endif	// traktor_render_TextureBrowsePreview_H
