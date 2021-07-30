#pragma once

#include <string>
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"
#include "Model/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace model
	{

/*! Joint
 * \ingroup Model
 */
class T_DLLCLASS Joint : public ISerializable
{
	T_RTTI_CLASS;

public:
	Joint() = default;

	explicit Joint(const std::wstring& name);

	explicit Joint(uint32_t parent, const std::wstring& name, const Transform& transform, float length);

	void setParent(uint32_t parent);

	uint32_t getParent() const;

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setTransform(const Transform& transform);

	const Transform& getTransform() const;

	void setLength(float length);

	float getLength() const;

	virtual void serialize(ISerializer& s) override final;

	bool operator == (const Joint& rh) const;

private:
	uint32_t m_parent = c_InvalidIndex;
	std::wstring m_name;
	Transform m_transform = Transform::identity();
	float m_length = 1.0f;
};

	}
}
