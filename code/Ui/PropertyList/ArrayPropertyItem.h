#pragma once

#include "Ui/PropertyList/PropertyItem.h"

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

class MiniButton;

/*! \brief Array property item.
 * \ingroup UI
 */
class T_DLLCLASS ArrayPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	ArrayPropertyItem(const std::wstring& text, const TypeInfo* elementType, bool readOnly);

	void setElementType(const TypeInfo* objectType);

	const TypeInfo* getElementType() const;

protected:
	virtual bool needRemoveChildButton() const override;

	virtual void createInPlaceControls(Widget* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void paintValue(Canvas& canvas, const Rect& rc) override;

private:
	Ref< IBitmap > m_imageSmallDots;
	Ref< IBitmap > m_imageSmallPlus;
	const TypeInfo* m_elementType;
	bool m_readOnly;
	Ref< MiniButton > m_buttonEdit;

	void eventClick(ButtonClickEvent* event);
};

	}
}

