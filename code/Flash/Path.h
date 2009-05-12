#ifndef traktor_flash_Path_H
#define traktor_flash_Path_H

#include <vector>
#include <list>
#include "Core/Object.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Segment type.
 * \ingroup Flash
 */
enum SubPathSegmentType
{
	SpgtUndefined,
	SpgtLinear,
	SpgtQuadratic
};

/*! \brief Sub path segment.
 * \ingroup Flash
 */
struct SubPathSegment
{
	SubPathSegmentType type;
	std::vector< Vector2 > points;

	SubPathSegment(SubPathSegmentType type_ = SpgtUndefined)
	:	type(type_)
	{}
};

/*! \brief Sub path.
 * \ingroup Flash
 */
struct SubPath
{
	uint16_t fillStyle0;
	uint16_t fillStyle1;
	uint16_t lineStyle;
	std::vector< SubPathSegment > segments;

	SubPath()
	:	fillStyle0(0)
	,	fillStyle1(0)
	,	lineStyle(0)
	{
	}
};

/*! \brief Shape path.
 * \ingroup Flash
 */
class T_DLLCLASS Path : public Object
{
	T_RTTI_CLASS(Path)

public:
	enum CoordinateMode
	{
		CmRelative,
		CmAbsolute
	};

	Path();

	/*! \brief Reset path. */
	void reset();

	/*! \brief Move cursor to position.
	 *
	 * \param x Cursor x position.
	 * \param y Cursor y position.
	 * \param mode Coordinate mode.
	 */
	void moveTo(float x, float y, CoordinateMode mode);

	/*! \brief Line from cursor to position.
	 *
	 * \param x End x position.
	 * \param y End y position.
	 * \param mode Coordinate mode.
	 */
	void lineTo(float x, float y, CoordinateMode mode);

	/*! \brief Quadratic spline from cursor to position.
	 *
	 * \param x1 Control point.
	 * \param y1 Control point.
	 * \param x End x position.
	 * \param y End y position.
	 * \param mode Coordinate mode.
	 */
	void quadraticTo(float x1, float y1, float x, float y, CoordinateMode mode);

	/*! \brief End path.
	 *
	 * \param fillStyle0 Index to odd fill style, 0 = no style.
	 * \param fillStyle1 Index to even fill style, 0 = no style.
	 * \param lineStyle Index to line style, 0 = no style.
	 */
	void end(uint16_t fillStyle0, uint16_t fillStyle1, uint16_t lineStyle);

	/*! \brief Get cursor position.
	 *
	 * \return Cursor position.
	 */
	Vector2 getCursor() const;

	/*! \brief Get sub paths.
	 *
	 * \return List of sub-paths.
	 */
	const std::list< SubPath >& getSubPaths() const;

private:
	Vector2 m_cursor;
	std::list< SubPath > m_subPaths;
	SubPath m_current;

	/*! \brief Transform between coordinate modes.
	 *
	 * \param from From coordinate mode.
	 * \param to To coordinate mode.
	 * \param x X coordinate.
	 * \param y Y coordinate.
	 */
	void transform(CoordinateMode from, CoordinateMode to, float& x, float& y) const;
};

	}
}

#endif	// traktor_flash_Path_H
