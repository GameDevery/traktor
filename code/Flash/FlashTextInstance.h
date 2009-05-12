#ifndef traktor_flash_FlashTextInstance_H
#define traktor_flash_FlashTextInstance_H

#include "Flash/FlashCharacterInstance.h"

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

class FlashText;

/*! \brief Flash static text instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashTextInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS(FlashTextInstance)

public:
	FlashTextInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashText* text);

	const FlashText* getText() const;

	virtual SwfRect getBounds() const;

private:
	Ref< const FlashText > m_text;
};

	}
}

#endif	// traktor_flash_FlashTextInstance_H
