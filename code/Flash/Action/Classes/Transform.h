#ifndef traktor_flash_Transform_H
#define traktor_flash_Transform_H

#include "Flash/Action/ActionObject.h"

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

class ColorTransform;
class FlashCharacterInstance;

/*! \brief Geometry transform wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS Transform : public ActionObject
{
	T_RTTI_CLASS;

public:
	Transform(FlashCharacterInstance* instance);

	Ref< ColorTransform > getColorTransform() const;

	void setColorTransform(const ColorTransform* colorTransform);

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	Ref< FlashCharacterInstance > m_instance;
};

	}
}

#endif	// traktor_flash_Transform_H
