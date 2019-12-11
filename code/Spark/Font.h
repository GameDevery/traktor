#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"
#include "Spark/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Shape;

/*! Font
 * \ingroup Spark
 */
class T_DLLCLASS Font : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum CoordinateType
	{
		CtTwips,
		CtEMSquare
	};

	Font();

	bool create(const AlignedVector< SwfShape* >& shapeTable);

	bool create(
		const std::string& fontName,
		bool italic,
		bool bold,
		const AlignedVector< SwfShape* >& shapeTable,
		int16_t ascent,
		int16_t descent,
		int16_t leading,
		const AlignedVector< int16_t >& advanceTable,
		const AlignedVector< Aabb2 >& boundsTable,
		const AlignedVector< SwfKerningRecord >& kerningRecords,
		const AlignedVector< uint16_t >& codeTable,
		CoordinateType coordinateType
	);

	const std::string& getFontName() const;

	bool isItalic() const;

	bool isBold() const;

	const RefArray< Shape >& getShapes() const;

	const Shape* getShape(uint16_t index) const;

	int16_t getAscent() const;

	int16_t getDescent() const;

	int16_t getLeading() const;

	int16_t getAdvance(uint16_t index) const;

	const Aabb2* getBounds(uint16_t index) const;

	const Vector2& getMaxDimension() const;

	int16_t lookupKerning(uint16_t leftCode, uint16_t rightCode) const;

	uint16_t lookupIndex(uint16_t code) const;

	CoordinateType getCoordinateType() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::string m_fontName;
	bool m_italic;
	bool m_bold;
	RefArray< Shape > m_shapes;
	int16_t m_ascent;
	int16_t m_descent;
	int16_t m_leading;
	AlignedVector< int16_t > m_advanceTable;
	AlignedVector< Aabb2 > m_boundsTable;
	Vector2 m_maxDimension;
	SmallMap< uint32_t, int16_t > m_kerningLookup;
	SmallMap< uint16_t, uint16_t > m_indexLookup;
	CoordinateType m_coordinateType;
};

	}
}

