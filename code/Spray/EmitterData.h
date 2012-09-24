#ifndef traktor_spray_EmitterData_H
#define traktor_spray_EmitterData_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

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

	namespace render
	{

class Shader;

	}

	namespace spray
	{

class Emitter;
class ModifierData;
class SourceData;

/*! \brief Emitter persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EmitterData : public ISerializable
{
	T_RTTI_CLASS;

public:
	EmitterData();

	Ref< Emitter > createEmitter(resource::IResourceManager* resourceManager) const;

	virtual bool serialize(ISerializer& s);

	const SourceData* getSource() const { return m_source; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	Ref< SourceData > m_source;
	RefArray< ModifierData > m_modifiers;
	resource::Id< render::Shader > m_shader;
	float m_middleAge;
	float m_cullNearDistance;
	float m_fadeNearRange;
	float m_warmUp;
	bool m_sort;
	bool m_worldSpace;
};

	}
}

#endif	// traktor_spray_EmitterData_H
