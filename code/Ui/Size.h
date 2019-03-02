#pragma once

namespace traktor
{
	namespace ui
	{

/*! \brief Size
 * \ingroup UI
 */
class Size
{
public:
	int cx;
	int cy;

	inline Size();

	inline Size(int x, int y);

	inline Size(const Size& size);

	inline Size operator - () const;

	inline Size operator + (const Size& r) const;

	inline Size& operator += (const Size& r);

	inline Size operator - (const Size& r) const;

	inline Size& operator -= (const Size& r);
};

	}
}

#include "Ui/Size.inl"

