#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Ui/Associative.h"
#include "Ui/Point.h"

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

class GraphCanvas;
class GraphControl;
class IBitmap;
class Pin;
class Size;

/*! Graph edge.
 * \ingroup UI
 */
class T_DLLCLASS Edge
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	Edge() = default;

	explicit Edge(Pin* source, Pin* destination);

	void setSourcePin(Pin* source);

	Pin* getSourcePin() const;

	void setDestinationPin(Pin* destination);

	Pin* getDestinationPin() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setThickness(int32_t thickness);

	int32_t getThickness() const;

	void setSelected(bool selected);

	bool isSelected() const;

	bool hit(const Point& p) const;

	void paint(GraphControl* graph, GraphCanvas* canvas, const Size& offset, IBitmap* imageLabel) const;

private:
	Ref< Pin > m_source;
	Ref< Pin > m_destination;
	std::wstring m_text;
	int32_t m_thickness = 2;
	bool m_selected = false;
	SmallMap< std::wstring, Ref< Object > > m_data;
	mutable AlignedVector< Point > m_spline;
};

	}
}

