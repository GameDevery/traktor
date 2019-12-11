#pragma once

#include "Ui/PropertyList/PropertyList.h"
#include "Core/Serialization/ISerializable.h"

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

/*! Auto property list.
 * \ingroup UI
 */
class T_DLLCLASS AutoPropertyList : public PropertyList
{
	T_RTTI_CLASS;

public:
	bool bind(ISerializable* object);

	bool refresh();

	bool refresh(PropertyItem* parent, ISerializable* object);

	bool apply();

	/*! Add object to property list.
	 *
	 * Use this method when adding an object to an array item.
	 */
	bool addObject(PropertyItem* parent, ISerializable* object);

	virtual bool paste() override;

private:
	Ref< ISerializable > m_object;
};

	}
}

