#ifndef traktor_ui_custom_SequenceItem_H
#define traktor_ui_custom_SequenceItem_H

#include <string>
#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Ui/Associative.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Canvas;
class Rect;
class Point;

		namespace custom
		{

class SequencerControl;

/*! \brief Sequence item.
 * \ingroup UIC
 */
class T_DLLCLASS SequenceItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS(SequenceItem)

public:
	SequenceItem(const std::wstring& name);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	bool setSelected(bool selected);

	bool isSelected() const;

	SequenceItem* getParentItem();

	void addChildItem(SequenceItem* childItem);

	void removeChildItem(SequenceItem* childItem);

	RefList< SequenceItem >& getChildItems();

	int getDepth();

	virtual void mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) = 0;

	virtual void mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) = 0;

	virtual void paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset) = 0;

private:
	std::wstring m_name;
	bool m_selected;
	Ref< SequenceItem > m_parent;
	RefList< SequenceItem > m_childItems;
};

		}
	}
}

#endif	// traktor_ui_custom_SequenceItem_H
