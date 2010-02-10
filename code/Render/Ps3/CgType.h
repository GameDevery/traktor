#ifndef traktor_render_CgType_H
#define traktor_render_CgType_H

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup PS3 */
//@{

enum CgType
{
	CtVoid,
	CtBoolean,
	CtFloat,
	CtFloat2,
	CtFloat3,
	CtFloat4,
	CtFloat4x4,
	CtTexture
};

int32_t cg_attr_index(DataUsage usage, int32_t index);

std::wstring cg_semantic(DataUsage usage, int32_t index);

std::wstring cg_type_name(CgType type);

CgType cg_from_data_type(DataType type);

//@}

	}
}

#endif	// traktor_render_CgType_H
