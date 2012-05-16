#ifndef traktor_hf_Heightfield_H
#define traktor_hf_Heightfield_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Heightfield/HeightfieldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS Heightfield : public Object
{
	T_RTTI_CLASS;

public:
	Heightfield(
		uint32_t size,
		const Vector4& worldExtent
	);

	float getGridHeightNearest(int32_t gridX, int32_t gridZ) const;

	float getGridHeightBilinear(float gridX, float gridZ) const;

	float getWorldHeight(float worldX, float worldZ) const;

	void gridToWorld(float gridX, float gridZ, float& outWorldX, float& outWorldZ) const;

	void worldToGrid(float worldX, float worldZ, float& outGridX, float& outGridZ) const;

	float unitToWorld(float unitY) const;

	uint32_t getSize() const { return m_size; }

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	height_t* getHeights() { return m_heights.ptr(); }

	const height_t* getHeights() const { return m_heights.c_ptr(); }

private:
	uint32_t m_size;
	Vector4 m_worldExtent;
	AutoArrayPtr< height_t > m_heights;
};

	}
}

#endif	// traktor_hf_Heightfield_H
