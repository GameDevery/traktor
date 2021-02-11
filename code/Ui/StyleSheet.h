#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Color4ub.h"
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

class StyleBitmap;

/*! Widget style sheet.
 * \ingroup UI
 */
class T_DLLCLASS StyleSheet : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Entity
	{
		std::wstring typeName;
		SmallMap< std::wstring, Color4ub > colors;
	};

	Entity* findEntity(const std::wstring& typeName);

	Entity* addEntity(const std::wstring& typeName);

	void setColor(const std::wstring& typeName, const std::wstring& element, const Color4ub& color);

	Color4ub getColor(const std::wstring& typeName, const std::wstring& element) const;

	Color4ub getColor(const Object* widget, const std::wstring& element) const;

	void setValue(const std::wstring& name, const std::wstring& value);

	std::wstring getValue(const std::wstring& name) const;

	/*! Merge this style sheet with another.
	 *
	 * Styles defined in right override existing styles.
	 * */
	Ref< StyleSheet > merge(const StyleSheet* right) const;

	virtual void serialize(ISerializer& s) override;

	AlignedVector< Entity >& getEntities() { return m_entities; }

	const AlignedVector< Entity >& getEntities() const { return m_entities; }

	const SmallMap< std::wstring, std::wstring >& getValues() const { return m_values; }

	static Ref< StyleSheet > createDefault();

private:
	AlignedVector< Entity > m_entities;
	SmallMap< std::wstring, std::wstring > m_values;
};

	}
}

