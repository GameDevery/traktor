#ifndef traktor_flash_As_flash_geom_ColorTransform_H
#define traktor_flash_As_flash_geom_ColorTransform_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ColorTransform;

class As_flash_geom_ColorTransform : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_ColorTransform(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);

private:
	avm_number_t ColorTransform_get_alphaMultiplier(ColorTransform* self) const;

	void ColorTransform_set_alphaMultiplier(ColorTransform* self, avm_number_t value) const;

	avm_number_t ColorTransform_get_alphaOffset(ColorTransform* self) const;

	void ColorTransform_set_alphaOffset(ColorTransform* self, avm_number_t value) const;

	avm_number_t ColorTransform_get_blueMultiplier(ColorTransform* self) const;

	void ColorTransform_set_blueMultiplier(ColorTransform* self, avm_number_t value) const;

	avm_number_t ColorTransform_get_blueOffset(ColorTransform* self) const;

	void ColorTransform_set_blueOffset(ColorTransform* self, avm_number_t value) const;

	avm_number_t ColorTransform_get_greenMultiplier(ColorTransform* self) const;

	void ColorTransform_set_greenMultiplier(ColorTransform* self, avm_number_t value) const;

	avm_number_t ColorTransform_get_greenOffset(ColorTransform* self) const;

	void ColorTransform_set_greenOffset(ColorTransform* self, avm_number_t value) const;

	avm_number_t ColorTransform_get_redMultiplier(ColorTransform* self) const;

	void ColorTransform_set_redMultiplier(ColorTransform* self, avm_number_t value) const;

	avm_number_t ColorTransform_get_redOffset(ColorTransform* self) const;

	void ColorTransform_set_redOffset(ColorTransform* self, avm_number_t value) const;

	avm_number_t ColorTransform_get_rgb(ColorTransform* self) const;

	void ColorTransform_set_rgb(ColorTransform* self, avm_number_t value) const;
};

	}
}

#endif	// traktor_flash_As_flash_geom_ColorTransform_H
