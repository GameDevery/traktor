#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace model
	{

/*! \brief
 * \ingroup Model
 */
class VoxelGrid : public Object
{
	T_RTTI_CLASS;

public:
	VoxelGrid(int32_t sizeX, int32_t sizeY, int32_t sizeZ);

	void set(int32_t x, int32_t y, int32_t z, uint32_t value);

	uint32_t get(int32_t x, int32_t y, int32_t z) const;

private:
	int32_t m_sizeX;
	int32_t m_sizeY;
	int32_t m_sizeZ;
	AlignedVector< uint32_t > m_cells;
};

	}
}

