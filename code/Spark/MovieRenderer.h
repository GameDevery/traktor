#pragma once

#include "Core/Object.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"

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

class ColorTransform;
class IDisplayRenderer;
class Dictionary;
class Sprite;
class SpriteInstance;
class CharacterInstance;

/*! Movie renderer.
 * \ingroup Spark
 */
class T_DLLCLASS MovieRenderer : public Object
{
	T_RTTI_CLASS;

public:
	explicit MovieRenderer(IDisplayRenderer* displayRenderer);

	void render(
		SpriteInstance* movieInstance,
		const Aabb2& frameBounds,
		const Vector4& frameTransform,
		float viewWidth,
		float viewHeight
	);

private:
	Ref< IDisplayRenderer > m_displayRenderer;

	void renderSprite(
		SpriteInstance* spriteInstance,
		const Matrix33& transform,
		const Aabb2& clipBounds,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteDefault(
		SpriteInstance* spriteInstance,
		const Matrix33& transform,
		const Aabb2& clipBounds,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteLayered(
		SpriteInstance* spriteInstance,
		const Matrix33& transform,
		const Aabb2& clipBounds,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderSpriteWithScalingGrid(
		SpriteInstance* spriteInstance,
		const Matrix33& transform,
		const Aabb2& clipBounds,
		const ColorTransform& cxTransform,
		bool renderAsMask
	);

	void renderCharacter(
		CharacterInstance* characterInstance,
		const Matrix33& transform,
		const Aabb2& clipBounds,
		const ColorTransform& cxTransform,
		bool renderAsMask,
		uint8_t blendMode
	);
};

	}
}

