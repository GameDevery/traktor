/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_FilePropertyItem_H
#define traktor_ui_FilePropertyItem_H

#include "Ui/PropertyList/PropertyItem.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edit;
class MiniButton;

/*! \brief File property item.
 * \ingroup UI
 */
class T_DLLCLASS FilePropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	FilePropertyItem(const std::wstring& text, const Path& path);

	void setPath(const Path& path);

	const Path& getPath() const;

protected:
	virtual void createInPlaceControls(Widget* parent) T_OVERRIDE;

	virtual void destroyInPlaceControls() T_OVERRIDE;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) T_OVERRIDE;

	virtual void mouseButtonDown(MouseButtonDownEvent* event) T_OVERRIDE;

	virtual void paintValue(Canvas& canvas, const Rect& rc) T_OVERRIDE;

private:
	Ref< Edit > m_editor;
	Ref< MiniButton > m_buttonEdit;
	Path m_path;

	void eventEditFocus(FocusEvent* event);

	void eventClick(ButtonClickEvent* event);
};

	}
}

#endif	// traktor_ui_FilePropertyItem_H
