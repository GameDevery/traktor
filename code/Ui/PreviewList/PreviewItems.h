#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

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

class PreviewItem;

class T_DLLCLASS PreviewItems : public Object
{
	T_RTTI_CLASS;

public:
	void add(PreviewItem* item);

	void remove(PreviewItem* item);

	void removeAll();

	int count() const;

	Ref< PreviewItem > get(int index) const;

private:
	RefArray< PreviewItem > m_items;
};

	}
}

