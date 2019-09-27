#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace svg
	{

class Gradient;

/*! SVG style description.
 * \ingroup SVG
 */
class T_DLLCLASS Style : public Object
{
	T_RTTI_CLASS;

public:
	Style();

	void setFillEnable(bool fillEnable);

	bool getFillEnable() const;

	void setFillGradient(const Gradient* fillGradient);

	const Gradient* getFillGradient() const;

	void setFill(const Color4f& fill);

	const Color4f& getFill() const;

	void setStrokeEnable(bool strokeEnable);

	bool getStrokeEnable() const;

	void setStrokeGradient(const Gradient* strokeGradient);

	const Gradient* getStrokeGradient() const;

	void setStrokeWidth(float strokeWidth);

	float getStrokeWidth() const;

	void setStroke(const Color4f& stroke);

	const Color4f& getStroke() const;

	void setOpacity(float opacity);

	float getOpacity() const;

	bool operator == (const Style& other) const;

private:
	bool m_fillEnable;
	Ref< const Gradient > m_fillGradient;
	Color4f m_fill;
	bool m_strokeEnable;
	Ref< const Gradient > m_strokeGradient;
	float m_strokeWidth;
	Color4f m_stroke;
	float m_opacity;
};

	}
}

