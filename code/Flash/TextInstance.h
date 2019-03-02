#pragma once

#include "Flash/CharacterInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class Text;

/*! \brief Flash static text instance.
 * \ingroup Flash
 */
class T_DLLCLASS TextInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	TextInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Text* text);

	const Text* getText() const;

	virtual Aabb2 getBounds() const override final;

private:
	Ref< const Text > m_text;
};

	}
}

