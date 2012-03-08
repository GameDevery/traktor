#ifndef traktor_scene_TranslateModifier_H
#define traktor_scene_TranslateModifier_H

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Scene/Editor/IModifier.h"

namespace traktor
{
	namespace scene
	{

class EntityAdapter;

/*! \brief Translation modifier. */
class TranslateModifier : public IModifier
{
	T_RTTI_CLASS;

public:
	TranslateModifier(SceneEditorContext* context);

	/*! \name Notifications */
	//\{

	virtual void selectionChanged();

	virtual bool cursorMoved(const TransformChain& transformChain, const Vector2& cursorPosition);

	virtual bool handleCommand(const ui::Command& command);

	//\}

	/*! \name Modifications */
	//\{

	virtual void begin(const TransformChain& transformChain);

	virtual void apply(const TransformChain& transformChain, const Vector4& screenDelta, const Vector4& viewDelta);

	virtual void end(const TransformChain& transformChain);

	//\}

	/*! \name Preview */
	//\{

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const;

	//\}

private:
	SceneEditorContext* m_context;
	RefArray< EntityAdapter > m_entityAdapters;
	AlignedVector< Vector4 > m_baseTranslations;
	Vector4 m_center;
	uint32_t m_axisEnable;
};

	}
}

#endif	// traktor_scene_TranslateModifier_H
