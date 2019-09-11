#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

class Source;

/*! Particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS SourceData : public ISerializable
{
	T_RTTI_CLASS;

public:
	SourceData();

	virtual Ref< const Source > createSource(resource::IResourceManager* resourceManager) const = 0;

	virtual void serialize(ISerializer& s) override;

	float getConstantRate() const { return m_constantRate; }

	float getVelocityRate() const { return m_velocityRate; }

private:
	float m_constantRate;
	float m_velocityRate;
};

	}
}

