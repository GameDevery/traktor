#ifndef traktor_flash_AsTextField_StyleSheet_H
#define traktor_flash_AsTextField_StyleSheet_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief TextField.StyleSheet class.
 * \ingroup Flash
 */
class AsTextField_StyleSheet : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsTextField_StyleSheet(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args) const;

	virtual void coerce(ActionObject* self) const;

private:
	void TextField_StyleSheet_clear(CallArgs& ca);

	void TextField_StyleSheet_getStyle(CallArgs& ca);

	void TextField_StyleSheet_getStyleNames(CallArgs& ca);

	void TextField_StyleSheet_load(CallArgs& ca);

	void TextField_StyleSheet_parseCSS(CallArgs& ca);

	void TextField_StyleSheet_setStyle(CallArgs& ca);

	void TextField_StyleSheet_transform(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsTextField_StyleSheet_H
