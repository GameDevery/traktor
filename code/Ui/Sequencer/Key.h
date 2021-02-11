#pragma once

#include "Core/Object.h"
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

class Canvas;
class Point;
class Rect;
class Sequence;
class SequencerControl;

/*! Sequence key.
 * \ingroup UI
 */
class T_DLLCLASS Key
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	virtual void move(int offset) = 0;

	virtual void getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const = 0;

	virtual void paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset) = 0;
};

	}
}

