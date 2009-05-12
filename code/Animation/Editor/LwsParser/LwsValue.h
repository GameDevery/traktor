#ifndef traktor_animation_LwsValue_H
#define traktor_animation_LwsValue_H

#include "Animation/Editor/LwsParser/LwsNode.h"

namespace traktor
{
	namespace animation
	{

class LwsValue : public LwsNode
{
	T_RTTI_CLASS(LwsValue)

public:
	LwsValue(const std::vector< std::wstring >& pieces);

	uint32_t getCount() const;

	int32_t getInteger(uint32_t index) const;

	float getFloat(uint32_t index) const;

	const std::wstring& getString(uint32_t index) const;

private:
	std::vector< std::wstring > m_values;
};

	}
}

#endif	// traktor_animation_LwsValue_H
