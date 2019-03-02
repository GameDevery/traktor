#pragma once

#include <map>
#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Ui/Associative.h"

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

/*! \brief Layer item.
 * \ingroup UI
 */
class T_DLLCLASS LayerItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	LayerItem(const std::wstring& text, bool enable = true);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setEnable(bool enable);

	bool isEnabled() const;

	void setSelected(bool selected);

	bool isSelected() const;

	Ref< LayerItem > getParentLayer();

	void addChildLayer(LayerItem* childLayer);

	void removeChildLayer(LayerItem* childLayer);

	RefArray< LayerItem >& getChildLayers();

private:
	std::wstring m_text;
	bool m_enable;
	bool m_selected;
	Ref< LayerItem > m_parent;
	RefArray< LayerItem > m_childLayers;
};

	}
}

