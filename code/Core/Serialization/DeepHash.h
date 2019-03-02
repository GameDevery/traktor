#pragma once

#include "Core/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

/*! \brief Calculate hash of object.
 * \ingroup Core
 *
 * Calculates an Adler-32 hash from a given object
 * using serialization, thus other objects referenced
 * by the object is part of the final hash value.
 */
class T_DLLCLASS DeepHash : public Object
{
	T_RTTI_CLASS;

public:
	DeepHash(const ISerializable* object);

	uint32_t get() const;

	bool operator == (const DeepHash& hash) const;

	bool operator != (const DeepHash& hash) const;

	bool operator == (const DeepHash* hash) const;

	bool operator != (const DeepHash* hash) const;

	bool operator == (const ISerializable* object) const;

	bool operator != (const ISerializable* object) const;

	bool operator == (uint32_t hash) const;

	bool operator != (uint32_t hash) const;

private:
	uint32_t m_hash;
};

}

